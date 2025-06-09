#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <atasmart.h>
#include <libnvme.h>

#include "smartssd.h"

int smartssd_init (smartssd *dev, char *drive) {
    
    struct stat st;
    if (stat(drive, &st) != 0) {
        printf("Device not found.\n");
        return -1;
    }

    dev->path = drive;
    dev->type = SMARTSSD_PROTO_UNKNOWN;
    dev->sata_drive = NULL;
    dev->nvme_drive = NULL;

    if (sk_disk_open(path, &dev->sata_drive) == 0) {
        ssd->protocol = SMARTSSD_PROTO_ATA;
        return 0;
    }

    struct nvme_dev *nvme = nvme_open(drive);
    if (nvme) {
        struct nvme_id_ctrl id;
        if (nvme_identify_ctrl(nvme, &id) == 0) {
            dev->nvme_drive = nvme;
            dev->type = SMARTSSD_PROTO_NVME;
            return 0;
        } else {
            nvme_close(nvme);
        }
    }

    nvme_free_tree(r);

    printf("Unknown or unsupported device protocol\n");
    return -1;
}

int smartssd_deinit(smartssd *dev) {
    if (dev->type == SMARTSSD_PROTO_ATA) {
        sk_disk_free(dev->sata_drive);
        dev->sata_drive = NULL;
    } else if (dev->type == SMARTSSD_PROTO_NVME) {
        nvme_close(dev->nvme_drive);
        dev->nvme_drive = NULL;
    }
}