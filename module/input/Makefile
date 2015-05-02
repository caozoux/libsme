
TARGET := input.a
TAR_TEST :=in_test

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.c))
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@
$(TARGET):  $(OBJECTS)
	$(CC) $(INCLUDE) -o $@ -shared -fPIC $(CFLAGS) $(LDFLAGS) $^ $(LIBS)
#$(CROSS)strip $(TARGET)

test:
	g++ -o $(TAR_TEST) input_read.cpp

clean:
	@rm *.o $(TARGET) -rf
