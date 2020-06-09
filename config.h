#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEBUG 1

#define YUYV 1
//#define MJPEG 1

#define WIDTH 320
#define HEIGHT 240
#define FPS 30

#if (YUYV + MJPEG != 1)
#error "please Choose one , #define YUYV or MJPEG,"
#endif

#endif