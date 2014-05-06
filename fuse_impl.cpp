#include "fuse_impl.hpp"

// untuk linux error number
#include <cerrno>

// pake filesystem yg ada di main.cpp
extern CCFS filesystem;

/************ Implementasi FUSE ***************/

/* get attribute */
int ccfs_getattr(const char* path, struct stat* stbuf){
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
		if (entry.getName() == "") {
			return -ENOENT;
		}
		
		// tulis stbuf, tempat memasukkan atribut file
		stbuf->st_nlink = 1;
		
		// cek direktori atau bukan
		if (entry.getAttr() & 0x8) {
			stbuf->st_mode = S_IFDIR | (0770 + entry.getAttr() & 0x7);
		}
		else {
			stbuf->st_mode = S_IFREG | (0770 + entry.getAttr() & 0x7);
		}
		
		// ukuran file
		stbuf->st_size = entry.getSize();
		
		// waktu pembuatan file, asumsinya sama dengan waktu mounting
		stbuf->st_mtime = filesystem.mount_time; //ganti ya ntar
		
		return 0;
	}
}

int ccfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi){
	// selain root directory, error
	if (string(path) != "/"){
		return -ENOENT;
	}
	
	// fungsi filler digunakan untuk setiap entry pada direktori tsb
	// ditulis ke buffer "buf"
	
	// current & parent directory
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	Entry entry(0, 0);
	while (!entry.isEmpty()) {
		filler(buf, entry.getName().c_str(), NULL, 0);
		entry = entry.nextEntry();
	}
	
	return 0;
}

int ccfs_mkdir(const char *path, mode_t mode)
{
	/* mencari parent directory */
	int i;
	for(i = strlen(path)-1;path[i]!='/';i--){
		
	}
	
	string parentPath = string(path, i);
	
    Entry entry = Entry(0,0).getEntry(parentPath);
    
    ptr_block index = entry.getIndex();
    entry = Entry(index, 0);
    
    /* mencari entry kosong di parent */
    while (!entry.isEmpty()) {
		entry = entry.nextEntry();
	}
	
	/* menuliskan data di entry tersebut */
	entry.setName(path + i);
	entry.setAttr(0x0F);
	entry.setTime(0x00);
	entry.setDate(0x00);
	entry.setIndex(filesystem.firstEmpty);
	while (filesystem.nextBlock[filesystem.firstEmpty] != 0x0000) {
		filesystem.firstEmpty++;
	}
	filesystem.nextBlock[filesystem.firstEmpty] = 0xFFFF;
    entry.setSize(0x00);
    
    entry.write();
    
    return 0;

/*    
    int retstat = 0;
    char fpath[PATH_MAX];

    AKMFS_fullpath(fpath, path);
    fprintf(stderr, "Path = %s\n", path);
    debug("Folder sedang dibuat, tekan enter untuk melanjutkan");
	// Get first free
  addresstype v_addr = (addresstype) (getFSFirstFreeAddress() & 0xffff);
  
  // File ini akan ditaruh di direktori mana dengan cara ambil alamat pertama 
  // block entry yang kosong
  unsigned int address = getAddFreeEntryFromPath(path);
  if (address==0) return -errno;

  // Taruh entry data
  attrtype attr = getAttributeType(0, 0, 0, 1);
  addEntryPathToAddress(path, address, attr, v_addr);
  
  // Ganti first free di VI ke free yang baru <-- ini gimana...
  changeFSFirstAddress(getNEWFSFirstAddress(v_addr));
  
  // Kurangi free capacity di VI
  changeFSFreeCapacity(getFSFreeCapacity()-1);
    retstat = mkdir(fpath, mode);
    if (retstat < 0)	retstat = -errno;
    
    return retstat;
*/
}
