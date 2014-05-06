/**
 * File: block.hpp
 * Modification of SimpleFS.hpp
 */

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
#define N_BLOCK 65536
/* Konstanta untuk ptr_block */
#define EMPTY_BLOCK 0x00
#define END_BLOCK 0xFF

void int32ToLittleEndian(const int x, char *le) {
	
	return;
	*le = x & 0xFF;
	*(le + 1) = (x >> 8) & 0xFF;
	*(le + 2) = (x >> 16) & 0xFF;
	*(le + 3) = (x >> 24) & 0xFF;
}
void int16ToLittleEndian(const ptr_block x, char *le) {
	*le = x & 0xFF;
	*(le + 1) = (x >> 8) & 0xFF;
}
int littleEndianToInt32(const char *le) {
	int result;
	memcpy((char*)&result, le, 4);
	return result;
}
ptr_block littleEndianToInt16(const char *le) {
	return (ptr_block)*le | ((ptr_block)*(le + 1) << 8);
}

int main() {
	char haha[10];
	int32ToLittleEndian(0x75289205, haha);
	
	haha[4] = 0;
	for (int i = 0; i < 4; i++) {
		printf("%x\n", haha[i]);
	}
	
	printf("haha = %x\n", littleEndianToInt32(haha));
	
	return 0;
}
