#include "./kernel.h"
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
	static int i,j,t,h = 0;
	static int M = 20;
	static int head;
	static int p = 0; // hash value for pattern
	static int q = 101;
	static ap_uint<8> stream_mem[NUM_BYTES];
	#pragma HLS UNROLL
		for(i = 0; i<NUM_BYTES; i++){
			stream_mem[i] = word.data.range(i*8,i*8+7);
		 //everytime we get a pkt populate the stream_mem
		#ifndef __SYNTHESIS__
		std::cout << "tmp = " << stream_mem[i].to_char() << std::endl;
		#endif
		}

	for(head = 0; head < (NUM_BYTES-M); head++){
				// Calculate the hash value of pattern and first
				// window of text. When head increments we need to move the window forward
				for (int i = head; i < head+M; i++){
					t = (d*t + stream_mem[i])%q; //rolling hash
				}
				#ifndef __SYNTHESIS__
				printf("Text hash = %d\n", t);
				#endif

				hash_out = t;
			}
	out.write(word);
}
