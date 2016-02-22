TARGET := mesend_mail 
CFLAGS := -g -o2 
LDFLAGS +=-lasound  -lpthread
INCLUDE += -I. -I/home/zoucao/github/libsme/module/sockect/server/ -I/home/zoucao/work/libmail-0.3/
LIBS :=/home/zoucao/github/libsme/module/sockect/server/socket.a  libmail.a
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
