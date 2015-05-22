#include <stdio.h>	/* Standard input/output definitions */
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
//#define DEVNAME "/dev/ttyUSB0"
#define DEVNAME "/dev/ttyS0"

int serial_fd;

int serial_init(void)
{
	struct termios options;
	int fd;

	/* 以非阻塞方式打开串口 */
	fd = open(DEVNAME, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf("Open the serial port error!\n");
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

void input_thread_handle(void* data)
{
	printf("input ready\n");
	while(1) {
		char ch;
		ch=getc(stdin);
		write(serial_fd, &ch, 1);
	}
	printf("input over\n");
}

int init_input(void)
{
	int status;
	unsigned long int id;
	printf("create thread\n");
	status = pthread_create(&id, NULL, (void*)input_thread_handle , NULL);
	return status;
}

int main(int argc, char * args[])
{
	serial_init();
	if (init_input()) {
		printf("err: init input \n");
		close(serial_fd);
		return 1;
	}
	while(1) {
		#define READ_BUFFER_SIZE 32
		char read_buffer[READ_BUFFER_SIZE] = { 0 };
		int size;
		size = read(serial_fd, read_buffer, READ_BUFFER_SIZE);
		if (size) {
			printf("%s", read_buffer);
		}
	}
	return 0;
}
