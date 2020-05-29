# video_util_code
一些实用的视频处理代码

./raw_to_h264 /dev/video0

注意format的可选项有：420：yv12 i420 422：yuy2 uyvy

mplayer -demuxer rawvideo -rawvideo w=176:h=144 test.yuv

mplayer -fps 30 test.264

ffmpeg采集摄像头视频数据
ffmpeg -f video4linux2 -s 320*240 -r 10 -i /dev/video0 test.asf


# 方案１
## LINUX下载编译libx264

git clone https://code.videolan.org/videolan/x264.git

tar xf x264-master.tar.gz
cd x264-master

*.sh
BUILD_LIBS=${HOME}/build_libs
export PATH=${BUILD_LIBS}/bin:${PATH}
./configure --prefix=${BUILD_LIBS} --enable-shared --enable-pic --enable-static
make
make install

# 方案２

