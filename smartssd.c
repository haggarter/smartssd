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

    nvme_root_t root = nvme_scan(NULL);
    if (!r) {
        printf("Failed to scan NVMe devices.\n");
        return -1;
    }

    for (nvme_ctrl_t controller = nvme_root_ctrls(root); controller; controller = nvme_ctrl_next(controller)) {
        for (nvme_ns_t ns = nvme_ctrl_namespaces(controller); ns; ns = nvme_ns_next(ns)) {
            const char *device = nvme_ns_get_name(ns);
            if (device && strcmp(device, drive + 5) == 0) {
                dev->nvme_drive = nvme_open(drive);
                if (dev->nvme_drive) {
                    dev->type = SMARTSSD_PROTO_NVME;
                    nvme_free_tree(root);
                    return 0;
                } else {
                    nvme_free_tree(root);
                    return -1;
                }
                return 0;
            }
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