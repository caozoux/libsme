#include "LIBMESocked_base.h"

int LIBMEServerBS::CreateServer(int port)
{
    struct sockaddr_in server_addr;
	int status;
	unsigned long int id;
    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if( server_socket < 0)
    {
        fprintf(stderr, "Create Socket Failed!");
        exit(1);
    }

    //把socket和socket地址结构联系起来
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        fprintf(stdout, "Server Bind Port : %d Failed!", port); 
        exit(1);
    }
 
    //server_socket用于监听
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        fprintf(stderr, "Server Listen Failed!"); 
        exit(1);
    }
	mSocket = server_socket;
	mPort = port;
    fprintf(stderr, "Server start listen port:%d\n", mPort); 
}

int LIBMEServerBS::GetConnect(struct sockaddr_in *client_addr)
{
	socklen_t length = sizeof(client_addr);

	//接受一个到server_socket代表的socket的一个连接
	//如果没有连接请求,就等待到有连接请求--这是accept函数的特性
	//accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
	//new_server_socket代表了服务器和客户端之间的一个通信通道
	//accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
	int new_server_socket = accept(mSocket,(struct sockaddr*)client_addr,&length);
	if (new_server_socket) {
		printf("ip %s connect\n",inet_ntoa(client_addr->sin_addr));
		return new_server_socket;
	}
	return 0;
}

unsigned long int  LIBMEServerBS::CreateClientThread(int clinet_socke_id, void *(*start_routine) (void *), void *arg)
{
	unsigned long int  id;
	int ret;
	ret = pthread_create(&id, NULL, start_routine, arg);
	if (ret) {
		return -1;
	}
	return id;
}

LIBMEServerBS::LIBMEServerBS()
{
    fprintf(stderr, "start obj LIBMEServerBS\n"); 
}

LIBMEServerBS::~LIBMEServerBS()
{
	//if (mSocket)
    //	close(mSocket);
}

LIBMEClientBS::LIBMEClientBS()
{

}

LIBMEClientBS::~LIBMEClientBS()
{

}

bool LIBMEClientBS::ConServer(char *addr, int port)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(addr);
    server_addr.sin_port = htons(port);
    mSocket_id = socket(PF_INET,SOCK_STREAM,0);
	if (mSocket_id <0 )
		return false;
	if (connect(mSocket_id, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		printf("connect error, exit");
		return false;
	}
	return true;
}

