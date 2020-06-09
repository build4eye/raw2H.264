#include "config.h"
#include "v4l2_device.h"
#include "x264_encoder.h"

void cameraParamInit(struct Camera* c) {
  c->Param.format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

#if YUYV
  c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
#endif
#if MJPEG
  c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
#endif
  c->Param.format.fmt.pix.height = HEIGHT;
  c->Param.format.fmt.pix.width = WIDTH;
  // c->Param.format.fmt.pix.field = V4L2_FIELD_NONE;
  c->Param.format.fmt.pix.field = V4L2_FIELD_NONE;
  //设置流相关，帧率
  c->Param.stream.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // TOTO:denominator
  c->Param.stream.parm.capture.timeperframe.denominator = FPS;
  c->Param.stream.parm.capture.timeperframe.numerator = 1;

  c->Param.reqbuf.count = 5;
  c->Param.reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  c->Param.reqbuf.memory = V4L2_MEMORY_MMAP;
  c->Init(c);

#if DEBUG
  printf("denominator = %d \n",
         c->Param.stream.parm.capture.timeperframe.denominator);
#endif
}

void process_image(void* start, size_t length) {
#if YUYV
  int fd = open("/dev/shm/video.yuv", O_RDWR | O_CREAT | O_APPEND, 0777);
  write(fd, (char*)start, length);
  close(fd);
#endif
#if MJPEG
  int fd = open("/dev/shm/video.mjpeg", O_RDWR | O_CREAT | O_APPEND, 0777);
  write(fd, (char*)start, length);
  close(fd);
#endif
}

void print_current_fps(long long int time) {
  static long long int _t = 0;

  printf("fps: %f\n", 1 / ((time - _t) * 0.000001));

  _t = time;
}

int main(int argc, char* argv[]) {
  if (argc != 2) return -1;

  //创建一个摄像头
  struct RawBuffer* buf = (void*)0;
  struct Camera* camera = NewCamera(argv[1]);
  cameraParamInit(camera);

  // TODO:创建一个mjpeg转rgb 的转码器

  // TODO:创建一个h264编码器,暂时支持ｘ264
  X264Encoder_P x264 = NewX264Encoder();

  // TODO:v4l2驱动开始采集
  camera->Start(camera);

  for (;;) {
    buf = camera->PopRaw(camera);

#if DEBUG
    print_current_fps(buf->timestamp);
#endif

    //存本地文件
    process_image(buf->start, camera->Param.format.fmt.pix.height *
                                  camera->Param.format.fmt.pix.width * 2);

    // TODO:mjpeg转rgb

    // TODO:h264编码
    x264encode(x264, buf->start,
               camera->Param.format.fmt.pix.height *
                   camera->Param.format.fmt.pix.width * 2);

    // TODO:RTSP推流

    camera->PushRaw(camera, buf);
  }

  x264close(x264);

  camera->Stop(camera);

  camera->Close(camera);
}
