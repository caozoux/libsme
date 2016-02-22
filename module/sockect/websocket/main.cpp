/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<pthread.h>
//#include <netinet in.h="">

#include "base64.h"
#include "sha1.h"
#include "intLib.h"
#include "testdata.h"

#define _LDBGD(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGW(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGI(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGF(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)


#define REQUEST_LEN_MAX 1024*4
#define DEFEULT_SERVER_PORT 8000
#define WEB_SOCKET_KEY_LEN_MAX 256
#define RESPONSE_HEADER_LEN_MAX 1024
#define LINE_MAX 256
struct websocket_connect_d {
	int websock_fd;
	int connected;
};
void shakeHand(int connfd,const char *serverKey);
char * fetchSecKey(const char * buf);
char * computeAcceptKey(const char * buf);
char * analyData(const char * buf,const int bufLen);
char * packData(const char * message,unsigned long * len);
void response(const int connfd,const char * message);
void* accepted_func(void* arg);

char * fetchSecKey(const char * buf)
{
  char *key;
  char *keyBegin;
  char *flag="Sec-WebSocket-Key: ";
  int i=0, bufLen=0;

  key=(char *)malloc(WEB_SOCKET_KEY_LEN_MAX);
  memset(key,0, WEB_SOCKET_KEY_LEN_MAX);
  if(!buf)
  {
      return NULL;
  }
 
  keyBegin=strstr((char *)buf,flag);
  if(!keyBegin)
  {
      return NULL;
  }
  keyBegin+=strlen(flag);

  bufLen=strlen(buf);
  for(i=0;i<bufLen;i++)
  {
    if(keyBegin[i]==0x0A||keyBegin[i]==0x0D)
	{
	  break;
	}
    key[i]=keyBegin[i];
  }
  
  return key;
}

char * computeAcceptKey(const char * buf)
{
  char * clientKey;
  char * serverKey; 
  char * sha1DataTemp;
  char * sha1Data;
  short temp;
  int i,n;
  const char * GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
 

  if(!buf)
  {
     return NULL;
  }
  clientKey=(char *)malloc(LINE_MAX);
  memset(clientKey,0,LINE_MAX);
  clientKey=fetchSecKey(buf);
 
  if(!clientKey)
  {
      return NULL;
  }

  strcat(clientKey,GUID);

  sha1DataTemp=sha1_hash(clientKey);
  n=strlen(sha1DataTemp);

  sha1Data=(char *)malloc(n/2+1);
  memset(sha1Data,0,n/2+1);
 
  for(i=0;i<n;i+=2)
  {      
    sha1Data[i/2]=htoi(sha1DataTemp,i,2);    
  } 

  serverKey = base64_encode(sha1Data, strlen(sha1Data)); 

  return serverKey;
}

void shakeHand(int connfd,const char *serverKey)
{
  char responseHeader [RESPONSE_HEADER_LEN_MAX];

  if(!connfd)
  {
    return;
  }

  if(!serverKey)
  {
    return;
  }

  memset(responseHeader,'\0',RESPONSE_HEADER_LEN_MAX);

  sprintf(responseHeader, "HTTP/1.1 101 Switching Protocols\r\n");
  sprintf(responseHeader, "%sUpgrade: websocket\r\n", responseHeader);
  sprintf(responseHeader, "%sConnection: Upgrade\r\n", responseHeader);
  sprintf(responseHeader, "%sSec-WebSocket-Accept: %s\r\n\r\n", responseHeader, serverKey);
 
  printf("Response Header:%s\n",responseHeader);

  write(connfd,responseHeader,strlen(responseHeader));
}

