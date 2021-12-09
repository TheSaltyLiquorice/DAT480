#include "./kernel.h"

using namespace std;

int main()
{
   FILE *fp;
   ap_uint<8> x[22*NUM_BYTES];
   uint16_t y[NUM_BYTES];
   int sz = SIZE;
   int bpb = BYTES_PER_BEAT;

   int i = 0;
   fp=fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/test_vec_input.txt","r");
   if(fp == NULL){
	   cout << "ERROR OPENING FILE\n";
	   exit(1);

   }
   for (i=0; i<22*NUM_BYTES; i++){
	  ap_uint<8> tmp;
	  fscanf(fp, "%c", &tmp);
	  x[i] = tmp;
   }
   fclose(fp);

   for (i=0; i<22*NUM_BYTES; i++){
	   printf("%c", x[i].to_char());
   }
  hls::stream<pkt> in, out;
  pkt word_in, word_out;
  



  for (unsigned int l = 0; l < 3; l++) {
      printf("writing\n");
      ap_uint<512> res;
      for(int i=0;i<NUM_BYTES;i++)
      {
       res.range((i*8),(i*8)+7) = x[NUM_BYTES*l+i];
      }

      word_in.data = res;
      word_in.keep = -1;

      // set last signals when last piece of data or
      // multiple of 1408 bytes, packetize the payload
      if ((((sz/bpb) - 1)==l) || ((((l + 1) * bpb) % sz) == 0))
        word_in.last = 1;
      else
        word_in.last = 0;

      word_in.dest = 0; //not sure if i need to set this in the tb
      in.write(word_in);
      krnl_hash(in,out);
      out.read(word_out);
	for(int i=0;i<512/16;i++)
	{
	 y[i] = word_out.data.range((i*16),(i*16)+15);
	 cout << "Testbench receives idx = " << y[i] << endl;
	}

    }
  printf("Reached end of tb\n");
  return 0;
}

