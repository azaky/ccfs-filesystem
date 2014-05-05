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

using namespace std;

/* Konstanta */
#define BLOCK_SIZE 512
#define N_BLOCK 65536

class CCFS{
	private:
	fstream handle;			// file .ccfs
	
	public:
	//Slot files[SLOT_NUM]; 	// data seluruh slot file yang ada
	
	int available;			// jumlah slot yang masih kosong
	int firstEmpty;			// slot pertama yang masih kosong
	time_t mount_time;		// waktu mounting, diisi di konstruktor
	
	/* konstruktor & destruktor */
	CCFS();
	~CCFS();
	
	/* buat file simple.fs baru */
	void create(const char *filename);
	void initVolumeInformation(const char *filename);
	void initAllocationTable();
	void initDataPool();
	
	/* baca file simple.fs */
	void load(const char *filename);
	void readVolumeInformation();
};
