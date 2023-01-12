#include "block.h"

#include <stdlib.h>
#include <time.h>

char str[SPFS_BLKSZ];

static void rand_str(char *s) {
    for (int i = 0; i < SPFS_BLKSZ; i++) {
        s[i] = rand() % 94 + 33;
    }
    s[SPFS_BLKSZ - 1] = '\0';
}

static void rw_block(int blkno) {
    spfs_write_block(str, blkno);

    char *output;
    spfs_read_block((void **)&output, blkno);

    int r = strcmp(str, output);
    spdk_free(output);
    if (r != 0) {
        printf("Test failed.\n");
        exit(1);
    }
}

int main(int argc, char **argv) {
    spfs_init(argc, argv);
    srand(time(NULL));
    rand_str(str);
    for (int i = 0; i < (1 << 15); i++) {
        rw_block(i);
        if (((i >> 10) << 10) == i) {
            printf("blkno = %d\n", i);
        }
    }
    spfs_finish();
    printf("Test passed.\n");
    return 0;
}
