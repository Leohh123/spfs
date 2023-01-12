#include "fs.h"

#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERROR: invalid arguments.\n");
        return 1;
    }

    spfs_init(argc, argv);

    struct spfs_dir_item item = resolve_path(argv[1]);
    if (item.blkno) {
        list_dir(item.blkno);
    } else {
        printf("ERROR: directory not exists.\n");
    }

    spfs_finish();
    return 0;
}
