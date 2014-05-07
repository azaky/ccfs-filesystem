#include "fuse_impl.hpp"
#include <iostream>
using namespace std;
// untuk linux error number
#include <cerrno>

// pake filesystem yg ada di main.cpp
extern CCFS filesystem;

void getFullPath(char *fpath, const char *path)
{
//	fprintf(stderr, "Halo\n");
//	fprintf(stderr, "rootdir = %s\n", CCFS_DATA->rootdir);
	strcpy(fpath, CCFS_DATA->rootdir);
//	fprintf(stderr, "Halo lagi\n");
	strncat(fpath, path, PATH_MAX);
}

/************ Implementasi FUSE ***************/

/* get attribute */
int ccfs_getattr(const char* path, struct stat* stbuf) {
	/* jika root path */
	if (string(path) == "/"){
		stbuf->st_nlink = 1;
		stbuf->st_mode = S_IFDIR | 0777; // file dengan permission rwxrwxrwx
		stbuf->st_mtime = filesystem.mount_time;
		return 0;
	}
	else {
		Entry entry = Entry(0, 0).getEntry(path);
		
		//Kalau path tidak ditemukan
		if (entry.isEmpty()) {
			return -ENOENT;
		}
		
		// tulis stbuf, tempat memasukkan atribut file
		stbuf->st_nlink = 1;
		
		// cek direktori atau bukan
		if (entry.getAttr() & 0x8) {
			stbuf->st_mode = S_IFDIR | (0770 + (entry.getAttr() & 0x7));
		}
		else {
			stbuf->st_mode = S_IFREG | (0770 + (entry.getAttr() & 0x7));
		}
		
		// ukuran file
		stbuf->st_size = entry.getSize();
		
		// waktu pembuatan file, asumsinya sama dengan waktu mounting
		stbuf->st_mtime = filesystem.mount_time; //ganti ya ntar
		
		return 0;
	}
}

/* membaca direktori: mendaftar file & direktori dalam suatu path */
int ccfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
	// selain root directory, error
	fprintf(stderr, "LAGI ccfs_readdir. PATH = %s\n", path);
/*
	if (string(path) != "/"){
		return -ENOENT;
	}
*/	
	// fungsi filler digunakan untuk setiap entry pada direktori tsb
	// ditulis ke buffer "buf"
	
	// current & parent directory
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	Entry entry = Entry(0,0).getEntry(path);
	ptr_block index = entry.getIndex();
	entry = Entry(index,0);
	while (entry.position != END_BLOCK) {
		if(!entry.isEmpty()){
			filler(buf, entry.getName().c_str(), NULL, 0);
		}
		entry = entry.nextEntry();
	}
	
	return 0;
}

/* membuat direktori */
int ccfs_mkdir(const char *path, mode_t mode) {
	fprintf(stderr, "LAGI ccfs_mkdir. PATH = %s\n", path);
	/* mencari parent directory */
	int i;
	for(i = strlen(path)-1;path[i]!='/';i--){
		
	}
	
	string parentPath = string(path, i);
	cout<<"parent Path = " << parentPath<<endl;
	
    Entry entry;
    //bagi kasus kalau dia root
    if (parentPath == "") {
		entry = Entry(0, 0);
	}
	else {
		entry = Entry(0,0).getEntry(parentPath.c_str());
		ptr_block index = entry.getIndex();
		entry = Entry(index, 0);
    }
    
    /* mencari entry kosong di parent */
    entry = entry.getNextEmptyEntry();
    
	/* menuliskan data di entry tersebut */
	entry.setName(path + i + 1);
	cout<<entry.getName()<<endl;
	entry.setAttr(0x0F);
	entry.setTime(0x00);
	entry.setDate(0x00);
	entry.setIndex(filesystem.allocateBlock());
	entry.setSize(0x800);

	entry.write();

	return 0;

	//jangan lupa urusin mode
}

/* memeriksa apakah file ada atau tidak */
int ccfs_open(const char* path, struct fuse_file_info* fi) {
	/* hanya mengecek apakah file ada atau tidak */

	Entry entry = Entry(0,0).getEntry(path);
	
	if(entry.isEmpty()) {
		return -ENOENT;
	}
	
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
	
	return 0;
}

/* menghapus direktori */
int ccfs_rmdir(const char *path) {
	/* mencari entry dengan nama path */
	Entry entry = Entry(0,0).getEntry(path);
	if(entry.isEmpty()){
		return -ENOENT;
	}
	
	/* masuk ke direktori dari indeks */
	/* menghapus dari tiap allocation table */
	filesystem.freeBlock(entry.getIndex());
	//removeDir(entry.getIndex());
	entry.makeEmpty();
	
	return 0;
}

void removeDir(ptr_block Alloc) {
	if(filesystem.nextBlock[Alloc] != 0xFFFF){
		removeDir(filesystem.nextBlock[Alloc]);
	}
	filesystem.setNextBlock(Alloc, EMPTY_BLOCK);
}

/* menamai direktori */
int ccfs_rename(const char* path, const char* newpath) {
	
	Entry entryAsal = Entry(0,0).getEntry(path);
	Entry entryLast = Entry(0,0).getNewEntry(newpath);
	if(!entryAsal.isEmpty()){
		printf("entry Last: [%s] (%d, %d)\n", entryLast.getName().c_str(), entryLast.position, entryLast.offset);
		//entryLast.setName(entryAsal.getName().c_str());
		entryLast.setAttr(entryAsal.getAttr());
		entryLast.setIndex(entryAsal.getIndex());
		entryLast.setSize(entryAsal.getSize());
		entryLast.setTime(entryAsal.getTime());
		entryLast.setDate(entryAsal.getDate());
		entryLast.write();
		/* set entry asal jadi kosong */
		entryAsal.makeEmpty();
	}
	else
		return -ENOENT;
}

/* ??? */
int ccfs_unlink(const char *path) {
	Entry entry = Entry(0,0).getEntry(path);
	if(entry.getAttr() & 0x8){
		return -ENOENT;
	}
	else{
		entry.makeEmpty();
	}
	return 0;
}
