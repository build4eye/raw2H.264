#ifndef _V4L3_DEVICE_
#define _V4L3_DEVICE_

//一个人性化的v4l2设备操作库
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct RawBuffer {
  void* start;
  unsigned int index;
  unsigned int length;
  long long int timestamp;
};

struct CameraInfo {
  struct v4l2_capability cap;         //设备属性
  struct v4l2_format format;          //帧格式
  struct v4l2_streamparm stream;      //流相关 (如帧率)
  struct v4l2_requestbuffers reqbuf;  //内存映射配置
};

struct Camera {
  struct CameraInfo Param;

  //开始采集
  void (*Start)(struct Camera* camera);
  //结束采集
  void (*Stop)(struct Camera* camera);
  //出队一帧数据缓存
  struct RawBuffer* (*PopRaw)(struct Camera* camera);
  //入队数据缓存
  void (*PushRaw)(struct Camera* camera, struct RawBuffer* buf);
  //_init　将相关参数配置到设备
  //设置帧格式 设置流相关，帧率。设置内存映射
  //用户NewCamera()后，可以得到Camera类，可以通过camera->info自定义参数，然后调用本方法
  //也可以使用Camera类默认的参数
  void (*Init)(struct Camera* camera);
  // Close　释放资源
  void (*Close)(struct Camera* camera);
  // GetVideoPixelFormat　获取设备支持的格式：eg:YUYV 4:2:2 or Motion-JPEG
  char* (*GetVideoPixelFormat)(struct Camera* camera, unsigned int index);
  // IsSuppertPixelFormat　判断设备是否支持某种格式
  // eg: format = V4L2_PIX_FMT_RGB32
  int (*IsSuppertPixelFormat)(struct Camera* camera, unsigned int format);
};

struct Camera* NewCamera(const char* cameraName);

#endif
