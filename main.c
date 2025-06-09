#include "smartssd.h"

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        printf("Too few arguments.\nUsage: ./query_smart [string: drive name] [int: number of cycles]\n");
        exit(1);
    }

    int cycles;

    if ((cycles = atoi(argv[2])) < 1) {
        printf("Cycles must be an integer greater than 0.\n");
        exit(1);
    }
    
    smartssd dev;
    return 0;
}