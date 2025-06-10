#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <cjson/cJSON.h>

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

    if (debug)
        printf("Checking if drive exists...");

    struct stat st;
    if (stat(path, &buffer) < 0) {
        printf("Drive not found.\n");
        exit(1);
    }

    if (debug)
        printf("Drive exists.\n")

    int cycles;

    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n%s", usage);
        exit(1);
    }

    if (debug)
        printf("Cycles: %d\n", cycles);

    if (debug)
        printf("Args ok.\n");
    
    

    return 0;
}