#include <stdio.h>	/* Standard input/output definitions */
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/msg.h> 
#include <sys/ipc.h>


#include "serialme.h"
//#define DEVNAME "/dev/ttyUSB0"
#define DEVNAME "/dev/ttyS0"
//epoll event number
#define NUM_EVENTS  128

//tty fd id
int serial_fd;
//pipe fd id
int pipe_fd;
// hook_mode 是否处理了串口的内容
int hook_mode = 1;
int msqid;


#define READ_BUFFER_SIZE 256 
static char read_buffer[READ_BUFFER_SIZE+128] = { 0 };
static int read_start = 0,read_end = 0;
static pthread_rwlock_t rwlock;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* it is flag value, should be set 0 after inputting boot cmd*/
static int u_boot_cmd = 0;
/*init the tty ttyS0, 115200, 8,1*/
int serial_init(void)
{
	struct termios options;
	int fd;

	/*以非阻塞方式打开串口*/
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

char bootcmd[]="set serverip 128.224.162.212;set ipaddr 128.224.162.103;set netargs setenv bootargs console=ttyO0,115200n8 root=/dev/nfs nfsroot=128.224.162.212:/var/lib/tftpboot/dist,nolock rw ip=dhcp ;tftp 0x80200000  /dist/boot/zImage_wr7dbg;tftp    0x80f80000 /boot_TISDK8/dtb_ti-linux-kernel3.14.y;setenv autoload no;run netargs; bootz 0x80200000 - 0x80f80000\r\n";
void tty_write_thread_handle(void *data)
{
	int ret, msgid;
	struct msgstru msgs;

	msgid = msgget(MSGKEY,IPC_EXCL);/*检查消息队列是否存在 */
	if (msgid < 0) {
		while (1) {
			printf("msg %d no find\n",MSGKEY);
			sleep(1);
		}
	}

	while (1) {
		ret = msgrcv(msgid,&msgs,sizeof(struct msgstru),0,0);
		switch (msgs.msgtype) {
			case MSG_U_BOOT_CMD:
				sleep(1);
				ret = write(serial_fd, bootcmd, strlen(bootcmd));
				if (ret < strlen(bootcmd)) {
					printf("command write error:%d\n", ret);
				}
				sleep(1);
				u_boot_cmd = 0;
				break;
			case MSG_LOGIN:
				{
					char cmd[]="root\n";
					sleep(1);
					ret = write(serial_fd, cmd, strlen(cmd));
					sleep(1);
					ret = write(serial_fd, cmd, strlen(cmd));
					sleep(1);
				}
				break;
			case MSG_REBOOT:
				{
					char cmd[]="reboot\n";
					printf("excule reboot\n");
					ret = write(serial_fd, cmd, strlen(cmd));
					sleep(1);
				}
				break;
			default:
				break;
		}
	}
}

int tty_context_handle(char *buf) {
	int ret;
	int len = strlen(buf);
	int i = 0;
	char ch[2]="\r\n";
	int retry = 5;
	struct msgstru msgs;
	if (len == 0)
		return 0;

	if (hook_mode) {
		if (strstr(buf, "DRAM:")) {
			while(retry--) {
				ret = write(serial_fd,ch,2);
				if (ret <= 0) {
					printf("entry write error\n");
				}
			}
			return 1;
		} else if (strstr(buf, "U-Boot#")) {
			if (u_boot_cmd == 0 ) {
				msgs.msgtype = MSG_U_BOOT_CMD;
				ret = msgsnd(msqid,&msgs,sizeof(struct msgstru),IPC_NOWAIT);  
				if (ret < 0) {
					printf("msg send error\n");
				}
				u_boot_cmd = 1;
			}
			//hook_mode = 0;
		//} else if (strstr(buf, "Wind River Linux 7.0.0.0 128.224.162.191 ttyO0")) {
		} else if (strstr(buf, "Wind River Linux 7.0.0.0")) {
				printf("start root\n");
				msgs.msgtype = MSG_LOGIN;
				ret = msgsnd(msqid,&msgs,sizeof(struct msgstru),IPC_NOWAIT);  
				u_boot_cmd = 0;
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

	status = pthread_create(&id, NULL, (void*)tty_write_thread_handle, NULL);
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

//printf handle
void printf_thread_handle(void* data)
{
	char line_buffer[READ_BUFFER_SIZE];
	int line_s = 0;
	int lines=0, old_lines=0;

	
	memset(line_buffer, 0x20, READ_BUFFER_SIZE);
	while(1) {
		int i = 0;
		int size=0;
		int pr_end = 0;
		char pr_buffer[READ_BUFFER_SIZE] = {0};

		pthread_mutex_lock(&mut);
		pthread_cond_wait(&cond, &mut);
		pthread_mutex_unlock(&mut);

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
		for (i=0 ;i<size;i++) {
			if (pr_buffer[i] == 0) {
				pr_buffer[i] = 0x20;
			}
		}

		for (i=0 ;i<size;i++) {
			if (pr_buffer[i] == '\n') {
				lines++;
				memcpy(line_buffer+line_s, pr_buffer+pr_end, i - pr_end + 1);
				line_buffer[line_s+i-pr_end+1] = 0;
				//printf("%s", line_buffer);
				if (hook_mode)
					tty_context_handle(line_buffer);
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
}


int main(int argc, char * args[])
{
	int  epfd;
	int i;
									    struct epoll_event epevent;
	if (serial_input_pipe_init()) {
		return 1;
	}
	/*init tty control*/
	serial_init();

	if (pthread_mutex_init(&mut, NULL) != 0)
	{
		printf("mutex init error\n");
		return 1;
	}

	if (pthread_cond_init(&cond, NULL) != 0)
	{
		printf("cond init error\n");
		return 1;
	}

	msqid=msgget(MSGKEY,IPC_EXCL);  /*检查消息队列是否存在*/
	if(msqid < 0){
		msqid = msgget(MSGKEY,IPC_CREAT|0666);/*创建消息队列*/
		if(msqid <0){
			printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));
			exit(-1);
		}
	}

	/*create work thread*/
	if (thread_init()) {
		printf("err: init input \n");
		close(serial_fd);
		return 1;
	}
	pthread_rwlock_init(&rwlock, NULL);

    epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        return 7;
    }

    epevent.events = EPOLLIN | EPOLLET;
    epevent.data.ptr = NULL;
	epevent.data.fd = serial_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, serial_fd, &epevent)) {
        perror("epoll_ctl");
        return 8;
    }

    i = 0;
	while (i < NUM_EVENTS) {
        uint64_t finished_aio;
		char test_buf[128] = {0};
		int size;
		char line_pr[128] = {0};

        if (epoll_wait(epfd, &epevent, 1, -1) != 1) {
            perror("epoll_wait");
            return 9;
        }

		pthread_rwlock_wrlock(&rwlock);
		size = read(serial_fd, read_buffer+read_end, READ_BUFFER_SIZE);
		if (size) {
			memcpy(line_pr, read_buffer+read_end, size);
			read_end +=size;
			pthread_mutex_lock(&mut);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mut);
			printf("%s",line_pr);
		}
		//printf("%s",test_buf);
		pthread_rwlock_unlock(&rwlock);
	}
	close(epfd);
    close(serial_fd);
	return 0;
}

