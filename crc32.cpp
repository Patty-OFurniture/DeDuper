#include "stdafx.h"

/* Version to skip bytes
UINT32 MemGetCRC( UINT8 *cImage, tgd_size_t size, tgd_size_t skip_bytes, UINT32 crcStart ) {
	register UINT32 crc;
	tgd_size_t		x;

	crc = crcStart^0xFFFFFFFF;	// precondition
	size += skip_bytes;

	for(x=skip_bytes; x<size ; x++)	//need to test for valid memory pointer?
		crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[ (crc^cImage[x]) & 0xFF ];
	return( crc^0xFFFFFFFF );	// postcondition
}
*/

unsigned crcTable[256];

unsigned MemGetCRC(unsigned char *cImage, int size, unsigned crcStart) {
	register unsigned crc;		// could use crcStart ^= 0xFFFFFFFF instead
	int x;

	crc = crcStart ^ 0xFFFFFFFF;	// precondition

	for (x = 0; x < size; x++)		//need to test for valid memory pointer?
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crcTable[(crc^cImage[x]) & 0xFF];
	return(crc ^ 0xFFFFFFFF);	// postcondition
}

void crcgen(void) {
	unsigned crc, poly;
	int i, j;

	poly = 0xEDB88320L;
	for (i = 0; i < 256; i++) {
		crc = i;
		for (j = 8; j > 0; j--) {
			if (crc & 1) {
				crc = (crc >> 1) ^ poly;
			}
			else {
				crc >>= 1;
			}
		}
		crcTable[i] = crc;
	}
}

