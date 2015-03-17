
CC:=$(CROSS)g++
AR:=$(CROSS)ar
LD:=$(CROSS)ld
STRIP:=$(CROSS)strip

MAKEARGS := CC=$(CROSS)g++ AR=$(CROSS)ar LD=$(CROSS)ld
TARGET = libsme.so
INCLUDE += -I.
#INCLUDE += -I my_lib
#INCLUDE += -I av_chip
#INCLUDE += -I ipcam_proxy

LDFLAGS += -lpthread
#LDFLAGS += -lusb
#LDFLAGS += -lhd
#LDFLAGS += -lcore
#LDFLAGS += -lutility

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#OBJECTS += $(patsubst %.cpp, %.o, $(wildcard my_lib/*.cpp))
#OBJECTS += $(patsubst %.cpp, %.o, $(wildcard av_chip/*.cpp))
#OBJECTS += $(patsubst %.cpp, %.o, $(wildcard ipcam_proxy/*.cpp))

SUBDIR = module
#%.o:%.cpp
#	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@

#$(TARGET):  $(OBJECTS)
#	$(CC) $(INCLUDE) -o $@ -shared -fPIC $(CFLAGS) $(LDFLAGS) $^ $(LIBS)
#	$(CROSS)strip $(TARGET)
$(TARGET): $(SUBDIR) 
	@make $(MAKEARGS) -C $^ 

clean: $(SUBDIR)
	@make -C $^ clean
	@rm *.o $(TARGET) -rf
	
