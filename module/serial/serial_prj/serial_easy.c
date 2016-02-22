#include<stdio.h>	/* Standard input/output definitions */
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
#define DEVNAME "/dev/ttyUSB0"

int serial_fd;
int serial_init(void)
{
	struct termios options;
	int fd;

	/*以非阻塞方式打开串口*/
	fd = open(DEVNAME, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf("Open the serial port error:%s!\n", DEVNAME);
		return -1;
	}

	serial_fd = fd;

	fcntl(fd, F_SETFL, 0);

	tcgetattr(fd, &options);

	/*
	 * Set the baud rates to 9600
	 */
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);

	/*
	 * Enable the receiver and set local mode
	 */
	options.c_cflag |= (CLOCAL | CREAD);

	/*
	 * Select 8 data bits, 1 stop bit and no parity bit
	 */
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	/*
	 * Disable hardware flow control
	 */
	options.c_cflag &= ~CRTSCTS;

	/*
	 * Choosing raw input
	 */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/*
	 * Disable software flow control
	 */
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	/*
	 * Choosing raw output
	 */
	options.c_oflag &= ~OPOST;

	/*
	 * Set read timeouts
	 */
	options.c_cc[VMIN] = 8;
	options.c_cc[VTIME] = 10;
	//options.c_cc[VMIN] = 0;
	//options.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &options);

	return 0;
}

int main(int args, char *argc[])
{
	char buf[128];
	int ret;

	serial_init();
	while(1) {
		ret = read(serial_fd, buf, 1);
		buf[ret]=0;
		printf("%s %d", buf, ret);
	}
	return 0;
}
