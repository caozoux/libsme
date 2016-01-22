TARGET := audio.a
TAR_TEST := ttyserial_test
OBJECTS_TEST = 
CFLAGS := -g -o2
LDFLAGS +=-lasound  -lpthread
INCLUDE += -I. -I../include/ -I/home/zoucao/github/tinyxml-git
LIBS :=/home/zoucao/github/tinyxml-git/tinyxml.a
CC := g++

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
%.o:%.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@ 
$(TARGET):  $(OBJECTS)
	$(AR) rcs  $@  $^ $(LIBS) 
#$(CROSS)strip $(TARGET)

test: $(TAR_TEST)
$(TAR_TEST) : $(OBJECTS)   $(OBJECTS_TEST)
	$(CC) $(OBJECTS) $(OBJECTS_TEST) $(LIBS) -o $(TAR_TEST) $(LDFLAGS)
$(OBJECTS_TEST) : $(patsubst %.o, %.c, $(OBJECTS_TEST))
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@  

clean:
	@rm *.o $(TARGET) -rf
