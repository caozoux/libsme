#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <sys/signal.h>
#include "socket_cmd.h"
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/

#define HELLO_WORLD_SERVER_PORT    6666
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
 
/* 心跳检查 */
void *heart_connect(void *data)
{
	struct connect_soc_data *soc_data = (struct connect_soc_data*)data;
	while(1) {
		sleep(5000);
		soc_data->heart_lost++;
		if(soc_data->heart_lost == 3) {
			printf("heart timeout, kill work thread\n");
			pthread_kill(soc_data->id, SIGQUIT);
		}
	}
}

/*
 * return:
 * 	   <0 failed
 * 	   0, successfully,but exit
 * 	   >0 
 * */
int handle_connect_cmd(int socket_id, void *data)
{
	int ret;
	struct sockettop_cmd *top_cmd;
	char std_out[1024], *p_out;
	struct socketres_respone  *respo;
	top_cmd = (struct sockettop_cmd *) data;	

	respo = (struct socketres_respone  *)std_out;
	p_out = respo+sizeof(struct socketres_respone);
	DBG("cmd:%08x state:%08x", top_cmd->cmd_type, top_cmd->state);

	if (top_cmd->cmd_type & SOCKET_CMD_CONNECT) {
			if (top_cmd->state == TOP_STATE_EXEC) {
				char *exec_name;
				FILE * fp;
				int size;
				exec_name = (char *) (data + sizeof(struct sockettop_cmd));
				/* 是否需要回信命令的输出内容 */
				if (top_cmd->cmd_type & TOP_STATE_EXEC_RES) {
					fp=popen(exec_name, "r");

					//fgets(std_out,sizeof(std_out),fp);
					size = fread(p_out, sizeof(std_out), 1, fp);

					DBG("%d %s",size, std_out);
					pclose(fp);
					respo->req_code = REQ_CODE_SUC;
					respo->size =1024 - sizeof(struct socketres_respone);
					ret = send(socket_id, respo, 1024, 0);
					if (ret < 0) {
						ERR("client not recv data");
						return 1;
					} else {
						DBG("send %d", ret);
						return 1;
					}
				} else {
					
					respo->req_code = REQ_CODE_SUC;
					respo->size = 0;
					if (system(exec_name) == -1)
						respo->req_code = REQ_CODE_FAIL;

					send(socket_id, respo, sizeof(struct socketres_respone), 0);
				}
			}
	}

	return 0;
}

void *connect_thread(void *data)
{
	int socket_id = *(int*)(data);
	struct sockettop_cmd *top_cmd;
	int state = 0, size;
	unsigned char buf[1024];
	int ret;

	//while(1){
	//	sleep(1000);

	size = recv(socket_id,buf,1024,0);
	if (size > 0) {
		ret = handle_connect_cmd(socket_id, (void*)buf);
		if (ret < 0) {
			ERR("connect init falied, exit");
			return;
		} else if  (ret == 0) {
			DBG("cmd successful, exit");
			return;
		}
	} else {
		DBG("error recv");
	}

	return;
	//signal(SIGKILL,sig_handler);
	while (1) {
		size = recv(socket_id,buf,1024,0);
		if (size > 0) {
			const int *p;
			p = (int *) buf;
			if (*p == SOCKET_STATE_CMD) {
				if (size == sizeof(struct sockettop_cmd)) {
					top_cmd = (struct sockettop_cmd *)buf;
				}
			} else {
				top_cmd = NULL;
			}
		}

		if (top_cmd) {
			if (top_cmd->cmd_type == SOCKET_CMD_HEART)
			{
				continue;
			}

			ret = handle_connect_cmd(socket_id, top_cmd);
			if (ret > 0) {
				printf("socket cmd dones't handle\n");
				continue;
			} 
		}
	}
}

int main(int argc, char **argv)
{
    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;
	int status;
	unsigned long int id;
    fprintf(stderr, "Start...\n"); 
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
 
    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        fprintf(stderr, "Create Socket Failed!");
        exit(1);
    }

	{ 
   		int opt =1;
    	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	}
     
    //把socket和socket地址结构联系起来
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        fprintf(stdout, "Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        exit(1);
    }
 
    fprintf(stderr, "Server start listen\n"); 
    //server_socket用于监听
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        fprintf(stderr, "Server Listen Failed!"); 
        exit(1);
    }
    while (1) //服务器端要一直运行
    {
        //定义客户端的socket地址结构client_addr
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
 
        //接受一个到server_socket代表的socket的一个连接
        //如果没有连接请求,就等待到有连接请求--这是accept函数的特性
        //accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
        //new_server_socket代表了服务器和客户端之间的一个通信通道
        //accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
        int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
#if 0         
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            break;
        }
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
        strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
//        int fp = open(file_name, O_RDONLY);
//        if( fp < 0 )
        printf("%s\n",file_name);
        FILE * fp = fopen(file_name,"r");
        if (NULL == fp)
        {
            printf("File:\t%s Not Found\n", file_name);
        }
        else
        {
            bzero(buffer, BUFFER_SIZE);
            int file_block_length = 0;
//            while( (file_block_length = read(fp,buffer,BUFFER_SIZE))>0)
            while( (file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
            {
                printf("file_block_length = %d\n",file_block_length);
                //发送buffer中的字符串到new_server_socket,实际是给客户端
                if(send(new_server_socket,buffer,file_block_length,0)<0)
                {
                    printf("Send File:\t%s Failed\n", file_name);
                    break;
                }
                bzero(buffer, BUFFER_SIZE);
            }
//          close(fp);
            fclose(fp);
            printf("File:\t%s Transfer Finished\n",file_name);
        }
        //关闭与客户端的连接
        close(new_server_socket);
#else
		//fprintf(stderr, "create heart socket thread\n");
		status = pthread_create(&id, NULL, connect_thread, &new_server_socket);
		if (status) {
			fprintf(stderr, "ctreat connect thread failed\n");
			close(new_server_socket);
			return status;
		} else {
			DBG("connect thread");
		}
#endif
    }
    //关闭监听用的socket
    close(server_socket);
    return 0;
}