char * analyData(const char * buf,const int bufLen)
{
  char * data;
  char fin, maskFlag,masks[4];
  char * payloadData;
  char temp[8];
  unsigned long n, payloadLen=0;
  unsigned short usLen=0;
  int i=0; 


  if (bufLen < 2) 
  {
     return NULL;
  }

  fin = (buf[0] & 0x80) == 0x80; // 1bit，1表示最后一帧  
  if (!fin)
  {
       return NULL;// 超过一帧暂不处理 
  }

   maskFlag = (buf[1] & 0x80) == 0x80; // 是否包含掩码  
   if (!maskFlag)
   {
       return NULL;// 不包含掩码的暂不处理
   }

   payloadLen = buf[1] & 0x7F; // 数据长度 
   if (payloadLen == 126)
   {      
     memcpy(masks,buf+4, 4);      
     payloadLen =(buf[2]&0xFF) << 8 | (buf[3]&0xFF);  
     payloadData=(char *)malloc(payloadLen);
     memset(payloadData,0,payloadLen);
     memcpy(payloadData,buf+8,payloadLen);
    }
    else if (payloadLen == 127)
    {
     memcpy(masks,buf+10,4);  
     for ( i = 0; i < 8; i++)
     {
         temp[i] = buf[9 - i];
     } 

     memcpy(&n,temp,8);  
     payloadData=(char *)malloc(n); 
     memset(payloadData,0,n); 
     memcpy(payloadData,buf+14,n);//toggle error(core dumped) if data is too long.
     payloadLen=n;    
     }
     else
     {   
      memcpy(masks,buf+2,4);    
      payloadData=(char *)malloc(payloadLen);
      memset(payloadData,0,payloadLen);
      memcpy(payloadData,buf+6,payloadLen); 
     }

     for (i = 0; i < payloadLen; i++)
     {
       payloadData[i] = (char)(payloadData[i] ^ masks[i % 4]);
     }
 
     printf("data(%ld):%s\n",payloadLen,payloadData);
     return payloadData;
}

char *  packData(const char * message,unsigned long * len)
{
         char * data=NULL;
	 unsigned long n;

	 n=strlen(message);
            if (n < 126)
            {
	      data=(char *)malloc(n+2);
	      memset(data,0,n+2);	 
	      data[0] = 0x81;
	      data[1] = n;
	      memcpy(data+2,message,n);
	      *len=n+2;
            }
            else if (n < 0xFFFF)
            {
	      data=(char *)malloc(n+4);
	      memset(data,0,n+4);
	      data[0] = 0x81;
	      data[1] = 126;
	      data[2] = (n>>8 & 0xFF);
	      data[3] = (n & 0xFF);
	      memcpy(data+4,message,n);    
	      *len=n+4;
            }
            else
            {
	 
                // 暂不处理超长内容  
	      *len=0;
            }
  

	 printf("%s\n", message);
	 printf("len %d\n", *len);
        return data;
}

void response(int connfd,const char * message)
{
  char * data;
  unsigned long n=0;
  int i;
  if(!connfd)
  {
      return;
  }

  if(!data)
  {
      return;
  }
  data=packData(message,&n); 
 
  if(!data||n<=0)
  {
      printf("data is empty!\n");
      return;
  } 
 
  i = write(connfd,data,n);
  printf("%s write:%d\n", __func__, i);
}

struct request_type_data {
	int req;
	int size;
	unsigned char data[300];
};
#if 1
unsigned char senior_buf[1028];
unsigned char jump;
unsigned short int sleep_x, sleep_y, sleep_z, sleep_cnt;
unsigned short int *heart, temperate;
int walk;

void handle_data(unsigned char *buf)
{
	int i;
	printf("00: ");
	for (i=1; i < 216;i++) {
		if ((i-1)%10 == 0)
			printf("\n%02d: ", i);
		printf("%02x ",buf[i-1]);
	}
	printf("\n");
	heart = (unsigned short int*) buf;

	sleep_x=*(unsigned short int*)(buf+200);
	sleep_y=*(unsigned short int*)(buf+202);
	sleep_z=*(unsigned short int*)(buf+204);
	temperate = *(unsigned short int*)(buf+206);
	jump = *(buf+208);
	sleep_cnt= *(unsigned short int*)(buf+209); 
	walk= *(int *)(buf+211); 
	printf(" sleep_x/y/z:%04x %04x %04x, temperate:%04x, jump:%02x, sleep_cnt:%02x walk:%02x\n",
			sleep_x, sleep_y, sleep_z, temperate, jump, sleep_cnt, walk);
	
}
struct remote_data{
	int socket_id;
} remote_d;

