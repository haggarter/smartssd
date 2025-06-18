#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define NUM_READS_PER_CYCLE 14
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)
#define MAX_IO_SIZE 2 * MB

static const char *usage = "Usage: ./smartssd [string: path to drive] [int: number of cycles] [string: name of data validation input file] [string: name of data validation output file] [string: name of SMART output file]\n";

int main(int argc, char *argv[]) {
    /*
    Check for debug flag (functionally verbose)
    */
    int debug = 0;

    if ((argc > 6) && (strcmp(argv[6], "--debug") == 0))
        debug = 1;

    /*
    Requires at least 4 arguments
    1- path to the drive
    2- number of read cycles to perform on the drive
    3- name of checksum output file
    4- name of SMART output file
    5 (optional)- debug flag 
    */
    if (debug)
        printf("Checking args...\n");

    if (argc < 6) {
        printf("Too few arguments.\n%s", usage);
        exit(1);
    }

    /*
    Make sure the drive exists.
    */
    char *drive = argv[1];

    if (debug)
        printf("Drive: %s\n", drive);

    if (debug)
        printf("Checking if drive exists...\n");

    struct stat st;
    if (stat(drive, &st) < 0) {
        printf("Drive not found.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Drive exists.\n");

    /*
    Make sure cycles is a valid integer.
    */
    int cycles;

    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Cycles: %d\n", cycles);

    /*
    Make sure data validation output is opened
    for writing.
    */
    if (debug)
        printf("Opening validation input file...\n");

    char *validation_in = argv[3];
    int validation_in_fd;
    if ((validation_in_fd = open(validation_in, O_RDONLY)) < 0) {
        printf("Validation input file could not be opened.\n");
        exit(1);
    }

    if (debug)
        printf("Validation input opened successfully: fd %d\n", validation_in_fd);

    /*
    Make sure data validation output is opened
    for writing.
    */
    if (debug)
        printf("Opening data validation output file...\n");

    char *validation_out = argv[4];
    int validation_out_fd;
    if ((validation_out_fd = open(validation_out, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        printf("Data validation output file could not be opened.\n");
        exit(1);
    }

    if (debug)
        printf("Data validation output file opened successfully: fd %d\n", validation_out_fd);

    /*
    Make sure smart output is opened
    for writing.
    */
    if (debug)
        printf("Opening smart output file...\n");

    char *smart_out = argv[5];
    int smart_out_fd;
    if ((smart_out_fd = open(smart_out, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        printf("Smart output file could not be opened.\n");
        exit(1);
    }

    if (debug)
        printf("Smart output file opened successfully: fd %d\n", smart_out_fd);

    if (debug)
        printf("Args ok.\n");
    
    /*
    Creates a 1GB buffer for all read cycles.
    This buffer must be aligned to the block size of the
    SSD's controller. 1MB is a good size on most modern
    devices. Even if the block size is smaller, like 512kB,
    most devices support read requests up to 1MB.
    */
    if (debug)
        printf("Creating 1GB buffer...\n");
    
    void *buf;
    if (posix_memalign(&buf, MAX_IO_SIZE, GB) != 0) {
        printf("Failed to allocate aligned buffer.\n");
        exit(1);
    }

    if (debug)
        printf("Read buffer allocated.\n");

    /*
    Open the drive for reading.
    The O_DIRECT flag bypasses the kernel's cache and
    allows the SSD to be read from directly. It
    reuires _GNU_SOURCE, as is defined at the beginning
    of the script.
    */
    if (debug)
        printf("Opening drive for I/O...\n");

    int drive_fd;
    if ((drive_fd = open(drive, O_RDONLY | O_DIRECT)) < 0) {
        printf("Unable to access drive.\n");
        free(buf);
        exit(1);
    }

    if (debug)
        printf("Drive ready (drive_fd: %d).\n", drive_fd);

    /*
    
    */
    printf("DRIVE READY, BEGINNING READ CYCLES\n");
    time_t start = time(NULL);
    for (int i = 0; i < cycles; i++) {
        printf("Starting Cycle %d...\n", i + 1);
        for (int j = 0; j < NUM_READS_PER_CYCLE; j++) {
            if (debug)
                printf("Read %d\n", j + 1);
            ssize_t total_read = 0;
            while (total_read < GB) {
                ssize_t num_read = pread(drive_fd, (char *)buf + total_read, MAX_IO_SIZE, total_read);
                if (num_read < 0) {
                    printf("Read error, aborting.\n");
                    close(drive_fd);
                    free(buf);
                    exit(1);
                }
                total_read += num_read;
            }
            if (debug)
                printf("Complete\n");
        }
        printf("Cycle %d Complete\n", i + 1);
    }
    time_t end = time(NULL);

    double elapsed_minutes = (float)(end - start) / 60.0;
    printf("Total time: %.2f minutes\n", elapsed_minutes);

    printf("FINISHED READ CYCLES, VALIDATING DATA\n");

    start = time(NULL);
    char *temp = "tmp_data";
    int temp_fd = open(temp, O_WRONLY | O_CREAT, 0644);

    int w_total = 0;
    while (w_total < GB) {
        w_total += write(temp_fd, buf + w_total, MB);
    }

    char cmp_cmd[512];
    snprintf(cmp_cmd, sizeof(cmp_cmd), "cmp -l %s %s | wc -l > %s", validation_in, temp, validation_out);

    system(cmp_cmd);

    close(temp_fd);
    system("rm tmp_data");
    end = time(NULL);

    elapsed_minutes = (float)(end - start) / 60.0;
    printf("Total time: %.2f minutes\n", elapsed_minutes);

    printf("DATA VALIDATED, QUERYING SMART ATTRIBUTES\n");

    start = time(NULL);
    char smart_cmd[512];
    snprintf(smart_cmd, sizeof(smart_cmd), "sudo smartctl -a --json %s > %s", drive, smart_out);
    
    system(smart_cmd);
    end = time(NULL);

    elapsed_minutes = (float)(end - start) / 60.0;
    printf("Total time: %.2f minutes\n", elapsed_minutes);

    /*
    Make sure to free the buffer and close
    all the open file descriptors.
    */
    printf("SUCCESS, GRACEFULLY EXITING\n");

    close(drive_fd);
    close(validation_in_fd);
    close(validation_out_fd);
    close(smart_out_fd);
    free(buf);

    return 0;
}