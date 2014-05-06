#pragma once

/** Header implementasi fungsi-fungsi fuse */

// pakai versi 26
#define FUSE_USE_VERSION 26

#define PATH_MAX 100
struct CCFS_state {
    char *rootdir;
};
#define CCFS_DATA ((struct CCFS_state *) fuse_get_context()->private_data)

#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <string.h>

#include <fuse.h>
#include "ccfs.hpp"

int ccfs_getattr(const char* path, struct stat* stbuf);

int ccfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int ccfs_mkdir(const char *path, mode_t mode);

int ccfs_open(const char* path, struct fuse_file_info* fi);

int ccfs_rmdir(const char *path);

void removeDir(ptr_block Alloc);

int ccfs_rename(const char* path, const char* newpath);

int ccfs_unlink(const char *path);
