#include <stdexcept>

#include "ccfs.hpp"

/* Global filesystem */
extern CCFS filesystem;


/**                  *
 * BAGIAN KELAS CCFS *
 *                  **/

/** konstruktor */
CCFS::CCFS(){
	time(&mount_time);
}
/** destruktor */
CCFS::~CCFS(){
	handle.close();
}

/** buat file *.ccfs baru */
void CCFS::create(const char *filename){
	
	/* buka file dengan mode input-output, binary dan truncate (untuk membuat file baru) */
	handle.open(filename, fstream::in | fstream::out | fstream::binary | fstream::trunc);
	
	/* Bagian Volume Information */
	initVolumeInformation(filename);
	
	/* Bagian Allocation Table */
	initAllocationTable();
	
	/* Bagian Data Pool */
	initDataPool();
	
	handle.close();
}
/** inisialisasi Volume Information */
void CCFS::initVolumeInformation(const char *filename) {
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "CCFS" */
	memcpy(buffer + 0x00, "CCFS", 4);
	
	/* Nama volume */
	this->filename = string(filename);
	memcpy(buffer + 0x04, filename, strlen(filename));
	
	/* Kapasitas filesystem, dalam little endian */
	capacity = N_BLOCK;
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	available = N_BLOCK - 1;
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	firstEmpty = 1;
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);
	
	/* String "SFCC" */
	memcpy(buffer + 0x1FC, "SFCC", 4);
	
	handle.write(buffer, BLOCK_SIZE);
}
/** inisialisasi Allocation Table */
void CCFS::initAllocationTable() {
	short buffer = 0xFFFF;
	
	/* Allocation Table untuk root */
	handle.write((char*)&buffer, sizeof(short));
	
	/* Allocation Table untuk lainnya */
	buffer = 0;
	for (int i = 1; i < N_BLOCK; i++) {
		handle.write((char*)&buffer, sizeof(short));
	}
}
/** inisialisasi Data Pool */
void CCFS::initDataPool() {
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	for (int i = 0; i < N_BLOCK; i++) {
		handle.write(buffer, BLOCK_SIZE);
	}
}

