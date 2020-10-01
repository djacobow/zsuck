#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zsuck.h"

int myrand(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void *make_randblob(uint32_t len, uint16_t zblobs, uint32_t zmax) {
    uint8_t *op = malloc(len);
    if (!op) return 0;
    uint8_t *p = op;
    while (p < op + len) {
        *p = rand() & 0xff;
        p++; 
    }
    
    uint16_t bcount = zblobs;
    while (bcount) {
        uint16_t bloblen   = myrand(0, zmax);
        uint32_t blobstart = myrand(0, len);
        if ((blobstart + bloblen) < len) {
            memset(op + blobstart, 0, bloblen);
            bcount--; 
        }
    }
    return (void *)op;
}

void dumpfile(const char *n, void *d, uint32_t l) {
    FILE *f = fopen(n,"wb");
    fwrite(d,1,l,f);
    fclose(f);
}

void dump_raw(void *p, uint32_t l) {
    uint8_t *pp = (uint8_t *)p;
    uint32_t in_zero = 0;
    while (pp < (uint8_t *)p + l) {
        if (*pp) {
            if (in_zero) {
                printf("\nZERO -- count 0x%08x\n%02x",in_zero,*pp);
                in_zero = 0;
            } else {
                printf("%02x",*pp);
            }
        } else {
            in_zero++;
        }
        pp++;
    }
}

void dump_encoded(void *p, uint32_t l) {
    uint8_t *pp = (uint8_t *)p;
    while (pp < (uint8_t *)p + l) {
        if (*pp) {
            printf("%02x",*pp++);
        } else {
            pp++;
            printf("\nZERO ENC -- %02x%02x%02x%02x\n",pp[0],pp[1],pp[2],pp[3]);
            pp += sizeof(ZSUCK_ZC_TYPE);
        }
    }
}

void mycmp(void *x, void *y, uint32_t l) {
    uint8_t *ux = (uint8_t *)x;
    uint8_t *uy = (uint8_t *)y;

    for (uint32_t i=0;i<l;i++) {
        if (ux[i] != uy[i])  {
            printf("mismatch %u: %02x != %02x\n",i,ux[i],uy[i]);
        }
    }
}
#define MAX_E_SIZE (1024 * 1024)
#define IN_SIZE    (1024 * 1024)


int test1() {
    uint32_t MAX_CHUNK_SIZE = MAX_E_SIZE / 64;

    for (uint32_t i=0;i<400; i++) {
        void *i0 = make_randblob(IN_SIZE, 256, 8192);
        // dumpfile("i0.bin",i0,IN_SIZE); 
        // dump_raw(i0,IN_SIZE);

        uint8_t e0[MAX_E_SIZE];
        uint32_t act_encoded_size = 0;
        uint32_t encode_overflow = zsuck_encode(e0, MAX_E_SIZE, &act_encoded_size, i0, IN_SIZE);
        if (encode_overflow) {
            printf("Error: overflow in output buffer\n");
            return -2;
        }
        // dump_encoded(e0,act_encoded_size);
        // dumpfile("e0.bin",e0,act_encoded_size);

        uint8_t d0[IN_SIZE];

        zsuck_decode_context_t zcc = {};
        uint32_t act_decoded_written = 0;
        uint8_t *e_ptr = e0;
        while (e_ptr < e0 + act_encoded_size) {
            uint32_t encoded_remaining = e0 + act_encoded_size - e_ptr;
            uint32_t decode_step_size = MAX_CHUNK_SIZE > encoded_remaining ? encoded_remaining : MAX_CHUNK_SIZE;

            uint32_t newly_decoded = 0;
            uint32_t decode_chunk_overflow = zsuck_decode_chunk(
                d0 + act_decoded_written,
                MAX_E_SIZE - act_decoded_written,
                &newly_decoded,
                e_ptr,
                decode_step_size,
                &zcc
            );
            if (decode_chunk_overflow) {
                printf("Error: decode output overflow\n");
                return -1;
            }
            act_decoded_written += newly_decoded;
            e_ptr += decode_step_size;

            encoded_remaining = e0 + act_encoded_size - e_ptr;
        }
        int mismatch = memcmp(i0, d0, IN_SIZE);
        if (mismatch) {
            printf("Error: crap!\n");
            mycmp(i0, d0, IN_SIZE);
            dump_raw(d0,IN_SIZE);
            return -1;
        }
        printf("Size: before %u after %u percent %.2f%%\n",IN_SIZE,act_encoded_size,
            ((double)(100 * act_encoded_size) / (double)IN_SIZE));
        free(i0);
    }
    return 0;
}



int test0() {
    for (uint32_t i=0; i<400; i++) {
        void *b0 = make_randblob(IN_SIZE, 128, 128*1024);
        // dump_raw(b0,IN_SIZE);
        // dumpfile("i0.bin",b0,IN_SIZE); 
        uint8_t e0[MAX_E_SIZE];
        uint32_t act_e_size = 0;
        uint32_t encode_overflow = zsuck_encode(e0, MAX_E_SIZE, &act_e_size, b0, IN_SIZE);
        if (encode_overflow) {
            printf("Error: overflow in output buffer (encode)\n");
            return -2;
        }
        // dumpfile("e0.bin",e0,act_e_size); 
        // dump_encoded(e0,act_e_size);
        uint8_t d0[MAX_E_SIZE];
        uint32_t act_d_size = 0;
        uint32_t decode_overflow = zsuck_decode(d0, MAX_E_SIZE, &act_d_size, e0, act_e_size);
        if (decode_overflow) {
            printf("Error: overflow in output buffer (decode)\n");
            return -3;
        }
        // dumpfile("d0.bin",d0,act_d_size); 
        if (act_d_size != IN_SIZE) {
            printf("e_size %u, o_size %u\n",act_e_size, act_d_size);
        }
        int mismatch = memcmp(b0, d0, IN_SIZE);
        if (mismatch) {
            printf("Error: crap! mismatch!\n");
            return -1;
        }
        printf("Size: before %u after %u percent %.2f%%\n",IN_SIZE,act_e_size,
            ((double)(100 * act_e_size) / (double)IN_SIZE));
        free(b0);
    }
    return 0;
}

int main(int arg, char *argv[]) {
    int errors = test0();
    errors += test1();
    return errors;
}
