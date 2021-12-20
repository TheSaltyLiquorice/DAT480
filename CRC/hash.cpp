#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include "crc.h"
#include "crc_table.txt"
using namespace std;
int main(int argc, char *argv[])
{
	// check if there is more than one argument and use the second one
  //  (the first argument is the executable)
	// const char* s = "|10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F|";
	char *testvar = argv[1];
	uint32_t crc_res = 0;
	crc_res = crc_cal(crc_table, testvar , strlen(testvar));
	printf("%" PRIX32 "\n",crc_res);
	return 0;
}


uint32_t crc_cal(uint32_t* table, const char* buf, size_t len) {
	const char* p, * q;
	uint32_t crc = 0;
	uint8_t octet;
	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
		// printf("%" PRIX32 "\n", crc);
	}
	return ~crc;
}
