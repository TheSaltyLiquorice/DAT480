
#include <stdio.h>
#include <string.h>
#include "crc32.h"

using namespace std;
int
main()
{ 
	uint32_t table[256];
	generate_table(table);
	const char* s = "The quick brown fox jumps over the lazy dog";
	uint32_t crc_res = 0;
	crc_res = crc_cal(table, s, strlen(s));
	printf("%" PRIX32 "\n", crc_res);
	return 0; 
}