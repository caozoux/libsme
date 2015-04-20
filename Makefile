
CC:=$(CROSS)g++
AR:=$(CROSS)ar
LD:=$(CROSS)ld
STRIP:=$(CROSS)strip

HOST_OS := linux
MAKE := @make
RM := @rm
ifneq ($(DBG),)
	MAKE = make
endif

MAKE :=make
Q :=@
MAKEARGS := CC=$(CROSS)g++ AR=$(CROSS)ar LD=$(CROSS)ld
TARGET = libsme.so
INCLUDE += -I.

LDFLAGS += -lpthread
CFLAGS += -fPIC

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#OBJECTS += $(patsubst %.cpp, %.o, $(wildcard ipcam_proxy/*.cpp))
export MAKE CC AR LD STRIP HOST_OS RM INCLUDE LDFLAGS CFLAGS Q

SUBDIR = module

$(TARGET): $(SUBDIR) 
	$(Q)$(MAKE) $(MAKEARGS) -C $^

clean: $(SUBDIR)
	$(Q)$(MAKE) -C $^ clean
	$(Q)$(RM) *.o $(TARGET) -rf
	
