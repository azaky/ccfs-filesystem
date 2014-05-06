#pragma once

/** Header implementasi fungsi-fungsi fuse */

// pakai versi 26
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include "ccfs.hpp"

int ccfs_getattr(const char* path, struct stat* stbuf);

int ccfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

