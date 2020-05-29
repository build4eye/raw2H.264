//一个人性化的v4l2设备操作库
#include "v4l2_device.h"

#include <sys/mman.h>

#define DEFBUFCOUNT 5  //默认缓存映射大小
#define ERR_EXIT(m)                                       \
  do {                                                    \
    char out[50];                                         \
    sprintf(out, "%s line:%d %s", __FILE__, __LINE__, m); \
    perror(out);                                          \
    exit(EXIT_FAILURE);                                   \
  } while (0)

struct CameraClass {
  struct Camera Camera;

  int fd;
  char cameraName[50];
  struct v4l2_buffer buf;
  struct RawBuffer* bufs;
};
typedef struct CameraClass* CameraClass_P;

static void setMmap(struct Camera* camera);
static void _start(struct Camera* camera);
static void _stop(struct Camera* camera);
static struct RawBuffer* _popRaw(struct Camera* camera);
static void _pushRaw(struct Camera* camera, struct RawBuffer* buf);
static void _init(struct Camera* camera);
static void _close(struct Camera* camera);
static char* _getVideoPixelFormat(struct Camera* camera, unsigned int index);
static int _isSuppertPixelFormat(struct Camera* camera, unsigned int format);

struct Camera* NewCamera(const char* cameraName) {
  CameraClass_P c = (CameraClass_P)(malloc(sizeof(struct CameraClass)));
  if (c == 0) return (void*)0;
  memset(c, 0, sizeof(struct CameraClass));

  //阻塞模式打开摄像头设备
  c->fd = open(cameraName, O_RDWR);
  //非阻塞模式打开摄像头设备
  // c->fd = open(cameraName, O_RDWR|O_NONBLOCK);
  if (c->fd == 0) return (void*)0;

  c->Camera.Init = _init;
  c->Camera.Close = _close;
  c->Camera.GetVideoPixelFormat = _getVideoPixelFormat;
  c->Camera.IsSuppertPixelFormat = _isSuppertPixelFormat;
  c->Camera.PopRaw = _popRaw;
  c->Camera.PushRaw = _pushRaw;
  c->Camera.Start = _start;
  c->Camera.Stop = _stop;

  struct CameraInfo* info = (struct CameraInfo*)c;
  //获取设备信息
  if (0 != ioctl(c->fd, VIDIOC_QUERYCAP, &(info->cap))) {
    ERR_EXIT("VIDIOC_QUERYCAP");
  }

  //获取已经配置帧格式
  info->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_G_FMT, &(info->format))) {
    ERR_EXIT("VIDIOC_G_FMT");
  }

  //获取已经配置　流相关，帧率
  info->stream.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_G_PARM, &(info->stream))) {
    ERR_EXIT("VIDIOC_G_PARM");
  }

  //设置内存映射相关参数
  info->reqbuf.count = DEFBUFCOUNT;

  return (struct Camera*)c;
}

//申请用户空间的地址列
static void setMmap(struct Camera* camera) {
  if (camera == 0) return;

  CameraClass_P c = (CameraClass_P)camera;
  struct CameraInfo* info = (struct CameraInfo*)c;

  c->bufs = calloc(1, info->reqbuf.count * sizeof(struct RawBuffer));
  for (unsigned char _i = 0; _i < info->reqbuf.count; _i++) {
    c->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    c->buf.memory = V4L2_MEMORY_MMAP;
    c->buf.index = _i;
    //查询
    if (0 != ioctl(c->fd, VIDIOC_QUERYBUF, &(c->buf))) {
      ERR_EXIT("VIDIOC_QUERYBUF");
    }
    c->bufs[_i].length = c->buf.length;
    c->bufs[_i].start = mmap(NULL, c->buf.length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, c->fd, c->buf.m.offset);

    //入队
    if (0 != ioctl(c->fd, VIDIOC_QBUF, &(c->buf))) {
      ERR_EXIT("VIDIOC_QBUF");
    }
  }
}

//_close　释放资源
static void _close(struct Camera* camera) {
  if (camera == 0) return;

  CameraClass_P c = (CameraClass_P)camera;

  //停止采集
  camera->Stop(camera);

  enum v4l2_buf_type type;
  //关闭流
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_STREAMON, &type)) {
    ERR_EXIT("VIDIOC_STREAMON");
  }

  free(c->bufs);

  close(c->fd);

  free(c);
}

