#define _POSIX_C_SOURCE 200809L
#include <basicgol/filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

int bg_fs_copy_file(const char *src, const char *dst) {
    if (!src || !dst) return -1;
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return -1; }
    }
    if (ferror(in)) { fclose(in); fclose(out); return -1; }
    fclose(in);
    if (fclose(out) != 0) return -1;
    return 0;
}

int bg_fs_make_temp(char *buf, size_t sz) {
    if (!buf || sz < 16) return -1;
#ifdef _WIN32
    char tmp[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmp);
    if (len == 0 || len > MAX_PATH) return -1;
    if (GetTempFileNameA(tmp, "bg", 0, buf) == 0) return -1;
    return 0;
#else
    strcpy(buf, "/tmp/bg_XXXXXX");
    int fd = mkstemp(buf);
    if (fd == -1) return -1;
    close(fd);
    return 0;
#endif
}

static int walk_dir(const char *root, int (*cb)(const char*,int,void*), void *ctx) {
#ifdef _WIN32
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", root);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s", root, fd.cFileName);
        int is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        if (cb(path, is_dir, ctx) != 0) { FindClose(h); return -1; }
        if (is_dir) {
            if (walk_dir(path, cb, ctx) != 0) { FindClose(h); return -1; }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return 0;
#else
    DIR *d = opendir(root);
    if (!d) return -1;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", root, e->d_name);
        struct stat st;
        if (stat(path, &st) != 0) { closedir(d); return -1; }
        int is_dir = S_ISDIR(st.st_mode);
        if (cb(path, is_dir, ctx) != 0) { closedir(d); return -1; }
        if (is_dir) {
            if (walk_dir(path, cb, ctx) != 0) { closedir(d); return -1; }
        }
    }
    closedir(d);
    return 0;
#endif
}

int bg_fs_walk(const char *root, int (*cb)(const char *path, int is_dir, void *ctx), void *ctx) {
    if (!root || !cb) return -1;
    // notify root itself as dir
    if (cb(root, 1, ctx) != 0) return -1;
    return walk_dir(root, cb, ctx);
}
