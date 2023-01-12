#include "fs.h"

#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    spfs_init(argc, argv);
    spfs_mkfs();
    spfs_finish();
    printf("File system created successfully.\n");
    return 0;
}
