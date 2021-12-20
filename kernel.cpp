#include "./kernel.h"
#include "./crc_table.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/lengths.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/ruleset.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/elements.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/str_ruleset.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/switch.h"




void krnl_hash(hls::stream<pkt > &in,
		 ap_uint<NUM_BYTES*8> *out
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
		}

		static uint16_t el_count, curr_max_idx, curr_idx, count = 0;
		static uint8_t curr_max_len, match = 0;


		*out = -1; //initalize to -1, if -1/INTMAX should be counted as a none match.
		for(head = NUM_BYTES-1; head >= 0; head--){
			#pragma HLS UNROLL factor = 1
			el_count = 0;
				// Calculate the hash value of pattern and first
				// window of text. When head increments we move the window forward n Bytes
			match = 0; //initalize match to 0, meaning match, change if there isn't a match
			curr_max_len = 0;
			curr_max_idx = 0;
			for(uint16_t idx_len = 0; idx_len < sizeof(lengths)/sizeof(lengths[0]); idx_len++){
				#pragma HLS UNROLL
				uint16_t p_len = lengths[idx_len];
				uint32_t crc = 0xFFFFFFFF;
				uint8_t octet;
				for (int i = 0; i>=-p_len+1; i--) {
					octet = stream_mem[BUFFER_WIDTH-1][head+i];  /* Cast to unsigned octet. */
					crc = (crc >> 8) ^ crc_table[(crc & 0xff) ^ octet];
				}
				crc = ~crc;
				for(uint16_t k = 0; k<elements[idx_len]; k++){

					if(crc == rules[el_count+k]){ //do hashes match? Then save the idx and the length for which this was found
						if(curr_max_idx < el_count+k){
							curr_max_len = lengths[idx_len];
							curr_max_idx = el_count+k;
							curr_idx = k;
						}
					}
				}
				el_count += elements[idx_len];
			}
			//MACRO expansion of the auto generated switch, we needed to put the strings of every length into its own variable and we can't do variable substitution with a for loop, therefore
			// this has to be generated outside the c code.
			LONG_SWITCH

			if(match == 0 && count < 55){ //we can only fit 55 matches when using a 9 bit result
				out->range(count*9,count*9+8) = curr_max_idx;
				count++;
			}
		}
		count = 0;
	}

}
