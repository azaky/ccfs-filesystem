#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/*
static int ccfs_getattr(
	const char *path,
	struct stat *stbuf);

static int ccfs_open(
	const char *path,
	struct fuse_file_info *fi);

static int ccfs_read(
	const char *path,
	const char *buf,
	size_t size,
	off_t offset,
	struct fuse_file_info *fi);

static int ccfs_readdir(
	const char *path,
	void *buf,
	fuse_fill_dir_t filler,
	off_t offset,
	struct fuse_file_info *fi);

static int ccfs_write(
	const char *path,
	const char *buf,
	size_t size,
	off_t offset,
	struct fuse_file_info *fi);

static int ccfs_truncate(
	const char * path,
	off_t size);

static int ccfs_mkdir(
	const char *path,
	mode_t mode);
	
static int ccfs_rmdir(
	const char *path);

static int ccfs_rename(
	const char *from,
	const char *to);

static int ccfs_unlink(
	const char *path);

static struct fuse_operations ccfs_oper = {
	.getattr	= ccfs_getattr,
	.open		= ccfs_open,
	.read		= ccfs_read,
	.readdir	= ccfs_readdir,
	.write		= ccfs_write,
	.truncate	= ccfs_truncate,
	.mkdir		= ccfs_mkdir,
	.rmdir		= ccfs_rmdir,
	.rename		= ccfs_rename,
	.unlink		= ccfs_unlink
};
*/



int main(int argc, char *argv[])
{
	
	return 0;
	
	return fuse_main(argc, argv, &ccfs_oper, NULL);
}
