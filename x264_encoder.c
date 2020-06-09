#include "x264_encoder.h"

struct X264EncoderClass* NewX264EncoderClass() {
  X264EncoderClass_P c =
      (X264EncoderClass_P)(malloc(sizeof(struct X264EncoderClass)));
  if (c == 0) return (void*)0;
  memset(c, 0, sizeof(struct X264EncoderClass));

  c->fp_dst = fopen("/dev/shm/cuc_ieschool.h264", "wb");

  c->iNal = 0;
  c->pNals = NULL;
  c->pHandle = NULL;
  c->pPic_in.i_pts = 0;

  x264_param_default(&(c->pParam));
  c->pParam.i_width = 640;
  c->pParam.i_height = 480;
  c->pParam.i_csp = X264_CSP_YV16;
  c->pParam.i_fps_den = 1;
  c->pParam.i_fps_num = 10;
  /*
          //Param
          pParam->i_log_level  = X264_LOG_DEBUG;
          pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
          pParam->i_frame_total = 0;
          pParam->i_keyint_max = 10;
          pParam->i_bframe  = 5;
          pParam->b_open_gop  = 0;
          pParam->i_bframe_pyramid = 0;
          pParam->rc.i_qp_constant=0;
          pParam->rc.i_qp_max=0;
          pParam->rc.i_qp_min=0;
          pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
          pParam->i_fps_den  = 1;
          pParam->i_fps_num  = 25;
          pParam->i_timebase_den = pParam->i_fps_num;
          pParam->i_timebase_num = pParam->i_fps_den;
  */
  x264_param_apply_profile(&(c->pParam), x264_profile_names[5]);
  c->pHandle = x264_encoder_open(&(c->pParam));

  x264_picture_init(&(c->pPic_out));
  x264_picture_alloc(&(c->pPic_in), c->pParam.i_csp, c->pParam.i_width,
                     c->pParam.i_height);
  return c;
}

void encode(struct X264EncoderClass* c, void* start, size_t length) {
  int ret;
  unsigned int len = length;
  unsigned int y_i = 0;
  unsigned int u_i = 0;
  unsigned int v_i = 0;

  // for (int index = 0; index < len;) {
  //   // printf("%d\n",index);
  //   memset(&(c->pPic_in.img.plane[0][y_i++]), ((char*)start)[index], 1);  // y
  //   index++;
  //   memset(&(c->pPic_in.img.plane[1][u_i++]), ((char*)start)[index], 1);  // U
  //   index++;
  //   memset(&(c->pPic_in.img.plane[0][y_i++]), ((char*)start)[index], 1);  // y
  //   index++;
  //   memset(&(c->pPic_in.img.plane[2][v_i++]), ((char*)start)[index], 1);  // V
  //   index++;
  // }
  
    memcpy(c->pPic_in.img.plane[0], start, len/2);// y
    memcpy(c->pPic_in.img.plane[1], start + len/2 + len/8, len/8);// U
    memcpy(c->pPic_in.img.plane[2], start + len/2, len/8);// V


  ret = x264_encoder_encode(c->pHandle, &(c->pNals), &(c->iNal), &(c->pPic_in),
                            &(c->pPic_out));

  if (ret < 0) {
    printf("Error.\n");
  }

  // printf("Succeed encode frame: %5d\n", c->pPic_in.i_pts);

  c->pPic_in.i_pts = c->pPic_in.i_pts + 1;

  for (int j = 0; j < c->iNal; ++j) {
    fwrite(c->pNals[j].p_payload, 1, c->pNals[j].i_payload, c->fp_dst);
  }
  fflush(c->fp_dst);
}

void x264close(struct X264EncoderClass* c) {
  x264_picture_clean(&(c->pPic_in));
  x264_encoder_close(c->pHandle);
  c->pHandle = NULL;

  fclose(c->fp_dst);
}