# video_util_code
一些实用的视频处理代码

./raw_to_h264 /dev/video0

注意format的可选项有：420：yv12 i420 422：yuy2 uyvy
mplayer  -rawvideo format=help 

mplayer -demuxer rawvideo -rawvideo w=176:h=144 test.yuv
 mplayer -demuxer rawvideo -rawvideo w=320:h=240:format=yuy2 /dev/shm/video.yuv

mplayer -fps 30 video.264
mplayer -fps 30 video.mjpeg

ffmpeg采集摄像头视频数据
ffmpeg -f video4linux2 -s 320*240 -r 10 -i /dev/video0 test.asf

ffplay -f rawvideo -video_size 1920x1080 a.yuv
ffplay -i raw_out2.yuv -pix_fmt yuv422p -s 1280x720

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

ffmpeg -f video4linux2 -s qcif -i /dev/video0  ./output.mpg

./ffmpeg -f video4linux2 -r 12 -s  640x480 -i /dev/video0  -f flv rtmp://127.0.0.1:1935/live/live

./ffmpeg -f video4linux2  -r 12 -s 640x480 -i /dev/video0 -vcodec libx264 -f flv rtmp://127.0.0.1:1935/live/live