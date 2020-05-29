#ifndef _X264_ENCODER_
#define _X264_ENCODER_

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "x264.h"
#include "x264_config.h"

struct X264EncoderClass {
  FILE* fp_dst;

  int iNal;
  x264_nal_t* pNals;
  x264_t* pHandle;
  x264_picture_t pPic_in;
  x264_picture_t pPic_out;
  x264_param_t pParam;
};
typedef struct X264EncoderClass* X264EncoderClass_P;

struct X264EncoderClass* NewX264EncoderClass();

void x264close(struct X264EncoderClass* c);
void encode(struct X264EncoderClass* c, void* start, size_t length) ;

#endif
