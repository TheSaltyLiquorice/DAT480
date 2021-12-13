#include "./kernel.h"
#include "./crc_table.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/lengths.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/total_ruleset.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/rules.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/OneDimensionTestArray.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/elements.h"



void krnl_hash(hls::stream<pkt > &in,
	     hls::stream<pkt> &out
		 )
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
//#pragma HLS INTERFACE s_axilite port = hash_out bundle = control
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1

	pkt word;
	pkt result;
	int sz = SIZE;
	int bpb = BYTES_PER_BEAT;
	//in.read(word);
	static int i,j,t,h,k = 0;
	//different pattern lengths
	static uint32_t crc;
	static int head, buff_idx;
	static char stream_mem[BUFFER_WIDTH][NUM_BYTES] = {0}; //buffer to fit 364+64 bytes

	if (!in.empty()){

		in.read(word);
		//FIFO, moves data in chunks of 64 BYTES
		for(int h = BUFFER_WIDTH-2; h >= 0; h--){
			memcpy(stream_mem[h+1],stream_mem[h], NUM_BYTES);
		}
		//Assign the word from the stream into memory
		int tmp = 0;
		for(int j = NUM_BYTES-1; j>=0; j--){
			#pragma HLS UNROLL
			stream_mem[0][tmp++] = word.data.range(j*8,j*8+7);
			 //everytime we get a pkt populate the stream_mem
			#ifndef __SYNTHESIS__
//			std::cout << "Kernel receives = " << stream_mem[0][j] << std::endl;
			#endif
		}
		#ifndef __SYNTHESIS__
		printf("\n");
		for(int i = BUFFER_WIDTH-1; i>=0; i--){
			for(int j = NUM_BYTES-1; j>=0; j--){
				printf("%c",stream_mem[i][j]);
			}
//			printf("\n");
		}
		#endif


		static unsigned int count = 0;
		static uint16_t el_count = 0;
//		static uint16_t curr_max;
		result.data = -1; //initalize to -1, if -1/INTMAX should be counted as a none match.
		for(head = NUM_BYTES-1; head >= 0; head--){
			el_count = 0;
				// Calculate the hash value of pattern and first
				// window of text. When head increments we move the window forward 1 Byte
			static uint16_t idx_len;
			for(idx_len = 0; idx_len < sizeof(lengths)/sizeof(lengths[0]); idx_len++){
				uint16_t p_len = lengths[idx_len];
				crc = crc_cal(crc_table, &stream_mem[BUFFER_WIDTH-1][head], p_len); //Start with the byte at max_pos then count backwards
//				printf("crc = %x\n", crc);
				for(k = 0; k<elements[idx_len]; k++){
				#pragma HLS UNROLL
					if(crc == OneDimensionHashes[el_count+k]){
						result.data.range(count*12,count*12+11) = el_count+k;
						#ifndef __SYNTHESIS__
//						printf("kernel says idx: %d\n", el_count+k);
//						printf("%x\n",rules[idx_len][k]);
						#endif
						if(count < 41){ //this is trash, must find a better solution! (insert priority encoder here. p_len can be used to prioritize
							count++;
//							std::cout << "kernel says " << el_count+k << std::endl;

						}
						else{
							#ifndef __SYNTHESIS__
							std::cout << "Missed matches" << std::endl;
							#endif
						}
					}
				}
				el_count += elements[idx_len];
			}
		}

		count = 0;
		out.write(result);
//		}
	}

}



//uint32_t crc_cal(uint32_t* table, char* buf, size_t len) {
//	const char* p, * q;
//	uint32_t crc = 0;
//	uint8_t octet;
//	crc = ~crc;
//	q = buf + len;
//	for (p = buf; p < q; p++) {
//		octet = *p;  /* Cast to unsigned octet. */
//		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
//		//printf("%" PRIX32 "\n", crc);
//	}
//	return ~crc;
//}


uint32_t crc_cal(uint32_t* table, char* buf, int len) {
	uint32_t crc = 0;
	uint8_t octet;
	crc = ~crc;
//	q = buf - len;
	for (int i = 0; i>=-len+1; i--) {
		octet = buf[i];  /* Cast to unsigned octet. */
//		printf("%c",buf[i]);
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}



