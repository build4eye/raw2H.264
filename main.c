#include "v4l2_device.h"
#include "x264_encoder.h"

void process_image(void* start, size_t length);
void cameraParamInit(struct Camera* c);
struct X264EncoderClass* x;

int main(int argc, char* argv[]) {
  if (argc != 2) return -1;

  x = NewX264EncoderClass();

  struct RawBuffer* buf = (void*)0;
  struct Camera* c = NewCamera(argv[1]);
  cameraParamInit(c);
  

  printf("denominator = %d \n",
         c->Param.stream.parm.capture.timeperframe.denominator);
  c->Start(c);
  int _i = 0;
  struct timeval old;
  struct timeval now;
  while (_i < 40) {
    _i++;

    gettimeofday(&old, NULL);
    buf = c->PopRaw(c);

    process_image(buf->start, c->Param.format.fmt.pix.height *
                                  c->Param.format.fmt.pix.width * 2);

    c->PushRaw(c, buf);

    gettimeofday(&now, NULL);

    printf("time %d\n", now.tv_sec * 1000000 - old.tv_sec * 1000000 +
                            now.tv_usec - old.tv_usec);
  }
  c->Stop(c);

  c->Close(c);
  x264close(x);
}

void cameraParamInit(struct Camera* c) {
  c->Param.format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  //c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  // jpg格式
  //c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV422P;
  //c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV422M;
  c->Param.format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  c->Param.format.fmt.pix.height = 1080;
  c->Param.format.fmt.pix.width = 1920;
  // c->Param.format.fmt.pix.field = V4L2_FIELD_NONE;
  c->Param.format.fmt.pix.field = V4L2_FIELD_NONE;
  //设置流相关，帧率
  c->Param.stream.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // TOTO:denominator
  c->Param.stream.parm.capture.timeperframe.denominator = 30;
  c->Param.stream.parm.capture.timeperframe.numerator = 1;

  c->Param.reqbuf.count = 4;
  c->Param.reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  c->Param.reqbuf.memory = V4L2_MEMORY_MMAP;
  c->Init(c);
}

void process_image(void* start, size_t length) {
  // int fd = open("/dev/shm/video.yuv", O_RDWR | O_CREAT | O_APPEND, 0777);
  // write(fd, (char*)start, length);
  // close(fd);
  encode(x, start, length);
  // int fd = open("/dev/shm/video.mjpeg", O_RDWR | O_CREAT | O_APPEND, 0777);
  // write(fd, (char*)start, length);
  // close(fd);
}