/** baca file simple.fs */
void CCFS::load(const char *filename){
	/* buka file dengan mode input-output, dan binary */
	handle.open(filename, fstream::in | fstream::out | fstream::binary);
	
	/* cek apakah file ada */
	if (!handle.is_open()){
		handle.close();
		throw runtime_error("File not found");
	}
	
	/* periksa Volume Information */
	readVolumeInformation();
	
	/* baca Allocation Table */
	readAllocationTable();
}
/** membaca Volume Information */
void CCFS::readVolumeInformation() {
	char buffer[BLOCK_SIZE];
	handle.seekg(0);
	
	/* Baca keseluruhan Volume Information */
	handle.read(buffer, BLOCK_SIZE);
	
	/* cek magic string */
	if (string(buffer, 4) != "CCFS") {
		handle.close();
		throw runtime_error("File is not a valid CCFS file");
	}
	
	/* baca capacity */
	memcpy((char*)&capacity, buffer + 0x24, 4);
	
	/* baca available */
	memcpy((char*)&available, buffer + 0x28, 4);
	
	/* baca firstEmpty */
	memcpy((char*)&firstEmpty, buffer + 0x2C, 4);
}
/** membaca Allocation Table */
void CCFS::readAllocationTable() {
	char buffer[3];
	
	/* pindah posisi ke awal Allocation Table */
	handle.seekg(0x200);
	
	/* baca nilai nextBlock */
	for (int i = 0; i < N_BLOCK; i++) {
		handle.read(buffer, 2);
		memcpy((char*)&nextBlock[i], buffer, 2);
	}
}
/** menuliskan Volume Information */
void CCFS::writeVolumeInformation() {
	handle.seekp(0x00);
	
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "CCFS" */
	memcpy(buffer + 0x00, "CCFS", 4);
	
	/* Nama volume */
	memcpy(buffer + 0x04, filename.c_str(), filename.length());
	
	/* Kapasitas filesystem, dalam little endian */
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);
	
	/* String "SFCC" */
	memcpy(buffer + 0x1FC, "SFCC", 4);
	
	handle.write(buffer, BLOCK_SIZE);
}
/** menuliskan Allocation Table pada posisi tertentu */
void CCFS::writeAllocationTable(ptr_block position) {
	handle.seekp(BLOCK_SIZE + sizeof(ptr_block) * position);
	handle.write((char*)&nextBlock[position], sizeof(ptr_block));
}
/** mengatur Allocation Table */
void CCFS::setNextBlock(ptr_block position, ptr_block next) {
	nextBlock[position] = next;
	writeAllocationTable(position);
}
/** mendapatkan first Empty yang berikutnya */
ptr_block CCFS::allocateBlock() {
	ptr_block result = firstEmpty;
	
	setNextBlock(result, END_BLOCK);
	
	while (nextBlock[firstEmpty] != 0x0000) {
		firstEmpty++;
	}
	available--;
	writeVolumeInformation();
	return result;
}
/** membebaskan blok */
void CCFS::freeBlock(ptr_block position) {
	if (position == EMPTY_BLOCK) {
		return;
	}
	while (position != END_BLOCK) {
		ptr_block temp = nextBlock[position];
		setNextBlock(position, EMPTY_BLOCK);
		position = temp;
		available--;
	}
	writeVolumeInformation();
}
/** membaca isi block sebesar size kemudian menaruh hasilnya di buf */
int CCFS::readBlock(ptr_block position, char *buffer, int size, int offset) {
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	handle.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	/* cuma bisa baca sampai sebesar block size */
	if (size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE;
	}
	handle.read(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (size > BLOCK_SIZE) {
		return size_now + readBlock(nextBlock[position], buffer + BLOCK_SIZE, size - BLOCK_SIZE);
	}
}

/** menuliskan isi buffer ke filesystem */
int CCFS::writeBlock(ptr_block position, const char *buffer, int size, int offset,int *blockTime) {
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	handle.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	if (size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE;
	}
	handle.write(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (size > BLOCK_SIZE) {
		/* kalau nextBlock tidak ada, alokasikan */
		if (nextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return size_now + writeBlock(nextBlock[position], buffer + BLOCK_SIZE, size - BLOCK_SIZE);
	}
}

/**                   *
 * BAGIAN KELAS ENTRY *
 *                   **/

/** Konstruktor: buat Entry kosong */
Entry::Entry() {
	position = 0;
	offset = 0;
	memset(data, 0, ENTRY_SIZE);
}
/** Konstruktor parameter */
Entry::Entry(ptr_block position, unsigned char offset) {
	this->position = position;
	this->offset = offset;
	
	/* baca dari data pool */
	filesystem.handle.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
	filesystem.handle.read(data, ENTRY_SIZE);
}

/** Mendapatkan Entry berikutnya */
Entry Entry::nextEntry() {
	if (offset < 15) {
		return Entry(position, offset + 1);
	}
	else {
		return Entry(filesystem.nextBlock[position], 0);
	}
}

/** Mendapatkan Entry dari path */
Entry Entry::getEntry(const char *path) {
	printf("Entry::getEntry(%s)\n", path);
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	printf("topDirectory = [%s]\n", topDirectory.c_str());
	
	/* mencari entri dengan nama topDirectory */
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	/* kalau tidak ketemu, return Entry kosong */
	if (isEmpty()) {
		return Entry();
	}
	/* kalau ketemu, */
	else {
		if (endstr == strlen(path)) {
			return *this;
		}
		else {
			/* cek apakah direktori atau bukan */
			if (getAttr() & 0x8) {
				ptr_block index;
				memcpy((char*)&index, data + 0x1A, 2);
				Entry next(index, 0);
				return next.getEntry(path + endstr);
			}
			else {
				return Entry();
			}
		}
	}
}

/** Mendapatkan Entry dari path */
Entry Entry::getNewEntry(const char *path) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	printf("topDirectory = [%s]\n", topDirectory.c_str());
	
	/* mencari entri dengan nama topDirectory */
	Entry entry(position, offset);
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	/* kalau tidak ketemu, buat entry baru */
	if (isEmpty()) {
		while (!entry.isEmpty()) {
			if (entry.nextEntry().position == END_BLOCK) {
				entry = Entry(filesystem.allocateBlock(), 0);
			}
			else {
				entry = entry.nextEntry();
			}
		}
		/* beri atribut pada entry */
		entry.setName(topDirectory.c_str());
		entry.setAttr(0xF);
		entry.setIndex(filesystem.allocateBlock());
		entry.setSize(BLOCK_SIZE);
		entry.setTime(0);
		entry.setDate(0);
		entry.write();
		
		*this = entry;
	}
	
	if (endstr == strlen(path)) {
		return *this;
	}
	else {
		/* cek apakah direktori atau bukan */
		if (getAttr() & 0x8) {
			ptr_block index;
			memcpy((char*)&index, data + 0x1A, 2);
			Entry next(index, 0);
			return next.getNewEntry(path + endstr);
		}
		else {
			return Entry();
		}
	}
}

/** Mengembalikan entry kosong selanjutnya. Jika blok penuh, akan dibuatkan entri baru */
Entry Entry::getNextEmptyEntry() {
	Entry entry(*this);
	
	while (!entry.isEmpty()) {
		entry = entry.nextEntry();
	}
	if (entry.position == END_BLOCK) {
		/* berarti blok saat ini sudah penuh, buat blok baru */
		ptr_block newPosition = filesystem.allocateBlock();
		ptr_block lastPos = position;
		while (filesystem.nextBlock[lastPos] != END_BLOCK) {
			lastPos = filesystem.nextBlock[lastPos];
		}
		filesystem.setNextBlock(lastPos, newPosition);
		entry.position = newPosition;
		entry.offset = 0;
	}
	
	return entry;
}

/** mengosongkan entry */
void Entry::makeEmpty() {
	/* hapus index? */
	/*
	if (getIndex() != EMPTY_BLOCK && getIndex() != END_BLOCK) {
		filesystem.freeBlock(getIndex());
	}
	*/
	/* menghapus byte pertama data */
	*(data) = 0;
	write();
}

/** Memeriksa apakah Entry kosong atau tidak */
int Entry::isEmpty() {
	return *(data) == 0;
}

/** Getter-Setter atribut-atribut Entry */
string Entry::getName() {
	return string(data);
}

unsigned char Entry::getAttr() {
	return *(data + 0x15);
}

short Entry::getTime() {
	short result;
	memcpy((char*)&result, data + 0x16, 2);
	return result;
}

short Entry::getDate() {
	short result;
	memcpy((char*)&result, data + 0x18, 2);
	return result;
}

ptr_block Entry::getIndex() {
	ptr_block result;
	memcpy((char*)&result, data + 0x1A, 2);
	return result;
}

int Entry::getSize() {
	int result;
	memcpy((char*)&result, data + 0x1C, 4);
	return result;
}

void Entry::setName(const char* name) {
	strcpy(data, name);
}

void Entry::setAttr(const unsigned char attr) {
	data[0x15] = attr;
}

void Entry::setTime(const short time) {
	memcpy(data + 0x16, (char*)&time, 2);
}

void Entry::setDate(const short date) {
	memcpy(data + 0x18, (char*)&date, 2);
}

void Entry::setIndex(const ptr_block index) {
	memcpy(data + 0x1A, (char*)&index, 2);
}

void Entry::setSize(const int size) {
	memcpy(data + 0x1C, (char*)&size, 4);
}

/** Menuliskan entry ke filesystem */
void Entry::write() {
	if (position != END_BLOCK) {
		filesystem.handle.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
		filesystem.handle.write(data, ENTRY_SIZE);
	}
}
