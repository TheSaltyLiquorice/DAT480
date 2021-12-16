#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "ap_axi_sdata.h"
#include "ap_int.h"
#include "hls_stream.h"

#define DWIDTH 512
#define TDWIDTH 16
#define d 256
#define BUFFER_WIDTH 7
#define SIZE 1408
#define NUM_BYTES DWIDTH/8
#define BYTES_PER_BEAT DWIDTH/8
#define type ap_uint<DWIDTH>

//typedef hls::axis<type, 0, 0, 0> pkt;
typedef ap_axiu<DWIDTH, 96, 1, TDWIDTH> pkt;

//typedef ap_axiu<DWIDTH, 96, 1, TDWIDTH> pkt;

extern "C"{
	void krnl_hash(
            hls::stream<pkt> &in,
			ap_uint<NUM_BYTES*8> *out
//            hls::stream<pkt> &out
            );
};

uint32_t crc_cal(uint32_t* table, char* buf, int len);

int fun_strncmp_4(char ref1 [4], const char ref2[4]);


#endif // __KERNEL_H__ not defined


