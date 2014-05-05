#ifndef FUSE_IMPL_HPP
#define FUSE_IMPL_HPP

/** Header implementasi fungsi-fungsi fuse */

// pakai versi 26
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include "simplefs.hpp"

/* get attribute */
int simple_getattr(const char* path, struct stat* stbuf);

/* open file */
int simple_open(const char* path, struct fuse_file_info* fi);

/* mknod, untuk membuat file */
int simple_mknod(const char *path, mode_t mode, dev_t dev);

/* membaca direktori */
int simple_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);

/* untuk mengubah panjang suatu file, biasa digunakan saat penulisan pertama */
int simple_truncate(const char* path, off_t size);

/* membaca file */
int simple_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi);

/* menulis file */
int simple_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi);

/* rename/cut file */
int simple_rename(const char* from, const char* to);

/* hapus file */
int simple_unlink(const char* path);

#endif
