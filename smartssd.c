#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <openssl/sha.h>

#define NUM_READS_PER_CYCLE 14
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)

static const char *usage = "Usage: ./smartssd [string: path to drive] [int: number of cycles] [string: name of checksum output file] [string: name of SMART output file]\n";

int main(int argc, char *argv[]) {
    /*
    Check for debug flag (functionally verbose)
    */
    int debug = 0;

    if ((argc > 5) && (strcmp(argv[5], "--debug") == 0))
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

    if (argc < 5) {
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
    Make sure checksums output is opened
    for writing.
    */
    if (debug)
        printf("Opening checksums file...\n");

    char *checksum_out = argv[3];
    int checksum_fd;
    if (checksum_fd = open(checksum_out, O_WRONLY | O_CREAT, 0644) < 0) {
        printf("Checksum output file could not be opened.\n");
        exit(1);
    }

    if (debug)
        printf("Checksums file opened successfully.\n");

    /*
    Make sure SMART output is opened
    for writing.
    */
    if (debug)
        printf("Opening SMART file...\n");

    char *smart_out = argv[4];
    int smart_fd;
    if (smart_fd = open(smart_out, O_WRONLY | O_CREAT, 0644) < 0) {
        printf("SMART output file could not be opened.\n");
        exit(1);
    }

    if (debug)
        printf("SMART file opened successfully.\n");

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
    if (posix_memalign(&buf, MB, GB) != 0) {
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
                ssize_t num_read = pread(drive_fd, (char *)buf + total_read, MB, total_read);
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

    printf("FINISHED READ CYCLES, PERFORMING CHECKSUMS\n");

    if (debug)
        printf("Performing checksums...\n");

    unsigned char hashes[KB][SHA_DIGEST_LENGTH];
    for (int i = 0; i < KB; i++) {
        void *chunk = (char *)buf + i * MB;
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, chunk, MB);
        SHA1_Final(hashes[i], &ctx);
    }

    if (debug)
        printf("Checksums performed.\n");

    if (debug)
        printf("Saving checksums...\n");

    for (int i = 0; i < KB; i++) {
        write(checksum_fd, hashes[i], SHA_DIGEST_LENGTH);
    }

    if (debug)
        printf("Checksums saved.\n");

    printf("FINISHED CHECKSUMS, QUERYING SMART ATTRIBUTES\n");

    printf("SUCCESS, GRACEFULLY EXITING\n");

    close(drive_fd);
    free(buf);

    return 0;
}