#include "./kernel.h"
#include "./crc_table.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/lengths.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/ruleset.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/elements.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/str_ruleset.h"




void krnl_hash(hls::stream<pkt > &in,
		 ap_uint<NUM_BYTES*8> *out
//	     hls::stream<pkt> &out
		 )
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE m_axi port = out offset = slave bundle = gmem
#pragma HLS INTERFACE s_axilite port = out bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control



	pkt word;
//	pkt result;
	int sz = SIZE;
	int bpb = BYTES_PER_BEAT;
	//in.read(word);
	static int i,j,t,h = 0;
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
//		printf("\n");
//		for(int i = BUFFER_WIDTH-1; i>=0; i--){
//			for(int j = NUM_BYTES-1; j>=0; j--){
//				printf("%c",stream_mem[i][j]);
//			}
//////
//		}
//		printf("\n");
		#endif


		static unsigned int count = 0;
		static uint16_t el_count = 0;
		static uint16_t curr_max_idx = 0;
		static uint16_t curr_idx = 0;
		static uint8_t curr_max_len = 0;
		static uint8_t match = 0;


//		static uint16_t curr_max;
		*out = -1; //initalize to -1, if -1/INTMAX should be counted as a none match.
		for(head = NUM_BYTES-1; head >= 0; head--){
			#pragma HLS UNROLL factor = 4
			el_count = 0;
				// Calculate the hash value of pattern and first
				// window of text. When head increments we move the window forward 1 Byte
			match = 0; //initalize match to 0, meaning match, change if there isn't a match
			curr_max_len = 0;
			curr_max_idx = 0;
			for(uint16_t idx_len = 0; idx_len < sizeof(lengths)/sizeof(lengths[0]); idx_len++){
				#pragma HLS UNROLL

				const uint16_t p_len = lengths[idx_len];
				uint32_t crc = 0xFFFFFFFF;
				uint8_t octet;
				for (int i = 0; i>=-p_len+1; i--) {
					octet = stream_mem[BUFFER_WIDTH-1][head+i];  /* Cast to unsigned octet. */
					crc = (crc >> 8) ^ crc_table[(crc & 0xff) ^ octet];
				}
				crc = ~crc;
				for(uint16_t k = 0; k<elements[idx_len]; k++){

					if(crc == rules[el_count+k]){ //do hashes match? Then save the idx and the length for which this was found
//						printf("head = %d, curr_max_idx = %d, el_count+k = %d\n",head,curr_max_idx,el_count+k);
						if(curr_max_idx < el_count+k){
							curr_max_len = lengths[idx_len];
							curr_max_idx = el_count+k;
							curr_idx = k;
//							printf("Index updates, head = %d, len = %d, k = %d\n",head,idx_len,k);
						}
					}
				}
				el_count += elements[idx_len];
			}
			switch (curr_max_len) {
				case 4:
					for(int i = 0; i<4; i++){
						if(stream_mem[BUFFER_WIDTH-1][head-i] != str_rules_4[curr_idx][i]){
							match = 1;
						}
					}
					break;
				case 5:
					for(int i = 0; i<5; i++){
						if(stream_mem[BUFFER_WIDTH-1][head-i] != str_rules_5[curr_idx][i]){
							match = 1;
						}
					}
					break;
				case 6:
					for(int i = 0; i<6; i++){
						if(stream_mem[BUFFER_WIDTH-1][head-i] != str_rules_6[curr_idx][i]){
							match = 1;
						}
					}
					break;
				case 7:
					for(int i = 0; i<7; i++){
						if(stream_mem[BUFFER_WIDTH-1][head-i] != str_rules_7[curr_idx][i]){
							match = 1;
						}
					}
					break;
				case 8:
					for(int i = 0; i<8; i++){
						if(stream_mem[BUFFER_WIDTH-1][head-i] != str_rules_8[curr_idx][i]){
							match = 1;
						}
					}
					break;
				default:
					continue;
			}
			if(match == 0 && count < 55){
//				printf("Kernel says match at %d\n",curr_max_idx);
				out->range(count*9,count*9+8) = curr_max_idx;
				count++;
			#ifndef __synthesis__
				printf("Found full match\n");
			#endif

			}
		}
		count = 0;
	}

}

