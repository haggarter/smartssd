#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cjson/cJSON.h>

#define NUM_READS_PER_CYCLE 14
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)
#define PAGE_SIZE (4 * KB)

static const char *usage = "Usage: ./query_smart [string: path to drive] [int: number of cycles]\n";

int main(int argc, char *argv[]) {
    int debug = 0;

    if ((argc == 4) && (strcmp(argv[3], "--debug") == 0))
        debug = 1;

    if (debug)
        printf("Checking args...\n");

    if (argc < 3) {
        printf("Too few arguments.\n%s", usage);
        exit(1);
    }

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

    int cycles;

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
    
    void *buf;
    if (posix_memalign(&buf, PAGE_SIZE, GB) != 0) {
        printf("Failed to allocate aligned buffer.\n");
        exit(1);
    }

    if (debug)
        printf("Read buffer allocated.\n");

    if (debug)
        printf("Opening drive for I/O...\n");

    int fd;
    if ((fd = open(drive, O_RDONLY | O_DIRECT)) < 0) {
        printf("Uable to access drive.\n");
    }

    if (debug)
        printf("Drive ready.\n");

    free(buf);

    return 0;
}