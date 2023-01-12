#include "fs.h"

#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERROR: invalid arguments.\n");
        return 1;
    }

    spfs_init(argc, argv);

    char tmp[50], target[50];
    memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, argv[1]);

    int len = strlen(tmp);
    int pos = len - 1;
    if (tmp[pos] == '/' && pos > 0) {
        tmp[pos] = '\0';
        pos--;
    }
    while (pos >= 0 && tmp[pos] != '/') {
        pos--;
    }

    if (pos == len - 1 || pos < 0 || strlen(tmp + pos + 1) == 0) {
        printf("ERROR: invalid arguments.\n");
        return 1;
    }
    strcpy(target, tmp + pos + 1);

    tmp[pos + 1] = '\0';

    struct spfs_dir_item item = resolve_path(tmp);
    if (item.blkno) {
        remove_item(item.blkno, target);
        printf("Delete successful.\n");
    } else {
        printf("ERROR: invalid path.\n");
    }

    spfs_finish();
    return 0;
}
