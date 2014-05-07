/**
 * File: CCFS.hpp
 * Modification of SimpleFS.hpp
 */

#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <ctime>

/** Definisi tipe **/
typedef unsigned short ptr_block;

/** Konstanta **/
/* Konstanta ukuran */
#define BLOCK_SIZE 512
#define N_BLOCK 65536
#define ENTRY_SIZE 32
#define DATA_POOL_OFFSET 257
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x0000
#define END_BLOCK 0xFFFF

using namespace std;

/**
 * Kelas CCFS: kelas yang mendefinisikan keseluruhan filesystem
 */
class CCFS{
public:
/* Method */	
	/* konstruktor & destruktor */
	CCFS();
	~CCFS();
	
	/* buat file *.ccfs */
	void create(const char *filename);
	void initVolumeInformation(const char *filename);
	void initAllocationTable();
	void initDataPool();
	
	/* baca file *.ccfs */
	void load(const char *filename);
	void readVolumeInformation();
	void readAllocationTable();
	
	void writeVolumeInformation();
	void writeAllocationTable(ptr_block position);
	
	/* bagian alokasi block */
	void setNextBlock(ptr_block position, ptr_block next);
	ptr_block allocateBlock();
	void freeBlock(ptr_block position);
	
	/* bagian baca/tulis block */
	int readBlock(ptr_block position, char *buffer, int size, int offset = 0);
	int writeBlock(ptr_block position, const char *buffer, int size, int offset = 0);

/* Attributes */
	fstream handle;			// file .ccfs
	ptr_block nextBlock[N_BLOCK];	//pointer ke blok berikutnya
	
	string filename;		// nama volume
	int capacity;			// kapasitas filesystem dalam blok
	int available;			// jumlah slot yang masih kosong
	int firstEmpty;			// slot pertama yang masih kosong
	time_t mount_time;		// waktu mounting, diisi di konstruktor
};

/**
 * Kelas Entry
 */
class Entry {
public:
/* Method */
	Entry();
	Entry(ptr_block position, unsigned char offset);
	Entry nextEntry();
	Entry getEntry(const char *path);
	Entry getNewEntry(const char *path);
	Entry getNextEmptyEntry();
	
	void makeEmpty();
	int isEmpty();
	
	string getName();
	unsigned char getAttr();
	short getTime();
	short getDate();
	ptr_block getIndex();
	int getSize();
	
	void setName(const char* name);
	void setAttr(const unsigned char attr);
	void setTime(const short time);
	void setDate(const short date);
	void setIndex(const ptr_block index);
	void setSize(const int size);
	
	time_t getDateTime();
	void setCurrentDateTime();
	
	void write();
	
/* Attributes */
	char data[ENTRY_SIZE];
	ptr_block position;	//posisi blok
	unsigned char offset;	//offset dalam satu blok (0..15)
};
