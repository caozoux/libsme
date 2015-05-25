#include <stdio.h>	/* Standard input/output definitions */
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
#include <pthread.h>


#include "serialme.h"
//#define DEVNAME "/dev/ttyUSB0"
#define DEVNAME "/dev/ttyS0"

int serial_fd;
int pipe_fd;

/* init the tty ttyS0, 115200, 8,1*/
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

int tty_context_handle(char *buf) {
	int len = strlen(buf);
	int i = 0;
	if (len == 0)
		return 0;

	for (i =0;i <len; i++) {
		if (buf[i] == '\n') {
			printf("find change line\n");
			if (strstr("Hit any key to stop autoboot:", buf)) {
				printf("find :Hit any key to stop autoboot:\n");
				return 1;
			}
		}
	}
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

//thread pipe
void pipe_thread_handle(void* data)
{
  	char buf[100];/*存储数据*/ 
		printf("pipe ready\n");
	while(1) {
		int size;
		size = read(pipe_fd,buf,100);
	  	memset(buf, 0, sizeof(buf));/*清空buf数组*/ 
		write(serial_fd, buf, size);
		sleep(1);
	}
		printf("pipe over\n");
}
void printf_thread_handle(void* data);
/*get the stdin input string, write into tty*/
int thread_init(void)
{
	int status;
	unsigned long int id;
	printf("create thread\n");
	status = pthread_create(&id, NULL, (void*)input_thread_handle , NULL);
	if (status)
		return status;

	status = pthread_create(&id, NULL, (void*)pipe_thread_handle , NULL);
	if (status)
		return status;

	status = pthread_create(&id, NULL, (void*)printf_thread_handle , NULL);
	if (status)
		return status;
	return status;
}

/* serial input pipe create*/
int serial_input_pipe_init(void)
{
	printf("pipe %s create+\n", FIFO);
	if(access(FIFO, F_OK) == -1) {
		if(mkfifo(FIFO,0777)<0) /*创建管道*/ 
		{
			perror("Create error!\n");
			unlink(FIFO);/*清除管道*/
			return 1;
		}
	}

  	pipe_fd = open(FIFO,O_RDONLY | O_NONBLOCK);/*打开管道*/

	if(pipe_fd<0)
	{
		perror("open error!\n");
		unlink(FIFO);
		exit(0);
	}

	printf("pipe %s create-\n", FIFO);

	return 0;
}

void charbuff_dump(char *buf, int len)
{
	int i;
	char read_buffer[256] = { 0 };
	for(i=0;i <len; i++) {
		printf("%02x ",buf[i]);
	}
	printf("\n");
	memcpy(read_buffer, buf, len);
	printf("%s",read_buffer);
	printf("\n");
}

#define READ_BUFFER_SIZE 128
static char read_buffer[READ_BUFFER_SIZE+128] = { 0 };
static int read_start = 0,read_end = 0;
static pthread_rwlock_t rwlock;
//printf handle
void printf_thread_handle(void* data)
{
#if 0
	while(1) {
		int i = 0;
		char line_buffer[READ_BUFFER_SIZE] = {0};
		//printf("printf_thread_handle +\n");
		pthread_rwlock_rdlock(&rwlock);
		if (read_end > read_start) {
			memcpy(line_buffer, read_buffer+read_start, read_end-read_start);
			read_start = read_end;
		}
		if (read_end >= READ_BUFFER_SIZE) {
			read_end = read_start = 0;
		}
		pthread_rwlock_unlock(&rwlock);
		printf("%s", line_buffer);
		usleep(1000);
		//printf("printf_thread_handle -\n");
	}
#else
	char line_buffer[READ_BUFFER_SIZE];
	int line_s = 0;
	int lines=0, old_lines=0;

	int debug = 0;
	
	memset(line_buffer, 0x20, READ_BUFFER_SIZE);
	while(1) {
		int i = 0;
		int size=0;
		int pr_end = 0;
		char pr_buffer[READ_BUFFER_SIZE] = {0};

		usleep(1000);

		pthread_rwlock_rdlock(&rwlock);
		size = read_end - read_start;
		if (size <= 0) {
			pthread_rwlock_unlock(&rwlock);
			continue;
		}

		memcpy(pr_buffer, read_buffer+read_start, read_end-read_start);
		read_start = read_end;

		if (read_end >= READ_BUFFER_SIZE) {
			read_end = read_start = 0;
		}

		pthread_rwlock_unlock(&rwlock);
		pr_end = 0;
		for (i=0 ;i<size;i++) {
			if (pr_buffer[i] == 0) {
				pr_buffer[i] = 0x20;
			}
		}

		for (i=0 ;i<size;i++) {
			if (pr_buffer[i] == '\n') {
				debug++;
				if (debug == 3)
					debug =3;
				lines++;
				memcpy(line_buffer+line_s, pr_buffer+pr_end, i - pr_end + 1);
				line_buffer[line_s+i-pr_end+1] = 0;
				printf("%s", line_buffer);
				line_s = 0;
				pr_end = i+1;

				
			}
		}

		if (lines == old_lines) {
			memcpy(line_buffer+line_s, pr_buffer, size);
			line_s += size;
		} else {
			memcpy(line_buffer+line_s, pr_buffer+pr_end, size-pr_end);
			line_s = size-pr_end;
		}

		old_lines = lines;
	}
#endif

}
int main(int argc, char * args[])
{
	if (serial_input_pipe_init()) {
		return 1;
	}

	serial_init();
	if (thread_init()) {
		printf("err: init input \n");
		close(serial_fd);
		return 1;
	}
	pthread_rwlock_init(&rwlock, NULL);

	while(1) {
		int size;
		size = read(serial_fd, read_buffer+read_end, READ_BUFFER_SIZE);
		pthread_rwlock_wrlock(&rwlock);
		if (size) {
			read_end +=size;
			//printf("read_start:%d read_end:%d\n", read_start, read_end);
		}
		pthread_rwlock_unlock(&rwlock);
	}
	return 0;
}
