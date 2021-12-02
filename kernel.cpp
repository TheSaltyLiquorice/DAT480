#include "./kernel.h"
#include "./crc_table.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/total_ruleset.h"

void krnl_hash(hls::stream<pkt > &in,
	     hls::stream<pkt> &out,
		 unsigned int &hash_out
)
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE s_axilite port = hash_out bundle = control
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1

	pkt word;
	in.read(word);
	static int i,j,t,h,k = 0;
	//different pattern lengths
	static int lengths[106];
	#define M 49

	static uint32_t crc;

	static int head;
	static char stream_mem[NUM_BYTES];
	for(i = 0; i<NUM_BYTES; i++){
		#pragma HLS UNROLL
		stream_mem[i] = word.data.range(i*8,i*8+7);
		 //everytime we get a pkt populate the stream_mem
		#ifndef __SYNTHESIS__
//		std::cout << "tmp = " << stream_mem[i] << std::endl;
		#endif
		}

	for(head = 0; head < (NUM_BYTES-M); head++){
			// Calculate the hash value of pattern and first
			// window of text. When head increments we need to move the window forward
			for (int i = head; i < head+M; i++){
				crc = crc_cal(crc_table, &stream_mem[i], M);
				#ifndef __SYNTHESIS__
//				printf("hash out = %x\n", crc);
				#endif
				for(k = 0; k<26; k++){
				#pragma HLS UNROLL
					if(crc == rule_49[k]){
						hash_out = crc;
						#ifndef __SYNTHESIS__
						printf("%x\n",rule_49[k]);
						printf("Match at index = %d\n",head+M);
						#endif
					}
				}
			}
	}
	out.write(word);
}

uint32_t crc_cal(uint32_t* table, char* buf, size_t len) {
	const char* p, * q;
	uint32_t crc = 0;
	uint8_t octet;
	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
		//printf("%" PRIX32 "\n", crc);
	}
	return ~crc;
}
