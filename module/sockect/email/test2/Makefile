TARGET := mesend_mail 
CFLAGS := -g -o2 
LDFLAGS +=-lpthread
INCLUDE += -I. -I/home/zoucao/github/libsme/module/sockect/server/ 
#LIBS :=/home/zoucao/github/libsme/module/sockect/server/socket.a 
LIBS =
LIBS +=/home/zoucao/github/eHCR-1000/mail/mailsend.a
LIBS +=/home/zoucao/github/eHCR-1000/mail/libmsock.a
LIBS +=/home/zoucao/github/eHCR-1000/mail/libmutils.a
LIBS +=/home/zoucao/github/eHCR-1000/mail/libsll.a
CC := arm-linux-gnueabihf-g++

OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c))
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@ 
$(TARGET):  $(OBJECTS)
	$(CC) $(OBJECTS)  $(LIBS) -o $(TARGET) $(LDFLAGS)
	#$(CC) $(OBJECTS)  $(LIBS) -o $(TARGET) $(LDFLAGS) 
#$(CROSS)strip $(TARGET)

clean:
	@rm *.o $(TARGET) -rf
