#include "./kernel.h"
#include "./crc_table.h"
#include "./rule6.h"
void krnl_hash(hls::stream<pkt > &in,
	     hls::stream<pkt> &out,
		 unsigned int &hash_out
)
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE s_axilite port = hash_out bundle = control //This isnt used it seems if we try and synthesize the code
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1

	pkt word;
	in.read(word);
	static int i,j,t,h = 0;
	//different pattern lengths
	static int lengths[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
			22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,
			46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,64,65,67,73,75,76,78,
			79,80,81,84,85,87,88,89,90,91,92,95,97,101,102,103,107,113,114,115,130,
			132,133,136,145,147,150,162,165,166,171,172,175,179,189,240,262,364};

	static uint32_t crc;
	static int head;
	static int p = 0; // hash value for pattern
	static int q = 101;
	static char stream_mem[NUM_BYTES];
	int M = 6;                                                     // I added this for "M", what was it supposed to be? length of the pattern or?
	for(i = 0; i<NUM_BYTES; i++){
		#pragma HLS UNROLL
		stream_mem[i] = word.data.range(i*8,i*8+7);
		 //everytime we get a pkt populate the stream_mem
		#ifndef __SYNTHESIS__
		std::cout << "tmp = " << stream_mem[i] << std::endl;
		#endif
		}


		// Calculate the hash value of pattern and first
		// window of text. When head increments we need to move the window forward
		for (int i = 0; i < NUM_BYTES - M; i++){
			crc = crc_cal(crc_table, &stream_mem[i], M);
			for(int k =0; k < 65; ++k){
				#pragma HLS UNROLL
				if(crc == rule_6[k]){
					hash_out =crc;
					#ifndef __SYNTHESIS__
					printf("%x\n",rule_6[k]);
					printf("Match at index = %d\n",i);
					#endif
				}
			}
		}

	out.write(word);


	//Just some testing
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	for(i = 0; i <106; i++){
//		#pragma HLS UNROLL
//
//		//first create the different crc hashing engines to be used in parallel where "i" determines when the crc resets because i is now the length of the pattern
//		//This is also where we send in the packets to be examined
//		uint32_t testhash = crc_cal(crc_table, &stream_mem[i], i);
//
//
//		for(j = 0; j < precomputed_hashtable[i]; j++){
//			#pragma HLS UNROLL //I guess we can unroll this as well. Since we know how many hashes each length has
//
//			if(testhash == precomputed_hashtable[j]){
//				printf("Match found, some poopyhead is trying to hack you rip"); 		//If hash matches then we send that pattern back to the jupyter notebook
//			}
//		}
//
//	}

	//if they match the hash we send that back to the PC and to the jupyter notebook

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
