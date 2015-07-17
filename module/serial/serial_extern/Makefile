TARGET := audio.a
TAR_TEST := ttyserial_msg
OBJECTS_TEST = 
CFLAGS := -g -o2
LDFLAGS +=-lasound  -lpthread
INCLUDE += -I. -I../include/

OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c))
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@ 
$(TARGET):  $(OBJECTS)
	$(AR) rcs  $@  $^ $(LIBS) 
#$(CROSS)strip $(TARGET)

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>test module>>>>>>>>>>>>>>>>>>>>>>>
test: $(TAR_TEST)
$(TAR_TEST) : $(OBJECTS)   $(OBJECTS_TEST)
	$(CC) $(OBJECTS) $(OBJECTS_TEST) -o $(TAR_TEST) $(LDFLAGS)
$(OBJECTS_TEST) : $(patsubst %.o, %.c, $(OBJECTS_TEST))
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@  
#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<test module<<<<<<<<<<<<<<<<<<<<<<<

clean:
	@rm *.o $(TARGET) -rf
