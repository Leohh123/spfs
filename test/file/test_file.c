#include "io.h"

int main(int argc, char **argv) {
    spfs_init(argc, argv);

    struct spfs_file *file;
    file = spfs_fopen("/a/b.txt", "w");
    char tmp[] = "0123456789abcdefghijklmn";
    spfs_fwrite(tmp, sizeof(tmp), 1, file);
    spfs_fclose(file);

    file = spfs_fopen("/a/b.txt", "r");
    char buf[100];
    memset(buf, 0, sizeof(buf));
    spfs_fread(buf, 4, 1, file);
    printf("%s\n", buf);
    spfs_fseek(file, 4, SPFS_SEEK_CUR);
    memset(buf, 0, sizeof(buf));
    spfs_fread(buf, 4, 1, file);
    printf("%s\n", buf);
    spfs_fclose(file);

    spfs_finish();
    printf("Test passed.\n");

    return 0;
}
