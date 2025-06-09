#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <atasmart.h>
#include <libnvme.h>

#include "smartssd.h"

int smartssd_init (smartssd *dev, char *drive) {
    
    char *full_path = strcat("/dev/", drive);

    struct stat st;
    if (stat(full_path, &st) != 0) {
        printf("Device not found.\n");
        return -1;
    }

    dev->path = drive;
    dev->type = SMARTSSD_PROTO_UNKNOWN;
    dev->sata_drive = NULL;
    //dev->nvme_drive = NULL;

    printf("Opening SATA disk...\n");
    if (sk_disk_open(full_path, &dev->sata_drive) == 0) {
        dev->type = SMARTSSD_PROTO_ATA;
        return 0;
    }
    printf("SATA disk not found.\n");

    /*nvme_root_t root = nvme_scan(NULL);
    if (!root) {
        printf("Failed to scan NVMe tree.\n");
    }

    nvme_ctrl_t ctrls = nvme_scan_ctrls(root);
    if (!ctrl) {
        printf("Controller not found for %s\n", device);
    } else {
        dev->type = SMARTSSD_PROTO_NVME;
        dev->nvme_drive = ctrl;
    }

    nvme_free_tree(root);*/

    printf("Unknown or unsupported device protocol\n");
    return -1;
}

int smartssd_deinit(smartssd *dev) {
    if (dev->type == SMARTSSD_PROTO_ATA) {
        sk_disk_free(dev->sata_drive);
        dev->sata_drive = NULL;
    } /*else if (dev->type == SMARTSSD_PROTO_NVME) {
        nvme_close(dev->nvme_drive);
        dev->nvme_drive = NULL;
    }*/
}