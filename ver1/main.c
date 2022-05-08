#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zsuck.h"

void dumpfile(const char *n, void *d, uint32_t l) {
    FILE *f = fopen(n,"wb");
    fwrite(d,1,l,f);
    fclose(f);
}

void *readfile(const char *n, uint32_t *l) {
    FILE *f = fopen(n, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
    uint8_t *d = malloc(fsize + 1);
    fread(d, 1, fsize, f);
    fclose(f);
    *l = fsize;
    return d;
}

#define MAX_OFILE (3*1024*1024)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Need a filename.");
        return -1;
    } 
    uint32_t file_size = 0;
    void *d = readfile(argv[1], &file_size);

    uint8_t o[MAX_OFILE];
    uint32_t osize = 0;
    uint32_t encode_overflow = zsuck_encode(o, MAX_OFILE, &osize, d, file_size);

    char oname[255];
    strcpy(oname,argv[1]);
    strcat(oname,".zenc");

    dumpfile(oname, o, osize);
    return 0;
}



