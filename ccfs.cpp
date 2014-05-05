#include <stdexcept>
#include <algorithm>

#include "ccfs.hpp"

/* konstruktor */
CCFS::CCFS(){
	time(&mount_time);
}

/* destruktor */
CCFS::~CCFS(){
	handle.close();
}

/* buat file *.ccfs baru */
void CCFS::create(const char *filename){
	
	/* buka file dengan mode input-output, binary dan truncate (untuk membuat file baru) */
	handle.open(filename, fstream::in | fstream::out | fstream::binary | fstream::trunc);
	
	/** Bagian Volume Information **/
	initVolumeInformation(filename);
	
	/** Bagian Allocation Table **/
	initAllocationTable();
	
	/** Bagian Data Pool **/
	initDataPool();
	
	handle.close();
}
/* inisialisasi Volume Information */
void CCFS::initVolumeInformation(const char *filename) {
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "CCFS" */
	memcpy(buffer + 0x00, "CCFS", 4);
	
	/* Nama volume */
	memcpy(buffer + 0x04, filename, strlen(filename));
	
	/* Kapasitas filesystem, dalam little endian */
	int _available = available = N_BLOCK;
	reverse((char*)&_available, (char*)(&_available + 1));
	memcpy(buffer + 0x24, (char*)&_available, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	int _firstEmpty = firstEmpty = N_BLOCK;
	reverse((char*)&_firstEmpty, (char*)(&_firstEmpty + 1));
	memcpy(buffer + 0x28, (char*)&_available, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	memset(buffer + 0x2C, 0, 4);
	
	/* String "SFCC" */
	memcpy(buffer + 0x1FC, "SFCC", 4);
	
	handle.write(buffer, BLOCK_SIZE);
}
/* inisialisasi Allocation Table */
void CCFS::initAllocationTable() {
	short buffer = 0xFFFF;
	
	/* Allocation Table untuk root */
	handle.write((char*)&buffer, sizeof(short));
	
	/* Allocation Table untuk lainnya */
	buffer = 0;
	handle.write((char*)&buffer, sizeof(short));
}
/* inisialisasi Data Pool */
void CCFS::initDataPool() {
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	for (int i = 0; i < N_BLOCK; i++) {
		handle.write(buffer, BLOCK_SIZE);
	}
}

/* baca file simple.fs */
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
}
/* membaca Volume Information */
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
	
	/* baca available */
	*((char*)&available + 3) = buffer[0x24];
	*((char*)&available + 2) = buffer[0x25];
	*((char*)&available + 1) = buffer[0x26];
	*((char*)&available) = buffer[0x27];
	
	/* baca firstEmpty */
	*((char*)&firstEmpty + 3) = buffer[0x28];
	*((char*)&firstEmpty + 2) = buffer[0x29];
	*((char*)&firstEmpty + 1) = buffer[0x2A];
	*((char*)&firstEmpty) = buffer[0x2B];
}

