CC = gcc

inc = -I $(HOME)/build_libs/include/
libs = $(HOME)/build_libs/lib/libx264.a

prom = raw_to_h264

obj = main.o v4l2_device.o x264_encoder.o
#deps = main.h

$(prom): $(obj)
	$(CC) $(inc) -o $(prom) $(obj) $(libs)  -ldl  -m64  -lm -lpthread -ldl 
	rm -rf $(obj)

main.o: main.c 
	$(CC) $(inc)  -c main.c 

%.o: %.c 
	$(CC) $(inc)  -c $< -o $@ 

clean:
	rm -rf $(obj) $(prom) *.jpg *.yuv *.yuv *.h264 *.mjpeg