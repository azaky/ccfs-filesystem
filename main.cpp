#include <iostream>
#include "fuse_impl.hpp"
#include "ccfs.hpp"

using namespace std;

struct fuse_operations ccfs_op;

CCFS filesystem;

void init_fuse(){
	ccfs_op.getattr	= ccfs_getattr;
	ccfs_op.readdir	= ccfs_readdir;
	ccfs_op.mkdir	= ccfs_mkdir;
	ccfs_op.open 	= ccfs_open;
	ccfs_op.rmdir 	= ccfs_rmdir;
	ccfs_op.rename	= ccfs_rename;
	ccfs_op.unlink	= ccfs_unlink;
	ccfs_op.mknod	= ccfs_mknod;
}

int main(int argc, char** argv){
	
	// jika terdapat argumen --new, buat baru
	if (argc > 3 && string(argv[3]) == "--new") {
		filesystem.create(argv[2]);
	}
	
	filesystem.load(argv[2]);
	
	// inisialisasi fuse
	CCFS_state *CCFS_data;
	CCFS_data = (CCFS_state*) malloc(sizeof(CCFS_state));
    CCFS_data->rootdir = realpath(argv[1], NULL);
    
	int fuse_argc = 3; 
	char* fuse_argv[3] = {argv[0], "-d", argv[1]};
	
	init_fuse();
	
	// serahkan ke fuse
	return fuse_main(fuse_argc, fuse_argv, &ccfs_op, CCFS_data);
}

/*
#include <iostream>
#include "fuse_impl.hpp"

struct fuse_operations ccfs_op;
ccfsFS filesystem;

void init_fuse(){
	ccfs_op.getattr	= ccfs_getattr;
	ccfs_op.open		= ccfs_open;
	ccfs_op.read		= ccfs_read;
	ccfs_op.write		= ccfs_write;
	ccfs_op.truncate	= ccfs_truncate;
	ccfs_op.rename	= ccfs_rename;
	ccfs_op.readdir	= ccfs_readdir;
	ccfs_op.mknod		= ccfs_mknod;
	ccfs_op.unlink	= ccfs_unlink;
}

using namespace std;

int main(int argc, char** argv) {
	
	// jika terdapat argumen --new, buat baru
	if (argc > 3 && string(argv[3]) == "--new")
		filesystem.create(argv[2]);
	else
		filesystem.load(argv[2]);
		
	
	// inisialisasi fuse
	int fuse_argc = 2; 
	char* fuse_argv[2] = {argv[0], argv[1]};
	
	init_fuse();
	
	// serahkan ke fuse
	return fuse_main(fuse_argc, fuse_argv, &ccfs_op, NULL);
}
*/
