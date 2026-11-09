#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <basicgol.h>
#include <basicgol/filesystem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int walk_count;
static int walk_cb(const char *path, int is_dir, void *ctx) {
    (void)path; (void)is_dir; (void)ctx;
    walk_count++;
    return 0;
}

int main(void) {
    // temp file
    char tmp[256];
    if (bg_fs_make_temp(tmp, sizeof(tmp)) != 0) { printf("FAIL temp\n"); return 1; }
    FILE *f = fopen(tmp, "w");
    fputs("hello", f);
    fclose(f);

    // copy
    char dst[512];
    snprintf(dst, sizeof(dst), "%s.copy", tmp);
    if (bg_fs_copy_file(tmp, dst) != 0) { printf("FAIL copy\n"); return 1; }
    FILE *g = fopen(dst, "r");
    char buf[16]; fgets(buf, sizeof(buf), g); fclose(g);
    if (strcmp(buf, "hello") != 0) { printf("FAIL copy content\n"); return 1; }

    // walk
    // create dir with file
#ifdef _WIN32
    CreateDirectoryA("tst_fs", NULL);
    CreateDirectoryA("tst_fs/sub", NULL);
#else
    mkdir("tst_fs", 0777);
    mkdir("tst_fs/sub", 0777);
#endif
    FILE *h = fopen("tst_fs/a.txt", "w"); fputs("x", h); fclose(h);
    walk_count = 0;
    if (bg_fs_walk("tst_fs", walk_cb, NULL) != 0) { printf("FAIL walk\n"); return 1; }
    if (walk_count < 3) { printf("FAIL walk count %d\n", walk_count); return 1; }

    printf("OK\n");
    return 0;
}
