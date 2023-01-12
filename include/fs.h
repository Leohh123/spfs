#ifndef __SPFS_FS_H__
#define __SPFS_FS_H__

#include "block.h"

#include <string.h>

#define SPFS_DIRNUM 128
#define SPFS_DIR_ITEM_FILE 0x12
#define SPFS_DIR_ITEM_DIR 0x34

#define MASK64 0xffffffffffffffffULL

struct spfs_dir_item {
    uint16_t blkno;
    uint8_t type;
    char name[29];
};

struct spfs_dir {
    struct spfs_dir_item items[SPFS_DIRNUM];
};

static int is_empty_dir(uint16_t dir_blkno) {
    struct spfs_dir *pd;
    spfs_read_block((void **)&pd, dir_blkno);

    for (int i = 0; i < SPFS_DIRNUM; i++) {
        if (pd->items[i].blkno) {
            spdk_free(pd);
            return 0;
        }
    }

    spdk_free(pd);
    return 1;
}

static void list_dir(uint16_t dir_blkno) {
    // Read directory
    struct spfs_dir *pd;
    spfs_read_block((void **)&pd, dir_blkno);

    // Enumerate each item
    printf("---------------- ls %d start ----------------\n", dir_blkno);
    for (int i = 0; i < SPFS_DIRNUM; i++) {
        if (pd->items[i].blkno) {
            printf("<%s>\t\t%s\t\tBlkno: %u\n",
                   pd->items[i].type == SPFS_DIR_ITEM_DIR ? "DIR" : "FILE",
                   pd->items[i].name,
                   pd->items[i].blkno);
        }
    }
    printf("---------------- ls %d end ----------------\n", dir_blkno);
}

static void disp_blk(uint16_t blkno) {
    // Read content
    char *content;
    spfs_read_block(&content, blkno);

    // Display
    printf("---------------- disp %d start ----------------\n", blkno);
    for (int i = 0; i < SPFS_BLKSZ; i++) {
        if (33 <= content[i] && content[i] <= 126) {
            printf("%c", content[i]);
        } else {
            printf("(%d)", content[i]);
        }
    }
    printf("\n---------------- disp %d end ----------------\n", blkno);

    spdk_free(content);
}

static void disp_str(uint16_t blkno) {
    // Read content
    char *content;
    spfs_read_block(&content, blkno);

    // Display
    printf("---------------- cat %d start ----------------\n", blkno);
    printf("%s\n", content);
    printf("---------------- cat %d end ----------------\n", blkno);

    spdk_free(content);
}

static int clear_block(int blkno) {
    void *tmp = malloc(SPFS_BLKSZ);
    memset(tmp, 0, SPFS_BLKSZ);
    spfs_write_block(tmp, blkno);
    free(tmp);
}

static int spfs_mkfs(void) {
    void *tmp = malloc(SPFS_BLKSZ);

    // Empty root dir
    memset(tmp, 0, SPFS_BLKSZ);
    spfs_write_block(tmp, 1);

    // Empty data bitmap (except bitmap itself and root dir)
    ((uint8_t *)tmp)[0] = 0x03;
    spfs_write_block(tmp, 0);

    free(tmp);
    return 0;
}

static uint16_t get_free_blkno() {
    // Read bitmap
    uint64_t *map;
    uint16_t res = 0;
    spfs_read_block(&map, 0);

    // Find free block with smallest No.
    for (uint16_t i = 0; i < (SPFS_BLKSZ >> 6); i++) {
        if (map[i] != MASK64) {
            for (uint16_t j = 0; j < 64; j++) {
                if (!((map[i] >> j) & 1)) {
                    res = i * 64 + j;
                    // printf("get_free_blkno: %d\n", res);
                    goto __exit;
                }
            }
        }
    }

__exit:
    spdk_free(map);
    if (res == 0) {
        printf("ERROR: out of storage.\n");
        exit(1);
    }
    return res;
}

