#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc.h"
void generate_input_vec(uint32_t lut [2678], uint32_t lengths[2678]);
using namespace std;
int
main()
{
	uint32_t table[256];
	uint32_t lut[2678];
	uint32_t lengths[2678];
	char buff[400*2678];
	generate_table(table);
	FILE *fp = fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/pattern_match_snort3_content.txt","r");
	FILE *fp_hash_out = fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/pattern_match_snort3_content_hashed.txt","w+");
	FILE *fp_combined_out = fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/pattern_match_snort3_content_combined.txt","w+");
	FILE *fp_length_out = fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/pattern_match_snort3_content_length.txt","w+");
	if(fp == NULL){
		exit(0);
	}

	int i = 0;
	while(fgets(buff, 2678, fp)){
		int length = strlen(buff)-1;
		lengths[i] = length;
		printf("%s",buff);
		printf("%d\n",length);
		buff[strcspn(buff, "\n")] = 0;
		lut[i] = crc_cal(table, buff, length);
		fprintf(fp_combined_out, "%s %x\n", buff, lut[i]);
		fprintf(fp_hash_out,"%x\n",lut[i]);
		fprintf(fp_length_out,"%u\n",lengths[i]);
		i++;
	}
	fclose(fp);
	fclose(fp_hash_out);
	fclose(fp_length_out);
	fclose(fp_combined_out);


	const char* s = "|10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F|";
	uint32_t crc_res = 0;
	crc_res = crc_cal(table, s, strlen(s));
	// printf("----------------------------------\nlength = %d\n%x\n", strlen(s), crc_res);
	return 0;
}


void generate_table(uint32_t(&table)[256])
{
	static int have_table = 0;
	uint32_t rem;

	int i, j;


	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				}
				else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}
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
