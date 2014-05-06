#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <ctime>
#include <algorithm>

/* Definisi tipe */
typedef unsigned short ptr_block;

/** Konstanta **/
/* Konstanta ukuran */
#define BLOCK_SIZE 512
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x00
#define END_BLOCK 0xFF

class Entry {
public:
/* Method */
	Entry(ptr_block position, unsigned char offset);
	Entry nextEntry();
	Entry getEntry(const char *path);
	
/* Attributes */
	char data[BLOCK_SIZE];
	ptr_block position;	//posisi blok
	unsigned char offset;	//offset dalam satu blok (0..15)
};