//_init　将相关参数配置到设备
//设置帧格式 设置流相关，帧率。设置内存映射
//用户NewCamera()后，可以得到Camera类，可以通过camera->info自定义参数，然后调用本方法
//也可以使用Camera类默认的参数
static void _init(struct Camera* camera) {
  if (camera == 0) goto _exit;
  CameraClass_P c = (CameraClass_P)camera;
  struct CameraInfo* info = (struct CameraInfo*)camera;

  //设置帧格式
  if (0 != ioctl(c->fd, VIDIOC_S_FMT, &(info->format))) {
    ERR_EXIT("VIDIOC_S_FMT");
  }
  //设置流相关，帧率
  if (0 != ioctl(c->fd, VIDIOC_S_PARM, &(info->stream))) {
    ERR_EXIT("VIDIOC_S_PARM");
  }
  
  //获取已经配置帧格式
  info->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_G_FMT, &(info->format))) {
    ERR_EXIT("VIDIOC_G_FMT");
  }
  //获取已经配置　流相关，帧率
  info->stream.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_G_PARM, &(info->stream))) {
    ERR_EXIT("VIDIOC_G_PARM");
  }

  //设置内存映射相关参数
  if (0 != ioctl(c->fd, VIDIOC_REQBUFS, &(info->reqbuf))) {
    ERR_EXIT("VIDIOC_REQBUFS");
  }

_exit:
  return;
}

//_getVideoPixelFormat　获取设备支持的格式：eg:YUYV 4:2:2 or Motion-JPEG
static char* _getVideoPixelFormat(struct Camera* camera, unsigned int index) {
  if (camera == 0) goto _exit;
  CameraClass_P c = (CameraClass_P)camera;

  static struct v4l2_fmtdesc fmtdesc;
  fmtdesc.index = index;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(c->fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    return fmtdesc.description;
  }

_exit:
  return (void*)0;
}

//_isSuppertPixelFormat　判断设备是否支持每种格式
// eg: format = V4L2_PIX_FMT_RGB32
static int _isSuppertPixelFormat(struct Camera* camera, unsigned int format) {
  if (camera == 0) goto _exit;
  CameraClass_P c = (CameraClass_P)camera;

  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(struct v4l2_format));

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = format;
  if (ioctl(c->fd, VIDIOC_TRY_FMT, &fmt) != 0) return 0;

_exit:
  return 1;
}
//开始采集
static void _start(struct Camera* camera) {
  if (camera == 0) return;
  CameraClass_P c = (CameraClass_P)camera;

  struct CameraInfo* info = (struct CameraInfo*)camera;

  info->reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  info->reqbuf.memory = V4L2_MEMORY_MMAP;
  if (0 != ioctl(c->fd, VIDIOC_REQBUFS, &(info->reqbuf))) {
    ERR_EXIT("VIDIOC_REQBUFS");
  }
  //申请用户空间的地址列
  setMmap((struct Camera*)c);

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_STREAMON, &type)) {
    ERR_EXIT("VIDIOC_STREAMON");
  }
}
//结束采集
static void _stop(struct Camera* camera) {
  if (camera == 0) return;
  CameraClass_P c = (CameraClass_P)camera;

  //关闭内存映射
  for (char _i = 0; _i < c->Camera.Param.reqbuf.count; _i++) {
    munmap(c->bufs[_i].start, c->bufs[_i].length);
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 != ioctl(c->fd, VIDIOC_STREAMOFF, &type)) {
    ERR_EXIT("VIDIOC_STREAMOFF");
  }
}

//出队一帧数据缓存
static struct RawBuffer* _popRaw(struct Camera* camera) {
  if (camera == 0) return (void*)0;
  CameraClass_P c = (CameraClass_P)camera;

  memset(&(c->buf), 0, sizeof(struct v4l2_buffer));
  c->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  c->buf.memory = V4L2_MEMORY_MMAP;

  //出队
  if (0 != ioctl(c->fd, VIDIOC_DQBUF, &(c->buf))) {
    ERR_EXIT("VIDIOC_DQBUF");
  }
  c->bufs[c->buf.index].timestamp =
      c->buf.timestamp.tv_sec * 1000000 + c->buf.timestamp.tv_usec;
  c->bufs[c->buf.index].index = c->buf.index;

  return c->bufs + c->buf.index;
}

//入队数据缓存
static void _pushRaw(struct Camera* camera, struct RawBuffer* buf) {
  if (camera == 0) return;
  CameraClass_P c = (CameraClass_P)camera;

  memset(&(c->buf), 0, sizeof(struct v4l2_buffer));
  c->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  c->buf.memory = V4L2_MEMORY_MMAP;
  c->buf.index = buf->index;
  //出队
  if (0 != ioctl(c->fd, VIDIOC_QBUF, &(c->buf))) {
    ERR_EXIT("VIDIOC_QBUF");
  }

  return;
}
