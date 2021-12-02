#include "./kernel.h"

using namespace std;

int main()
{
   FILE *fp;
   ap_uint<8> x[NUM_BYTES];
   int i = 0;
   fp=fopen("/home/jonathan/test_string.txt","r");
   for (i=0; i<NUM_BYTES; i++){
	  ap_uint<8> tmp;
	  fscanf(fp, "%c", &tmp);
	  x[i] = tmp;
   }
   fclose(fp);

   for (i=0; i<NUM_BYTES; i++){
	   printf("%c", x[i].to_char());
   }
  hls::stream<pkt> in, out;
  pkt tmp1, tmp2;
  
  ap_uint<DWIDTH> temp  = 0;
  unsigned int hash_out;
  for(int i=0;i<NUM_BYTES;i++)
  {
   temp.range((i*8),(i*8)+7) = x[i];
  }

  tmp1.data = temp;

  in.write(tmp1);
  krnl_hash(in,out,hash_out);

  out.read(tmp2);
  printf("Hash out %x\n",hash_out);
  return 0;
}

