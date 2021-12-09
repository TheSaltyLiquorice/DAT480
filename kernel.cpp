#include "./kernel.h"
#include "./crc_table.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/lengths.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/total_ruleset.h"
#include "/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/rules.h"
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
	static char stream_mem[BUFFER_WIDTH][NUM_BYTES] = {0}; //buffer for an entire packet

	if (!in.empty()){
		// Auto-pipeline is going to apply pipeline to this loop
//		for (unsigned int i = 0; i < (sz / bpb); i++) { //one pkt split into 22 parts
//		for (unsigned int i = 0; i < 2; i++) { //one pkt split into 22 parts
		in.read(word);
//
		for(int h = BUFFER_WIDTH-2; h >= 0; h--){
			memcpy(stream_mem[h+1],stream_mem[h], NUM_BYTES);
		}
		for(int j = 0; j<NUM_BYTES; j++){
			#pragma HLS UNROLL
			stream_mem[0][j] = word.data.range(j*8,j*8+7);
			 //everytime we get a pkt populate the stream_mem
			#ifndef __SYNTHESIS__
			//std::cout << "tmp = " << stream_mem[0][j] << std::endl;
			#endif
		}



		static unsigned int count = 0;
		result.data = -1; //initalize to -1
		for(head = 0; head < (NUM_BYTES); head++){
				// Calculate the hash value of pattern and first
				// window of text. When head increments we need to move the window forward
			static uint16_t idx_len;
			for(idx_len = 0; idx_len < sizeof(lengths)/sizeof(lengths[0]); idx_len++){
				uint16_t p_len = lengths[idx_len];
				#pragma HLS INLINE
				crc = crc_cal(crc_table, &stream_mem[0][head], p_len);
				for(k = 0; k<elements[idx_len]; k++){
				#pragma HLS UNROLL
					if(crc == rules[idx_len][k]){
						result.data.range(count*16,count*16+15) = idx_len*sizeof(lengths)/sizeof(lengths[0])+k;
						#ifndef __SYNTHESIS__
						printf("kernel says idx: %d\n", idx_len*sizeof(lengths)/sizeof(lengths[0])+k);
						printf("%x\n",rules[idx_len][k]);
						#endif
						if(count < 31){ //this is trash, must find a better solution!
							count++;
						}
						else{
							#ifndef __SYNTHESIS__
							std::cout << "Missed matches" << std::endl;
							#endif

						}
					}
				}
			}
		}

		count = 0;
		out.write(result);
//		}
	}

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
