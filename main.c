#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smartssd.h"

static const char *usage = "Usage: ./query_smart [string: drive name] [int: number of cycles]\n";

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

    int cycles;

    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Args ok.\n");
    
    smartssd *dev = (smartssd *)malloc(sizeof(smartssd));

    if (debug)
        printf("Initializing smartssd object...\n");

    if (smartssd_init(dev, drive, debug) == 0) {
        printf("Device protocol: %s\n",
            dev->type == SMARTSSD_PROTO_ATA ? "ATA" :
            dev->type == SMARTSSD_PROTO_NVME ? "NVMe" : "Unknown");
    } else {
        printf("Failed to initialize SSD.\n");
        exit(1);
    }

    if (debug)
        printf("Initialized smartssd.\n");

    smartssd_deinit(dev);

    free(dev);

    return 0;
}