void* get_data(void* arg)
{
	struct remote_data *remote_p;
	int ret;
	remote_p = (struct remote_data *) arg;
	while (1) {
		ret = recv(remote_p->socket_id, senior_buf, 1024, 0);
		if (ret > 0) {
			printf("get data %d\n", ret);
			handle_data(senior_buf+5);
		}
	}
}

int init_remote_socket_data(char *addr, int port)
{
	struct sockaddr_in server_addr;
	int socket_id, ret;
	struct request_type_data *data;
	pthread_t pthread_id;
	bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);
	socket_id = socket(PF_INET,SOCK_STREAM,0);
	printf("start connect %s port %d \n", addr, port);
	if (socket_id <0 )
		return -1;

	if (connect(socket_id, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		return -1;
	}
	remote_d.socket_id = socket_id;
	pthread_create(&pthread_id, NULL,&get_data,(void *)&remote_d);
#if 0
	while (1) {
		ret = recv(socket_id, senior_buf, 1024, 0);
		if (ret > 0) {
			printf("get data %d\n", ret);
			data = (struct request_type_data*) senior_buf;
			handle_data(senior_buf+5);
		}
	}
#endif

	printf("connect successfully \n");
	return socket_id;
}
#endif
	
unsigned char *handle_buf;
unsigned char heart_test_data[] = {50,60,70,80,90,80,70,60,55,50};
int heart_test_data_off =0;

unsigned char *create_heart_data(void) 
{
	char *buf;
	char pri_buf[1000];
	int i, len = 0;

	buf =(char *) malloc(1000*4);
	memset(buf, 50, 4000);
	srand(time(0));
	for (i = 0; i <4000; i = i+30) {
		buf[i] = heart_test_data[0] + rand()%10+1;
		buf[i+1] = heart_test_data[1] + rand()%10+1;
		buf[i+2] = heart_test_data[2] + rand()%10+1;
		buf[i+3] = heart_test_data[3] + rand()%10+1;
		buf[i+4] = heart_test_data[4] + rand()%10+1;
		buf[i+5] = heart_test_data[5] + rand()%10+1;
		buf[i+6] = heart_test_data[6] + rand()%10+1;
		buf[i+7] = heart_test_data[7] + rand()%10+1;
		buf[i+8] = heart_test_data[8] + rand()%10+1;
		buf[i+9] = heart_test_data[9] + rand()%10+1;
		//memcpy(buf+i, heart_test_data, 10);
	}
	handle_buf = (unsigned char *)buf;
#if 0
	//while (1) {
		for (i=0; i < 100; i++) {
				len += sprintf(pri_buf+len,"%02d",handle_buf[i+heart_test_data_off]);
		}
		heart_test_data_off +=5;
		if (heart_test_data_off >=3900)
			heart_test_data_off = 0;
		for (i=0; i< len; i=i+2) {
			printf("%c%c ", pri_buf[i], pri_buf[i+1]);
		}
		len = 0;
	//	printf("\n");
	//	sleep(1);
	//}
#endif
}
void test_data(void)
{
	char *buf;
	int i;

	buf =(char *) malloc(1000*4);
	memset(buf, 50, 4000);
	for (i = 0; i <4000; i = i+30) {
		memcpy(buf+i, heart_test_data, 10);
	}
	exit(0);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd, data_control_fd;
	char buf[REQUEST_LEN_MAX];
	char *data;
	char str[INET_ADDRSTRLEN];
	char *secWebSocketKey;
	int i,n;
	int connected=0;//0:not connect.1:connected.
	int port= DEFEULT_SERVER_PORT;
	pthread_t pthread_id;
	struct websocket_connect_d * web_d;
//create_heart_data();
#if 0
	if ((data_control_fd = init_remote_socket_data("128.224.163.14", 9001))<0) {
		printf("connect data centr failed\n");
		return 1;
	}
#endif

	if(argc>1)
	{
	    port=atoi(argv[1]);
	}

	if(port<=0||port>0xFFFF)
	{
	    printf("Port(%d) is out of range(1-%d)",port,0xFFFF);
	    return 0;
	}
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
    
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	listen(listenfd, 20);

	while (1) {
		printf("Listen %d Accepting connections ... \n",port);
		cliaddr_len = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
		printf("From %s at PORT %d\n",
				   inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
				   ntohs(cliaddr.sin_port));
		web_d = new websocket_connect_d;
		memset(buf,0,REQUEST_LEN_MAX);

		n = read(connfd, buf, REQUEST_LEN_MAX);	
		printf("---------------------");


		if(0==web_d->connected)
		{
			printf("read:%d%s",n,buf);
			secWebSocketKey=computeAcceptKey(buf);	
			shakeHand(connfd,secWebSocketKey);
			web_d->connected=1;
			web_d->websock_fd = connfd;
			pthread_create(&pthread_id, NULL,&accepted_func,(void *)web_d);
			//pthread_join(pthread_id, NULL);
			//delete web_d;
		}
	}
}

