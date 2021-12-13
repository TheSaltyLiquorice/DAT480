#include "./kernel.h"

using namespace std;

int main()
{
   FILE *fp;
//   ap_uint<8> x[22*NUM_BYTES];
   uint8_t x[100000] = {0};
   uint16_t golden[1000000] = {0};
   ap_uint<12> y[NUM_BYTES/2];
   int sz = SIZE;
   int bpb = BYTES_PER_BEAT;

   int i = 0;
   fp=fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/test_vec_input.txt","r");
   if(fp == NULL){
	   cout << "ERROR OPENING FILE\n";
	   exit(1);

   }
   fseek(fp, 0L, SEEK_END);
   int sz_f = ftell(fp);
   fseek(fp, 0L, SEEK_SET);

   for (i=0; i<sz_f; i++){
	  uint8_t tmp;
	  fscanf(fp, "%c", &tmp);
	  x[i] = tmp;
	  printf("%c ", tmp);
   }
   fclose(fp);


   fp=fopen("/home/jonathan/Documents/Chalmers/Year5/DAT480/Lab_Project/DAT480/Rules_div_by_length/test_vec_gold.txt","r");
   if(fp == NULL){
	   cout << "ERROR OPENING FILE\n";
	   exit(1);

   }
   fseek(fp, 0L, SEEK_END);
   sz_f = ftell(fp);
   fseek(fp, 0L, SEEK_SET);
   printf("\n");
   for (i=0; i<sz_f; i++){
	  int tmp;
	  fscanf(fp, "%d", &tmp);
	  golden[i] = tmp & 0xffff;
	  printf(" golden = %d\n",golden[i]);
   }
   fclose(fp);


  hls::stream<pkt> in, out;
  pkt word_in, word_out;
  

  int count = 0;

for(int i = 0; i<100000/sz; i++){
	for (unsigned int l = 0; l < sz/bpb; l++) {
	//      printf("writing\n");
	      ap_uint<512> res;
	      for(int j=0;j<NUM_BYTES;j++)
	      {
	       res.range((j*8),(j*8)+7) = x[i*sz+NUM_BYTES*l+j];
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
		for(int j=0;j<512/12;j++)
		{
		ap_uint<12> result = word_out.data.range((j*12),(j*12)+11);
			if(result != 4095){
				y[count] = result;
				count++;
//				cout << "Testbench receives idx = " << result << endl;
			}
		}

	}
}

printf("------------------------------------\n");
  for(int i = 0; i<10; i++){
//	  cout << "y = " << y[i+1];
	  printf(" golden = %d\n",golden[i]);
//	 cout << "y = " << y[i+1] << "x = " << x[i] << endl;
//	 if(y[i+1] != x[i]){
//		 cout << "TB failed\n";
//		 return 0;
//	 }
  }


  printf("Reached end of tb\n");
  return 1;
}

