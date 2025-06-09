#include <atasmart.h>
#include <libnvme.h>

typedef enum {
    SMARTSSD_PROTO_UNKNOWN,
    SMARTSSD_PROTO_ATA,
    SMARTSSD_PROTO_NVME
} protocol;

typedef struct smartssd {
    protocol type;
    char *path;
    struct nvme_dev *nvme_drive;
    SkDisk *sata_drive;
} smartssd;

int smartssd_init(smartssd *dev, char *drive);

int smartssd_deinit(smartssd *dev);

