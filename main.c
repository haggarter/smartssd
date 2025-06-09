#include "smartssd.h"

int main(int argv, char *argc[]) {
    
    if (argv != 3) {
        printf("Too few arguments.\nUsage: ./query_smart [string: drive name] [int: number of cycles]\n");
        exit(1);
    }
    
    smartssd dev;
    return 0;
}