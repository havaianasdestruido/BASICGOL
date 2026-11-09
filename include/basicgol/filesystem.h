#ifndef BASICGOL_FILESYSTEM_H
#define BASICGOL_FILESYSTEM_H

#include <basicgol.h>
#include <stddef.h>

int bg_fs_copy_file(const char *src, const char *dst);
int bg_fs_make_temp(char *buf, size_t sz);
int bg_fs_walk(const char *root, int (*cb)(const char *path, int is_dir, void *ctx), void *ctx);

#endif
