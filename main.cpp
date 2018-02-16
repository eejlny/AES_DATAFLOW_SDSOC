/*Copyright (c) 2018, Jose Nunez-Yanez*/
/*University of Bristol. ENEAC project*/

#include "aes_enc.h"



uint8_t key[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};  // initial key

#define TIME_STAMP_INIT_SW  unsigned long long clock_start, clock_end;  clock_start = sds_clock_counter();
#define TIME_STAMP_SW  { clock_end = sds_clock_counter(); printf("CPU ON: Average number of processor cycles : %llu \n", (clock_end-clock_start)); clock_start = sds_clock_counter();  }


#define TIME_STAMP_INIT_HW  unsigned long long clock_start_hw, clock_end_hw;  clock_start_hw = sds_clock_counter();
#define TIME_STAMP_HW  { clock_end_hw = sds_clock_counter(); printf("FPGA ON: Average number of processor cycles : %llu \n", (clock_end_hw-clock_start_hw)); clock_start_hw = sds_clock_counter();  }

#define standalone 1

void read_input(uint8_t *state, char *file, unsigned int& block_size)
{

	#ifdef standalone
		for(int i=0; i< block_size; i++)
		{
			*(state+i) = i;
		}
		printf("Loaded generated data\n");
	#else
	   FILE *fp;

	   fp = fopen (file, "r");
	   if (!fp)
	   {
		  printf("file could not be opened for reading\n");
		   exit(1);
	   }
	   fseek(fp, 0L, SEEK_END);
	   block_size = ftell(fp);
	   fread(state, block_size, 1, fp); // Read in the entire block
	   fclose(fp);
   #endif
}

void write_output(uint8_t *state, char *file, unsigned int block_size)
{
	#ifdef standalone
	#else
	 FILE *fp;

	 fp = fopen (file, "w");
	 if (!fp)
	 {
 		printf("file could not be opened for writing\n");
		exit(1);
	 }
	 fwrite(state, block_size, 1, fp); // Read in the entire block
	 fclose(fp);
	#endif
}

void keyexpansion(uint8_t key[32], uint8_t ekey[240]);

int main(int argc, char* argv[])
{
	uint8_t x,y,i;
	//const unsigned int block_size=1000000;

	unsigned int block_size=1000000;
	uint8_t *state;
	uint8_t *cipher;

	char *ifile, *ofile;

	printf("Launching AES\n");

    #ifdef standalone
	#else
		if (argc != 3)
		{
			printf("incorrect number of inputs: aes in_file out_file\n");
			exit(1);
		}
		ifile = argv[1];
		ofile = argv[2];

    #endif




	uint8_t *ekey;
    state = (uint8_t *) sds_alloc(block_size * sizeof(uint8_t));
    if(!state)
   	{
   			printf("could not allocate state memory\n");
   			exit(0);
   	}
	ekey = (uint8_t *) sds_alloc(240 *sizeof(uint8_t));
	cipher = (uint8_t *) sds_alloc(block_size * sizeof(uint8_t));
	if(!cipher)
	{
		printf("could not allocate cipher memory\n");
		exit(0);
	}

	read_input(state, ifile,block_size);

	printf("block_size is %d\n",block_size);

	keyexpansion(key,ekey);


	TIME_STAMP_INIT_HW
	int block_count = block_size/16;
	int new_block_size = block_count*16;
	printf("new_block_size is %d\n",new_block_size);
	data_t * sub_state;
	data_t * sub_cipher;
    if (CU==1) {
    	printf("Using 1 CU\n");
        aes_enc((data_t *)state,(data_t*)cipher,ekey,new_block_size);
    }
    else
    {
     printf("Using 4 CU\n");
     #pragma SDS resource(1)
     #pragma SDS async(1)
	 aes_enc((data_t*)state,(data_t*)cipher,ekey,new_block_size/4);
	 #pragma SDS resource(2)
 	 #pragma SDS async(2)
	 sub_state = (data_t*)(state + new_block_size/4);
	 sub_cipher = (data_t*)(cipher + new_block_size/4);
	 aes_enc(sub_state,sub_cipher,ekey,new_block_size/4);
	 #pragma SDS resource(3)
 	 #pragma SDS async(3)
	 sub_state = (data_t*)(state + 2*new_block_size/4);
	 sub_cipher = (data_t*)(cipher + 2*new_block_size/4);
	 aes_enc(sub_state,sub_cipher,ekey,new_block_size/4);
	 #pragma SDS resource(4)
 	 #pragma SDS async(4)
	 sub_state = (data_t*)(state + 3*new_block_size/4);
	 sub_cipher = (data_t*)(cipher + 3*new_block_size/4);
	 aes_enc(sub_state,sub_cipher,ekey,new_block_size/4);
     #pragma SDS wait(1)
     #pragma SDS wait(2)
     #pragma SDS wait(3)
     #pragma SDS wait(4)
    }
//	aes_enc((data_t*)state,(data_t*)cipher,ekey,block_size);
	TIME_STAMP_HW

	printf("Computation finished\n");

	#ifdef standalone
	#else
		write_output(cipher, ofile,block_size);
	#endif

	for(x=0;x<16;x++){
			printf(" %x", cipher[x]);
	}
	printf("\n");


	read_input(state, ifile,block_size);


	TIME_STAMP_INIT_SW
	aes_enc_sw((uint8_t*)state,(uint8_t*)cipher,ekey,block_size);
	TIME_STAMP_SW


	for(x=0;x<16;x++){
			printf(" %x", (uint8_t)cipher[x]);
	}
	printf("\n");

	exit(0);
}

void keyexpansion(uint8_t key[32], uint8_t ekey[240])
{
	  uint32_t i, j, k;
	  uint8_t temp[4];

	  for(i = 0; i < nk; ++i)
	  {
	    ekey[(i * 4) + 0] = key[(i * 4) + 0];
	    ekey[(i * 4) + 1] = key[(i * 4) + 1];
	    ekey[(i * 4) + 2] = key[(i * 4) + 2];
	    ekey[(i * 4) + 3] = key[(i * 4) + 3];
	  }


	  for(; (i < (nb * (nr + 1))); ++i)
	  {
	    for(j = 0; j < 4; ++j)
	    {
	      temp[j]= ekey[(i-1) * 4 + j];
	    }
	    if (i % nk == 0)
	    {
	      {
	        k = temp[0];
	        temp[0] = temp[1];
	        temp[1] = temp[2];
	        temp[2] = temp[3];
	        temp[3] = k;
	      }


	      {
	        temp[0] = sbox[temp[0]];
	        temp[1] = sbox[temp[1]];
	        temp[2] = sbox[temp[2]];
	        temp[3] = sbox[temp[3]];
	      }

	      temp[0] =  temp[0] ^ Rcon[i/nk];
	    }
	    else if (nk > 6 && i % nk == 4)
	    {
	      // Function Subword()
	      {
	        temp[0] = sbox[temp[0]];
	        temp[1] = sbox[temp[1]];
	        temp[2] = sbox[temp[2]];
	        temp[3] = sbox[temp[3]];
	      }
	    }
	    ekey[i * 4 + 0] = ekey[(i - nk) * 4 + 0] ^ temp[0];
	    ekey[i * 4 + 1] = ekey[(i - nk) * 4 + 1] ^ temp[1];
	    ekey[i * 4 + 2] = ekey[(i - nk) * 4 + 2] ^ temp[2];
	    ekey[i * 4 + 3] = ekey[(i - nk) * 4 + 3] ^ temp[3];
	  }

}

