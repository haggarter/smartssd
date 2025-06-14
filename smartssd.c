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

static const char *usage = "Usage: ./smartssd [string: path to drive] [int: number of cycles]\n";

int main(int argc, char *argv[]) {
    /*
    Check for debug flag (functionally verbose)
    */
    int debug = 0;

    if ((argc > 3) && (strcmp(argv[3], "--debug") == 0))
        debug = 1;

    if (debug)
        printf("Checking args...\n");

    /*
    Requires at least 2 arguments (thirds is actual command)
    1- path to the drive
    2- number of read cycles to perform on the drive
    3 (optional)- debug flag 
    */
    if (argc < 3) {
        printf("Too few arguments.\n%s", usage);
        exit(1);
    }

    char *drive = argv[1];

    if (debug)
        printf("Drive: %s\n", drive);

    if (debug)
        printf("Checking if drive exists...\n");

    /*
    Make sure the drive exists.
    */
    struct stat st;
    if (stat(drive, &st) < 0) {
        printf("Drive not found.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Drive exists.\n");

    int cycles;

    /*
    Make sure the cycles is a valid integer.
    */
    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Cycles: %d\n", cycles);

    if (debug)
        printf("Args ok.\n");
    
    if (debug)
        printf("Creating 1GB buffer...\n");
    
    /*
    Creates a 1GB buffer for all read cycles.
    This buffer must be aligned to the block size of the
    SSD's controller. 1MB is a good size on most modern
    devices. Even if the block size is smaller, like 512kB,
    most devices support read requests up to 1MB.
    */
    void *buf;
    if (posix_memalign(&buf, MB, GB) != 0) {
        printf("Failed to allocate aligned buffer.\n");
        exit(1);
    }

    if (debug)
        printf("Read buffer allocated.\n");

    if (debug)
        printf("Opening drive for I/O...\n");

    /*
    Open the drive for reading.
    The O_DIRECT flag bypasses the kernel's cache and
    allows the SSD to be read from directly. It
    reuires _GNU_SOURCE, as is defined at the beginning
    of the script.
    */
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

    start = time(NULL);
    unsigned char hash[SHA_DIGEST_LENGTH];
    for (int i = 0; i < KB; i++) {
        void *chunk = (char *)buf + i * MB;
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, chunk, MB);
        SHA1_Final(hash, &ctx);
        printf("Block %d: ", i);
        for (int j = 0; j < SHA_DIGEST_LENGTH; j++) {
            printf("%02x", hash[i]);
        }
        printf("\n");
    }
    end = time(NULL);

    elapsed_minutes = (float)(end - start) / 60.0;
    printf("Total time: %.2f minutes\n", elapsed_minutes);

    printf("FINISHED CHECKSUMS, QUERYING SMART ATTRIBUTES");

    printf("SUCCESS, GRACEFULLY EXITING");

    close(drive_fd);
    free(buf);

    return 0;
}