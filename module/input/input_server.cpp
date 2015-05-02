/* File Name: server.c */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define DEFAULT_PORT 8000
#define MAXLINE 4096
#define INSOCK_DATA_LIST_MAX 100;

struct insock_data *insock_data_list;
struct insock_data * create_insock_data()
{
    insock_data *data = (insock_data*) malloc(sizeof(insock_data));
    if(data == 0) 
		return 0;
	memset((void *)data, 0, sizeof(insock_data));
	return data;
}

void insock_data_free(struct insock_data *data)
{
	if(data)
		free(data);
}

bool insock_data_init(void)
{
    insock_data *data = (insock_data*) malloc(sizeof(insock_data));
    if(data == 0) 
		return 0;

	memset((void *)data, 0, sizeof(insock_data));
	insock_data_list = data;
	insock_data_list->next = insock_data_list;
	for(i=0; i<INSOCK_DATA_LIST_MAX; i++) {
		data = create_insock_data();	
		if(!data) {
			E("mem error\n");
			goto OUT;
		}
		data->next = insock_data_list->next;
		insock_data_list->next = data;
	}
	return true;
OUT:
	while (insock_data_list->next != insock_data_list) {
		data = insock_data_list;
		insock_data_list = insock_data_list->next;
		free(data);
	}
	return false;
};

struct insock_data *get_insock_data(void)
{
	if (insock_data_list->next != insock_data_list) {

	}

}

void insock_data_exit(void)
{

    insock_data *data;
	int number = 0;
	while (insock_data_list->next != insock_data_list) {
		data = insock_data_list;
		insock_data_list = insock_data_list->next;
		free(data);
		number++;
	}

	if (number != INSOCK_DATA_LIST_MAX -1)
		E("free error\n");
	kfree(insock_data_list);
}

int server_create(void)
{
    int    socket_fd, connect_fd;
    struct sockaddr_in     servaddr;
    char    buff[4096];
    int     n;
    //初始化Socket
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }
    //初始化
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
    servaddr.sin_port = htons(DEFAULT_PORT);//设置的端口为DEFAULT_PORT

    //将本地地址绑定到所创建的套接字上
    if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }
    //开始监听是否有客户端连接
    if( listen(socket_fd, 10) == -1){
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }
    printf("======waiting for client's request======\n");
    while(1){
		//阻塞直到有客户端连接，不然多浪费CPU资源。
			if( (connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1){
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
			continue;
		}
		//接受客户端传过来的数据
		n = recv(connect_fd, buff, MAXLINE, 0);
		//向客户端发送回应数据
		if(!fork()){ /*紫禁城*/
			if(send(connect_fd, "Hello,you are connected!\n", 26,0) == -1)
			perror("send error");
			close(connect_fd);
			exit(0);
		}
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connect_fd);
    }
    close(socket_fd);
}

