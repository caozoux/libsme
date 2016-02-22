#include "LIBMESocked_base.h"
#include<stdio.h>   /* Standard input/output definitions */
#include<string.h>  /* String function definitions */
#include<unistd.h>  /* UNIX standard function definitions */
#include<fcntl.h>   /* File control definitions */
#include<errno.h>   /* Error number definitions */
#include<termios.h> /* POSIX terminal control definitions */
#include<pthread.h>
#include<sys/eventfd.h>
#include<sys/epoll.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<getopt.h>

int client_sock_id;
struct clinet_t_data {
	int socket_id;
	unsigned long int thread_id;
};
void * connect_work_thread (void *data)
{
	char buf[1024];
	int ret, socket_id;
	struct clinet_t_data  *cl_data;
	cl_data = (struct clinet_t_data *) data;
	//socket_id = data;
	printf("connect working\n");
	while(1) {
		ret = recv(cl_data->socket_id,buf,1,0);
		if (ret > 0)
			printf("%d\n", ret);
	}
	delete cl_data;
}

#include<stdio.h>
int main(int argc, char* agrs[])
{
	struct sockaddr_in *client_addr;
	int socket_id;
	struct clinet_t_data  *cl_data;
	LIBMEServerBS  mSer;
	mSer.CreateServer(9001);
	while(1) {
		socket_id = mSer.GetConnect(client_addr);
		cl_data = new clinet_t_data;
		cl_data->socket_id = socket_id;
		mSer.CreateClientThread(socket_id, connect_work_thread, (void *)cl_data);
	}
	return 0;
}
