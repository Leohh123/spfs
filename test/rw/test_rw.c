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

static int cmp(char *s, char *p) {
    int cmpr = strcmp(s, p);
    printf("len_write = %lu, len_read = %lu, strcmp = %d\n",
           strlen(s), strlen(p), cmpr);
    return cmpr;
}

static void test_rw(int blkno) {
    srand(time(NULL));
    rand_str(str);

    spfs_write_block(str, blkno);
    printf("write: %s\n", str);
    printf("strlen: %lu\n", strlen(str));

    char *output;
    spfs_read_block((void **)&output, blkno);
    printf("read: %s\n", output);
    printf("strlen: %lu\n", strlen(output));

    int r = cmp(str, output);
    spdk_free(output);
    if (r != 0) {
        printf("Test failed.\n");
        exit(1);
    }
}

int main(int argc, char **argv) {
    spfs_init(argc, argv);
    test_rw(0);
    spfs_finish();
    printf("Test passed.\n");
    return 0;
}
