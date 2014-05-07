#include "fuse_impl.hpp"
using namespace std;

// pake filesystem yg ada di main.cpp
extern CCFS filesystem;

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
			stbuf->st_mode = S_IFREG | (0660 + (entry.getAttr() & 0x7));
		}
		
		// ukuran file
		stbuf->st_size = entry.getSize();
		
		// waktu pembuatan file
		stbuf->st_mtime = entry.getDateTime();
		
		return 0;
	}
}

/* membaca direktori: mendaftar file & direktori dalam suatu path */
int ccfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
	// current & parent directory
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	Entry entry = Entry(0,0).getEntry(path);
	ptr_block index = entry.getIndex();
	entry = Entry(index,0);
	
	// fungsi filler digunakan untuk setiap entry pada direktori tsb
	// ditulis ke buffer "buf"
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
	/* mencari parent directory */
	int i;
	for(i = strlen(path)-1;path[i]!='/';i--);
	
	string parentPath = string(path, i);
	
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
	entry.setAttr(0x0F);
	entry.setCurrentDateTime();
	entry.setIndex(filesystem.allocateBlock());
	entry.setSize(0x00);

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

/* menamai direktori */
int ccfs_rename(const char* path, const char* newpath) {
	
	Entry entryAsal = Entry(0,0).getEntry(path);
	Entry entryLast = Entry(0,0).getNewEntry(newpath);
	if(!entryAsal.isEmpty()){
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
	
	return 0;
}

/* menghapus file */
int ccfs_unlink(const char *path) {
	Entry entry = Entry(0,0).getEntry(path);
	if(entry.getAttr() & 0x8){
		return -ENOENT;
	}
	else{
		filesystem.freeBlock(entry.getIndex());
		entry.makeEmpty();
	}
	return 0;
}

/* buat bikin file */
int ccfs_mknod(const char *path, mode_t mode, dev_t dev) {
	/* mencari parent directory */
	int i;
	for(i = strlen(path)-1;path[i]!='/';i--);
	
	string parentPath = string(path, i);
	
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
	entry.setAttr(0x06);
	entry.setTime(0x00);
	entry.setCurrentDateTime();
	entry.setIndex(filesystem.allocateBlock());
	entry.setSize(0x00);

	entry.write();

	return 0;
}

/* set file size */
int ccfs_truncate(const char *path, off_t newsize) {
	Entry entry = Entry(0, 0).getEntry(path);
	
	/* set sizenya */
	entry.setSize(newsize);
	entry.write();
	
	/* urusin allocation table */
	ptr_block position = entry.getIndex();
	while (newsize > 0) {
		newsize -= BLOCK_SIZE;
		if (newsize > 0) {
			/* kalau gak cukup, alokasiin baru */
			if (filesystem.nextBlock[position] == END_BLOCK) {
				filesystem.setNextBlock(position, filesystem.allocateBlock());
			}
			position = filesystem.nextBlock[position];
		}
	}
	filesystem.freeBlock(filesystem.nextBlock[position]);
	filesystem.setNextBlock(position, END_BLOCK);
	
	return 0;
}

/* read buffer dari filesystem */
int ccfs_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	//menuju ke entry
	Entry entry = Entry(0,0).getEntry(path);
	ptr_block index = entry.getIndex();
	
	//kalo namanya kosong
	if(entry.isEmpty()){
		return -ENOENT;
	}
	
	//read
	return filesystem.readBlock(index,buf,size,offset);
	
}

/* write buffer ke filesystem */
int ccfs_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	Entry entry = Entry(0,0).getEntry(path);
	ptr_block index = entry.getIndex();
	
	//kalo namanya kosong
	if(entry.isEmpty()){
		return -ENOENT;
	}
	
	entry.setSize(offset + size);
	entry.write();
	
	int result = filesystem.writeBlock(index, buf, size, offset);
	
	return result;
}

/* copy file */
int ccfs_link(const char *path, const char *newpath) {
	Entry oldentry = Entry(0,0).getEntry(path);
	
	/* kalo nama kosong */
	if(oldentry.isEmpty()){
		return -ENOENT;
	}
	/* buat entry baru dengan nama newpath */
	Entry newentry = Entry(0,0).getNewEntry(newpath);
	/* set atribut untuk newpath */
	newentry.setAttr(oldentry.getAttr());
	newentry.setCurrentDateTime();
	newentry.setSize(oldentry.getSize());
	newentry.write();
	
	/* copy isi file */
	char buffer[4096];
	/* lakukan per 4096 byte */
	int totalsize = oldentry.getSize();
	int offset = 0;
	while (totalsize > 0) {
		int sizenow = totalsize;
		if (sizenow > 4096) {
			sizenow = 4096;
		}
		filesystem.readBlock(oldentry.getIndex(), buffer, oldentry.getSize(), offset);
		filesystem.writeBlock(newentry.getIndex(), buffer, newentry.getSize(), offset);
		totalsize -= sizenow;
		offset += 4096;
	}
	
	return 0;
}