static int test_cnt=1;
//子线程函数
void* accepted_func(void* arg)
{
	int n, connfd, len = 0;
	char buf[REQUEST_LEN_MAX], data_buf[256];
	char *data;
	struct websocket_connect_d *web_d;
	short int *heart_var1, *heart_var2, *heart_var3, heart_imple = 50, i;
	web_d = (struct websocket_connect_d *) arg;
	connfd = web_d->websock_fd;
	_LDBGD("start websocket handle...............\n");
	handle_buf = create_heart_data();
	while (1) {
	//while (n = read(connfd, buf, REQUEST_LEN_MAX)) {
		n = read(connfd, buf, REQUEST_LEN_MAX);
		if (n < 0) {
			printf("clinet connet fialed, exit\n");
			break;
		}
			
		data=analyData(buf,n);
#if 1
		//memcpy(data_buf, handle_buf, 216);
		//memcpy(data_buf, &data_heart[test_cnt], 216);
		len += sprintf(buf,"%04d,%04d,%04d,%04d,%04d \n",(temperate>>8),walk,walk, sleep_cnt,sleep_cnt);
#if 0
		for (i=0; i < 100; i++) {
			heart_var1 = (short int *)data_buf+i*2;
			heart_var2 = (short int *)data_buf+i*2+2;
			heart_var3 = (short int *)data_buf+i*2+4;
			if ((heart_var2 - heart_var1) + (heart_var3 -heart_var2) > 700) {
				len += sprintf(buf +len,"%02d",heart_imple +1);
				heart_imple += 1;
			}
			else if ((heart_var2 - heart_var1) + (heart_var3 -heart_var2) < -700) {
				len += sprintf(buf +len,"%02d",heart_imple -1);
				heart_imple -= 1;
			} else {
				len += sprintf(buf +len,"%02d",heart_imple);
			}
		}
#else
		for (i=0; i < 100; i++) {
				len += sprintf(buf +len,"%02d",handle_buf[i+heart_test_data_off]);
		}
		heart_test_data_off +=5;
		if (heart_test_data_off >=3900)
			heart_test_data_off = 0;
#endif
#else
		//sprintf(buf,"%04d,%04d,%04d,%04d,%04d \n",temperate,walk,walk, sleep_cnt,sleep_cnt);
		len += sprintf(buf,"%04d,%04d,%04d,%04d,%04d \n",12,13,14, 15,16);
		for (i=0; i < 100; i++) {
		   len += sprintf(buf+len,"%02d",i);	
		}
#endif
		test_cnt++;
		response(connfd,buf);
		len = 0;
	}
	close(connfd);
}
