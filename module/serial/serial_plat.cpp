#include<string.h>	/* String function definitions */
#include<unistd.h>	/* UNIX standard function definitions */
#include<fcntl.h>	/* File control definitions */
#include<errno.h>	/* Error number definitions */
#include<termios.h>	/* POSIX terminal control definitions */
#include<pthread.h>
#include<sys/eventfd.h>
#include<sys/epoll.h>
#include<sys/msg.h> 
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<getopt.h>
#include "serial_plat.h"


bool serial_plat::init(int rate)
{
	struct termios options;
	int fd;

	/*以非阻塞方式打开串口*/
	fd = open(mSerialName, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf("Open the serial port error:%s!\n", mSerialName);
		return -1;
	}

	mFd = fd;
    /**1. tcgetattr函数用于获取与终端相关的参数。 
    *参数fd为终端的文件描述符，返回的结果保存在termios结构体中 
    */  
    tcgetattr(fd, &options);  
    /**2. 修改所获得的参数*/  
    options.c_cflag |= (CLOCAL | CREAD);//设置控制模式状态，本地连接，接收使能  
    options.c_cflag &= ~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位  
    options.c_cflag &= ~CRTSCTS;//无硬件流控  
    options.c_cflag |= CS8;//8位数据长度  
    options.c_cflag &= ~CSTOPB;//1位停止位  
    options.c_iflag |= IGNPAR;//无奇偶检验位  
    options.c_oflag = 0; //输出模式  
    options.c_lflag = 0; //不激活终端模式  
    cfsetospeed(&options, B115200);//设置波特率  
      
    /**3. 设置新属性，TCSANOW：所有改变立即生效*/  
    tcflush(fd, TCIFLUSH);//溢出数据可以接收，但不读  
    tcsetattr(fd, TCSANOW, &options); 
    return 0;
}
int serial_plat::read(void *buf, int length)
{
        return ::read(mFd, buf,length);
}
int serial_plat::write(void *buf, int length)
{
        return ::write(mFd, buf, length);
}

serial_plat::serial_plat(char * serial_name)
{
    strcpy(mSerialName, serial_name);
}

serial_plat::~serial_plat()
{
    if (mFd >0) {
       close(mFd);
    }
}
