#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define NUM_READS_PER_CYCLE 14
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)
#define MAX_IO_SIZE MB

int main(int argc, char *argv[]) {

    char *drive = argv[1];
    char *out = argv[2];

    int out_fd;
    if ((out_fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        printf("Data validation output file could not be opened.\n");
        exit(1);
    }

    void *buf;
    if (posix_memalign(&buf, MAX_IO_SIZE, GB) != 0) {
        printf("Failed to allocate aligned buffer.\n");
        exit(1);
    }

    int drive_fd;
    if ((drive_fd = open(drive, O_RDONLY | O_DIRECT)) < 0) {
        printf("Unable to access drive.\n");
        close(out_fd);
        free(buf);
        exit(1);
    }

    ssize_t total_read = 0;
    while (total_read < GB) {
        ssize_t num_read = pread(drive_fd, (char *)buf + total_read, MAX_IO_SIZE, total_read);
        if (num_read < 0) {
            printf("Read error, aborting.\n");
            close(drive_fd);
            free(buf);
            exit(1);
        }
        total_read += num_read;
    }

    int w_total = 0;
    while (w_total < GB) {
        w_total += write(out_fd, buf + w_total, MB);
    }

}