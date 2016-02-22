TARGET := socket.a
TAR_TEST :=socket_ser
#CFLAGS += -O2 -fPIC
CFLAGS += -O2 
#LDFLAGS +=-lasound  -lpthread 
LDFLAGS +=
CC := g++

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
%.o:%.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@
$(TARGET):  $(OBJECTS)
	$(AR) rcs  $@  $^ $(LIBS) 
	#$(CC) $(INCLUDE) -o $@ -shared -fPIC $(CFLAGS) $(LDFLAGS) $^ $(LIBS)
	#$(CC) $(INCLUDE) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LIBS)
#$(CROSS)strip $(TARGET)

test:
	gcc -o $(TAR_TEST) socket_server.c $(LDFLAGS) $(CFLAGS)

clean:
	@rm *.o $(TARGET) -rf
