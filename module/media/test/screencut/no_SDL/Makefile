TARGET := screen_recoard
CFLAGS := -g -o2 
LDFLAGS +=-lasound -lxcb -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lxcb-shape -lxcb -lX11 -lasound -lSDL -lm -lz -pthread 
INCLUDE += -I. -I/home/zoucao/github/libsme/module/sockect/server/ -I/home/zoucao/github/ffmpeg
LIBS :=/home/zoucao/github/ffmpeg/./libavformat/libavformat.a
LIBS +=/home/zoucao/github/ffmpeg/./libavdevice/libavdevice.a
LIBS +=/home/zoucao/github/ffmpeg/./libavfilter/libavfilter.a
LIBS +=/home/zoucao/github/ffmpeg/./libavcodec/libavcodec.a
LIBS +=/home/zoucao/github/ffmpeg/./libavutil/libavutil.a
LIBS +=/home/zoucao/github/ffmpeg/./libswscale/libswscale.a
LIBS +=/home/zoucao/github/ffmpeg/./libswresample/libswresample.a

CC := g++

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
%.o:%.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@ 
$(TARGET):  $(OBJECTS)
	$(CC)   -o $(TARGET) $(OBJECTS) $(LIBS) $(LDFLAGS)
	#$(CC) $(OBJECTS)  $(LIBS) -o $(TARGET) $(LDFLAGS)
#$(CROSS)strip $(TARGET)

clean:
	@rm *.o $(TARGET) -rf
