#include "fuse_impl.hpp"

// untuk linux error number
#include <cerrno>

// pake filesystem yg ada di main.cpp
extern SimpleFS filesystem;

/*  untuk menghilangkan / di depan path 
	karena setiap parameter path pada fungsi fuse merupakan path absolute dari root directory
	misal file abcd.txt pada root akan ditulis /abcd.txt
*/
string getPath(const char* path){
	return string(path + 1);
}

/* mencari index slot suatu file. -1 jika tidak ada, throw error jika nama terlalu panjang */
int searchFile (const char* fname){
	string name = getPath(fname);
	
	// validasi panjang nama file
	if (name.length() > 26){
		throw -ENAMETOOLONG;
	}
	
	// pencarian
	for (int i = 0; i < SLOT_NUM; i++){
		if (filesystem.files[i].name == name){
			return i;
		}
	}
	
	return -1;
}

/* mengalokasikan slot kosong baru untuk digunakan */
int nextFree(){
	// ambil firstEmpty saat ini
	int empty = filesystem.firstEmpty;
	
	// tentukan firstEmpty berikutnya
	filesystem.firstEmpty = 255;
	for (int i = empty + 1; i < SLOT_NUM; i++){
		if (filesystem.files[i].isEmpty()){
			filesystem.firstEmpty = i;
			break;
		}
	}
	
	// kurangi jumlah slot yang available
	filesystem.available--;
	
	// update header
	filesystem.writeHeader();
	
	return empty;
}

/* mengosongkan kembali suatu slot */
void returnFree(int index){
	// kosongkan slot lalu update
	filesystem.files[index].name = "";
	filesystem.files[index].size = 0;
	filesystem.writeFile(index);

	// naikkan jumlah slot yang available
	filesystem.available++;
	
	// update firstEmpty
	if (filesystem.firstEmpty > index){
		filesystem.firstEmpty = index;
	}
	
	// update header
	filesystem.writeHeader();
}

/************ Implementasi FUSE ***************/

/* get attribute */
int simple_getattr(const char* path, struct stat* stbuf){

	/* jika root path */
	if (string(path) == "/"){
		stbuf->st_nlink = 1;
		stbuf->st_mode = S_IFDIR | 0777; // file dengan permission rwxrwxrwx
		stbuf->st_mtime = filesystem.mount_time;
		return 0;
	}else{
		int index;
		
		// cari file
		try{
			index = searchFile(path);
		}catch(int e){
			return e;
		}
	
		// jika tidak ada, kembalikan error no such file/directory
		if (index < 0) return -ENOENT;
	
		// tulis stbuf, tempat memasukkan atribut file
		stbuf->st_nlink = 1;
		
		// entri ini adalah file dengan permission rwxrwxrwx
		stbuf->st_mode = S_IFREG | 0777;
		
		// ukuran file
		stbuf->st_size = filesystem.files[index].size;
		
		// waktu pembuatan file, asumsinya sama dengan waktu mounting
		stbuf->st_mtime = filesystem.mount_time;
	
		return 0;
	}
}

/* open file */
int simple_open(const char* path, struct fuse_file_info* fi){
	/* hanya mengecek apakah file ada atau tidak */

	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	// error no such file/directory
	if (index < 0) return -ENOENT;
			
	return 0;
}

/* mknod, untuk membuat file */
int simple_mknod(const char *path, mode_t mode, dev_t dev){
	
	// kalo filesystem sudah penuh, error over quota
	if (filesystem.available == 0){
		return -EDQUOT;
	}
	
	// cari file
	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	// kalo sudah ada, return error file already exist
	if (index >= 0){
		return -EEXIST;
	}
	
	// alokasi slot baru
	index = nextFree();
	
	// edit nama slot
	filesystem.files[index].name = getPath(path);
	filesystem.files[index].size = 0;
	
	
	// update data slot
	filesystem.writeFile(index);
	
	return 0;
}

/* membaca direktori */
int simple_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi){
	// selain root directory, error
	if (string(path) != "/"){
		return -ENOENT;
	}
	
	// fungsi filler digunakan untuk setiap entry pada direktori tsb
	// ditulis ke buffer "buf"
	
	// current & parent directory
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	for(int i = 0; i < SLOT_NUM; ++i){
		// untuk setiap slot yang tidak kosong, tulis ke buf
		if (!filesystem.files[i].isEmpty()){
			filler(buf, filesystem.files[i].name.c_str(), NULL, 0);
		}
	}
	
	return 0;
}

/* untuk mengubah panjang suatu file, biasa digunakan saat penulisan pertama */
int simple_truncate(const char* path, off_t size){
	// cari file
	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	if (index < 0){
		return -ENOENT;
	}
	
	// jika ukuran lebih dari 100 byte, error file too big
	if (size > 100){
		return -EFBIG;
	}
	
	// update ukuran slot
	filesystem.files[index].size = size;
	filesystem.writeFile(index);
	
	return 0;
}

/* membaca file */
int simple_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
	// cari file
	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	if (index < 0){
		return -ENOENT;
	}
	
	int fsize = filesystem.files[index].size;
	
	if (offset >= fsize){
		// jika titik awal pembacaan melebihi ukuran file
		// maka tidak ada data yg dibaca
		
		size = 0;
	}else{
		// sesuaikan panjang pembacaan jika melebihi ukuran file
		if (offset + size > fsize){
			size = fsize - offset;
		}
		
		// masukkan data ke buf
		filesystem.files[index].getContent(buf, size, offset);
	} 
	
	// kembalikan jumlah byte yang berhasil dibaca
	return size;
}

/* menulis file */
int simple_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
	// cari file
	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	if (index < 0){
		return -ENOENT;
	}
	
	
	int fsize = filesystem.files[index].size;
	
	// jika penulisan akan menyebabkan ukuran file melebihi 100 byte, error file too big
	if (offset + size > 100){
		return -EFBIG;		
	}
	
	// tulis isi file
	filesystem.files[index].setContent(buf, size, offset);
	
	// jika penulisan akan menyebabkan ukuran bertambah, update ukuran file
	if (offset + size > fsize){
		fsize = offset + size - fsize;
		filesystem.files[index].size = fsize;
	}
	
	// update data slot
	filesystem.writeFile(index);
	
	return size;
}

/* rename/cut file */
int simple_rename(const char* from, const char* to){
	// cari from dan to
	int idx_from, idx_to;
	try{
		idx_from = searchFile(from);
		idx_to = searchFile(to);
	}catch(int e){
		return e;
	}
	
	// jika from tidak ada, error
	if (idx_from < 0){
		return -ENOENT;
	}
	
	// ganti nama from jadi to
	filesystem.files[idx_from].name = getPath(to);
	filesystem.writeFile(idx_from);
	
	// jika to sudah ada, hapus to
	if (idx_to >= 0){
		returnFree(idx_to);		
	}

	return 0;
}

/* hapus file */
int simple_unlink(const char* path){
	// cari file
	int index;
	try{
		index = searchFile(path);
	}catch(int e){
		return e;
	}
	
	if (index < 0){
		return -ENOENT;
	}
	
	// hapus file
	returnFree(index);
		
	return 0;
}
