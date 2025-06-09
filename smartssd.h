#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <atasmart.h>
#include <nvme/nvme.h>

#define NVME 0
#define SATA 1
#define UNKNOWN -1

typedef struct smartssd {
    char type;
    const char *path;
    struct nvme_dev *nvme_drive;
    SkDisk *sata_drive;
} smartssd;

int smartssd_init(smartssd *dev);

int smartssd_deinit(smartssd *dev);

