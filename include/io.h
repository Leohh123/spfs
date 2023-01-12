#ifndef __SPFS_IO_H__
#define __SPFS_IO_H__

#include "fs.h"

#include <stdlib.h>

#define SPFS_FILE_WRITE 0x01
#define SPFS_FILE_READ 0x02

#define SPFS_SEEK_SET 0
#define SPFS_SEEK_CUR 1
#define SPFS_SEEK_END 2

struct spfs_file {
    char *buf;
    uint16_t blkno;
    int mode;
    int cursor;
    int end;
};

struct spfs_file *spfs_fopen(const char *filename, const char *mode);
int spfs_fclose(struct spfs_file *stream);
size_t spfs_fread(void *ptr, size_t size, size_t nmemb, struct spfs_file *stream);
size_t spfs_fwrite(const void *ptr, size_t size, size_t nmemb, struct spfs_file *stream);
int spfs_fseek(struct spfs_file *stream, long offset, int whence);

struct spfs_file *spfs_fopen(const char *filename, const char *mode) {
    struct spfs_file *file = malloc(sizeof(struct spfs_file));
    file->mode = 0;

    struct spfs_dir_item item = resolve_path(filename);
    if (item.blkno == 0) {
        printf("ERROR: the file must exists.\n");
        exit(1);
    }

    file->blkno = item.blkno;
    file->end = 0;
    spfs_read_block(&file->buf, file->blkno);
    while (file->end + 1 < SPFS_BLKSZ && file->buf[file->end] != '\0') {
        file->end++;
    }

    if (mode[0] == 'r') {
        file->mode |= SPFS_FILE_READ;
        file->cursor = 0;
    } else if (mode[0] == 'w') {
        file->mode |= SPFS_FILE_WRITE;
        clear_block(file->blkno);
        file->cursor = 0;
    } else if (mode[0] == 'a') {
        file->mode |= SPFS_FILE_WRITE;
        file->cursor = file->end;
    } else {
        printf("ERROR: invalid mode.\n");
        exit(1);
    }

    if (mode[1] == '+') {
        file->mode |= SPFS_FILE_READ | SPFS_FILE_WRITE;
    }

    return file;
}

int spfs_fclose(struct spfs_file *stream) {
    spdk_free(stream->buf);
    free(stream);
    return 0;
}

size_t spfs_fread(void *ptr, size_t size, size_t nmemb, struct spfs_file *stream) {
    if (!(stream->mode & SPFS_FILE_READ)) {
        printf("ERROR: no read permission.\n");
        exit(1);
    }

    int i;
    for (i = 0; i < nmemb; i++) {
        if (stream->cursor + size <= stream->end) {
            memcpy(ptr, stream->buf + stream->cursor, size);
            ptr += size;
            stream->cursor += size;
        } else {
            break;
        }
    }

    return i;
}

size_t spfs_fwrite(const void *ptr, size_t size, size_t nmemb, struct spfs_file *stream) {
    if (!(stream->mode & SPFS_FILE_WRITE)) {
        printf("ERROR: no write permission.\n");
        exit(1);
    }

    int i;
    for (i = 0; i < nmemb; i++) {
        if (stream->cursor + size <= SPFS_BLKSZ) {
            memcpy(stream->buf + stream->cursor, ptr, size);
            ptr += size;
            stream->cursor += size;
            if (stream->cursor > stream->end) {
                stream->end = stream->cursor;
            }
        } else {
            break;
        }
    }

    spfs_write_block(stream->buf, stream->blkno);

    return i;
}

int spfs_fseek(struct spfs_file *stream, long offset, int whence) {
    int to = stream->cursor;
    if (whence == SPFS_SEEK_SET) {
        to = 0;
    } else if (whence == SPFS_SEEK_END) {
        to = stream->end;
    }
    to += offset;
    if (to < 0 || to > stream->end) {
        return 1;
    }
    stream->cursor = to;
    return 0;
}

#endif