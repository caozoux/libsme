TARGET := socket.a
TAR_TEST :=socket_cli
CFLAGS += -O2 -I../server/
LDFLAGS +=-lasound  -lpthread 

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.c))
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@
$(TARGET):  $(OBJECTS)
	$(CC) $(INCLUDE) -o $@ -shared -fPIC $(CFLAGS) $(LDFLAGS) $^ $(LIBS)
#$(CROSS)strip $(TARGET)

test:
	gcc -o $(TAR_TEST) socket_client.c $(LDFLAGS) $(CFLAGS)

clean:
	@rm *.o $(TARGET) -rf
