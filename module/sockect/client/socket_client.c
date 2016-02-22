#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#include "socketme.h"
#include "socket_cmd.h"

/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/
#define HELLO_WORLD_SERVER_PORT   9001 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
 
int main(int argc, char **argv)
{
    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;
	char *cmd_str;
	
	if (argc != 2) {
		ERR("input cmd!");
		return 0;
	} else {
		cmd_str=argv[1];
	}
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
 
    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    int socket_id = socket(PF_INET,SOCK_STREAM,0);
    if( socket_id < 0)
    {
        ERR("Create Socket Failed!");
        exit(1);
    }
     
	if (connect(socket_id, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		ERR("connect error, exit");
		return;
	}

	struct sockettop_cmd *top_cmd;
	void * data, *p;
	int ret;
	data = malloc(1024);
	memset(data, 0, 1024);
	p = data;
	top_cmd = (struct sockettop_cmd *) data;
	top_cmd->cmd_type = SOCKET_CMD_CONNECT;
	top_cmd->state = TOP_STATE_EXEC;
	p += sizeof(struct sockettop_cmd);
	strcpy(p, cmd_str);
	ret = strlen(cmd_str);
	top_cmd->size = ret;

	//DBG("cmd:%s size :%d ", cmd_str, ret);
    if (send(socket_id, top_cmd ,top_cmd->size + sizeof(struct sockettop_cmd),0) < 0) {
		ERR("send err");
		return;
	}

	ret = recv(socket_id, data, 1024,0);
    if (ret <= 0) {
		ERR("server not respond");
	} else {
		struct socketres_respone  *respo; 
		respo = (struct socketres_respone  *) data;
		if (respo->req_code == REQ_CODE_SUC) {
			DBG("cmd successful");
			return 0;
		} else {
			ERR("cmd failed");
			return 1;
		}
		*(char*)(data+ret) = 0;
		CON_OUT("%s", data);
	}
	close(socket_id);
	free(data);
	return 0;

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
        printf("%s\n",file_name);
        FILE * fp = fopen(file_name,"r");
        if(NULL == fp )
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
//            close(fp);
            fclose(fp);
            printf("File:\t%s Transfer Finished\n",file_name);
        }
#else
#endif
    return 0;
}

