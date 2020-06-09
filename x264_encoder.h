#ifndef _X264_ENCODER_
#define _X264_ENCODER_

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "x264.h"
#include "x264_config.h"

struct X264Encoder {
  FILE* fp_dst;

  int iNal;
  x264_nal_t* pNals;
  x264_t* pHandle;
  x264_picture_t pPic_in;
  x264_picture_t pPic_out;
  x264_param_t pParam;
};

typedef struct X264Encoder* X264Encoder_P;

struct X264Encoder* NewX264Encoder();

void x264close(struct X264Encoder* x);
void x264encode(struct X264Encoder* x, void* start, size_t length) ;

#endif
