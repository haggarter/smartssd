#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smartssd.h"

static const char *usage = "Usage: ./query_smart [string: drive name] [int: number of cycles]\n";

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Too few arguments.\n%s", usage);
        exit(1);
    }

    char *drive = argv[1];

    int cycles;

    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n%s", usage);
        exit(1);
    }
    
    smartssd *dev = (smartssd *)malloc(sizeof(smartssd));

    if (smartssd_init(dev, drive) == 0) {
        printf("Device protocol: %s\n",
            dev->type == SMARTSSD_PROTO_ATA ? "ATA" :
            dev->type == SMARTSSD_PROTO_NVME ? "NVMe" : "Unknown");
    } else {
        printf("Failed to initialize SSD.\n");
        exit(1);
    }

    smartssd_deinit(dev);

    free(dev);

    return 0;
}