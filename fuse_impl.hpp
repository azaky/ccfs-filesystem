#pragma once

/** Header implementasi fungsi-fungsi fuse */

// pakai versi 26
#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ccfs.hpp"

int ccfs_getattr(const char* path, struct stat* stbuf);

int ccfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int ccfs_mkdir(const char *path, mode_t mode);

int ccfs_open(const char* path, struct fuse_file_info* fi);

int ccfs_rmdir(const char *path);

int ccfs_rename(const char* path, const char* newpath);

int ccfs_unlink(const char *path);

int ccfs_mknod(const char *path, mode_t mode, dev_t dev);

int ccfs_truncate(const char *path, off_t newsize);

int ccfs_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi);

int ccfs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

int ccfs_link(const char *path, const char *newpath);
