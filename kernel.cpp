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
	static int i,j,t,h = 0;
	//different pattern lengths
	static uint32_t crc;
	static int head, buff_idx;
	static char stream_mem[BUFFER_WIDTH][NUM_BYTES] = {0}; //buffer to fit 364+64 bytes, 364 for the longest pattern

//	if (!in.empty()){ //useful?

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
		}

		static uint16_t el_count, curr_max_idx, curr_idx, count, curr_max_len, match = 0;


		*out = -1; //initalize to -1, if -1/INTMAX should be counted as a none match.
		for(head = NUM_BYTES-1; head >= 0; head--){
			#pragma HLS UNROLL factor = 2
			el_count = 0; //the hash array is linear, this variable is used to keep track of how many elements we've encountered per length
			match = 0; //initalize match to 0, meaning match, change if there isn't a match
			curr_max_len = 0; //priority variables
			curr_max_idx = 0;
			for(uint16_t idx_len = 0; idx_len < sizeof(lengths)/sizeof(lengths[0]); idx_len++){
				uint16_t p_len = lengths[idx_len];
				uint32_t crc = 0xFFFFFFFF;
				uint8_t octet;
				for (int i = 0; i>=-p_len+1; i--) {
					octet = stream_mem[BUFFER_WIDTH-1][head+i];  /* Cast to unsigned octet. */
					crc = (crc >> 8) ^ crc_table[(crc & 0xff) ^ octet];
				}
				crc = ~crc;
				for(uint16_t k = 0; k<elements[idx_len]; k++){ //elements[idx_len] = number of patterns for a given string length
					if(crc == rules[el_count+k]){ //do hashes match? Then save the idx and the length for which this was found, this information is then used for the full strcmp
						if(curr_max_idx < el_count+k){
							curr_max_len = lengths[idx_len];
							curr_max_idx = el_count+k;
							curr_idx = k;
						}
					}
				}
				el_count += elements[idx_len]; //linear hash array, keep track of how many total elements we've gone through
			}
			//MACRO expansion of the auto generated switch, we needed to put the strings of every length into its own variable and we can't do variable substitution with a for loop, therefore
			// this has to be generated outside the c code. The switch is used for the full byte wise comparison between the input string and the pattern.
			LONG_SWITCH

			if(match == 0 && count < 41){ //we can only fit 42 matches when using a 12 bit result
				out->range(count*12,count*12+11) = curr_max_idx;
				count++; //one more position used in the output buffer
			}
		}
		count = 0; // reset when head resets
//	}

}