static int update_bitmap(uint16_t blkno, uint64_t val) {
    // Read bitmap
    uint64_t *map;
    uint16_t res = 0;
    spfs_read_block(&map, 0);

    // Calculate position
    int row = blkno >> 6;
    int col = blkno & 0x3f;

    // Update
    if (val) {
        map[row] |= (1ULL << col);
    } else {
        map[row] &= ~(1ULL << col);
    }
    spfs_write_block(map, 0);

    // printf("update_bitmap: blkno = %d\n", blkno);
    // disp_blk(0);

    spdk_free(map);
    return 0;
}

static int create_item(uint16_t dir_blkno, uint8_t type, char *name) {
    // printf("create_item: dir_blkno = %d, type = %d, name = %s\n", dir_blkno, type, name);
    // Read directory
    struct spfs_dir *pd;
    spfs_read_block((void **)&pd, dir_blkno);

    // Try to append the item to the dir
    int blkno = 0;
    for (int i = 0; i < SPFS_DIRNUM; i++) {
        if (pd->items[i].blkno) {
            if (strcmp(pd->items[i].name, name) == 0) {
                printf("ERROR: cannot use the same name.\n");
                exit(1);
            }
        } else {
            blkno = get_free_blkno();
            pd->items[i].blkno = blkno;
            pd->items[i].type = type;
            strcpy(pd->items[i].name, name);
            break;
        }
    }

    // Update bitmap and dir block, clear new block
    int rc = 0;
    if (blkno) {
        clear_block(blkno);
        update_bitmap(blkno, 1);
        spfs_write_block(pd, dir_blkno);
    } else {
        printf("ERROR: out of directory capacity.\n");
        rc = 1;
    }
    spdk_free(pd);
    return rc;
}

static struct spfs_dir_item find_item(uint16_t dir_blkno, char *name) {
    // printf("find_item: blkno = %d, name = %s\n", dir_blkno, name);
    struct spfs_dir_item item;
    memset(&item, 0, sizeof(item));

    // Read directory
    struct spfs_dir *pd;
    spfs_read_block((void **)&pd, dir_blkno);

    // Find the item
    for (int i = 0; i < SPFS_DIRNUM; i++) {
        if (pd->items[i].blkno && strcmp(pd->items[i].name, name) == 0) {
            memcpy(&item, &pd->items[i], sizeof(item));
            break;
        }
    }

    spdk_free(pd);
    return item;
}

static int remove_item(uint16_t dir_blkno, char *name) {
    // Read directory
    struct spfs_dir *pd;
    spfs_read_block((void **)&pd, dir_blkno);

    // Find the item
    int rc = 1, blkno = 0;
    for (int i = 0; i < SPFS_DIRNUM; i++) {
        if (strcmp(pd->items[i].name, name) == 0) {
            // Cannot delete a non-empty dir
            if (pd->items[i].type == SPFS_DIR_ITEM_DIR &&
                !is_empty_dir(pd->items[i].blkno)) {
                rc = 2;
                break;
            }

            blkno = pd->items[i].blkno;
            rc = 0;

            // Clear dir item
            memset(&pd->items[i], 0, sizeof(struct spfs_dir_item));

            break;
        }
    }

    if (rc == 1) {
        printf("ERROR: item not found.\n");
    } else if (rc == 2) {
        printf("ERROR: cannot remove a non-empty directory.\n");
    } else {
        // Write back and update bitmap
        spfs_write_block(pd, dir_blkno);
        update_bitmap(blkno, 0);
    }

    spdk_free(pd);
    return rc;
}

static struct spfs_dir_item resolve_path(char *path) {
    struct spfs_dir_item item;
    memset(&item, 0, sizeof(item));

    if (path[0] != '/') {
        printf("ERROR: path should start with '/'.\n");
        return item;
    }

    strcpy(item.name, "/");
    item.blkno = 1;
    item.type = SPFS_DIR_ITEM_DIR;

    char tmp[50];
    int pos = 0;
    for (int i = 1, len = strlen(path); i <= len; i++) {
        if (path[i] == '/' || path[i] == '\0') {
            if (pos) {
                tmp[pos] = '\0';
                item = find_item(item.blkno, tmp);
                pos = 0;
            }
        } else {
            tmp[pos++] = path[i];
        }
    }
    return item;
}

#endif