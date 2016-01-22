#include<stdio.h>

class serial_plat {
    char mSerialName[128];
public:
    int mFd;
public:
    bool init(int rate);
    int read(void *buf, int length);
    int write(void *buf, int length);
    serial_plat(char * serial_name);
    ~serial_plat();
};
