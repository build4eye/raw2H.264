#include "x264_encoder.h"

#include "config.h"

// NewX264Encoder 新建一个h264编码器
struct X264Encoder* NewX264Encoder() {
  X264Encoder_P x = (X264Encoder_P)(malloc(sizeof(struct X264Encoder)));
  if (x == 0) return (void*)0;
  memset(x, 0, sizeof(struct X264Encoder));

  // TODO:
  x->fp_dst = fopen("/dev/shm/video.h264", "wb");

  x->iNal = 0;
  x->pNals = NULL;
  x->pHandle = NULL;
  x->pPic_in.i_pts = 0;

  x264_param_default(&(x->pParam));
  x->pParam.i_width = WIDTH;
  x->pParam.i_height = HEIGHT;
#if YUYV
  x->pParam.i_csp = X264_CSP_I422;
#endif
  x->pParam.i_fps_den = 1;
  x->pParam.i_fps_num = FPS;
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
  x264_param_apply_profile(&(x->pParam), x264_profile_names[5]);
  x->pHandle = x264_encoder_open(&(x->pParam));

  x264_picture_init(&(x->pPic_out));
  x264_picture_alloc(&(x->pPic_in), x->pParam.i_csp, x->pParam.i_width,
                     x->pParam.i_height);
  return x;
}

void x264encode(struct X264Encoder* x, void* start, size_t length) {
  int ret;
  unsigned int len = length;
  unsigned int y_i = 0;
  unsigned int u_i = 0;
  unsigned int v_i = 0;
  char * buf = start;

#if YUYV
  for (int _i = 0; _i < len;_i = _i + 4) {
    x->pPic_in.img.plane[0][y_i++] = *(buf++);
    x->pPic_in.img.plane[1][u_i++] = *(buf++);
    x->pPic_in.img.plane[0][y_i++] = *(buf++);
    x->pPic_in.img.plane[2][v_i++] = *(buf++);
  }
#endif

  ret = x264_encoder_encode(x->pHandle, &(x->pNals), &(x->iNal), &(x->pPic_in),
                            &(x->pPic_out));

  if (ret < 0) {
    printf("Error.\n");
  }
  x->pPic_in.i_pts = x->pPic_in.i_pts + 1;


  for (int j = 0; j < x->iNal; ++j) {
    fwrite(x->pNals[j].p_payload, 1, x->pNals[j].i_payload, x->fp_dst);
  }
  fflush(x->fp_dst);
}

void x264close(struct X264Encoder* x) {
  x264_picture_clean(&(x->pPic_in));
  x264_encoder_close(x->pHandle);
  x->pHandle = NULL;

  fclose(x->fp_dst);
}