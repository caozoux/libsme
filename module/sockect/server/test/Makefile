TARGET := socket_server
TAR_TEST := ttyserial_test
OBJECTS_TEST = 
CFLAGS := -g -o2 
LDFLAGS +=-lasound  -lpthread
INCLUDE += -I. -I/home/zoucao/github/libsme/module/sockect/server/
LIBS :=/home/zoucao/github/libsme/module/sockect/server/socket.a
CC := g++

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
%.o:%.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@ 
$(TARGET):  $(OBJECTS)
	$(CC) $(OBJECTS)  $(LIBS) -o $(TARGET) $(LDFLAGS)
	#$(CC) $(OBJECTS)  $(LIBS) -o $(TARGET) $(LDFLAGS) 
#$(CROSS)strip $(TARGET)

clean:
	@rm *.o $(TARGET) -rf
