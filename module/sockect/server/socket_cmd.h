#ifndef _SOCKETCMDME_H_
#define _SOCKETCMDME_H_

#define SOCKET_STATE_CMD (1)
//execute one command
#define TOP_STATE_EXEC (1<2)
//need to respone
#define TOP_STATE_EXEC_RES (1<3)

enum {
	SOCKET_CMD_SHELL =1,
	SOCKET_CMD_HEART ,
	SOCKET_CMD_REQ ,
	SOCKET_CMD_CONNECT, // 初始化第一个命令
};

enum REQ_RESP {
	SOCKET_RESP_OK =1,
};

struct connect_soc_data {
	int socket_id;
	int heart_lost;
	int id;
};

/* 初始化连接时发送命令*/
struct sockettop_cmd {
	int cmd_type;
	int state;
	unsigned long int id;
	int size;
};

struct socketshell_cmd {
	char cmd[128];
	int resovled;
};

#define REQ_CODE_SUC  (0x100)
#define REQ_CODE_FAIL (0x101)
struct socketres_respone {
	int req_code;
	int size;
};

#define DBG(fmt,...) do {fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n");} while(0)
#define ERR(fmt,...) do {fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n");} while(0)

#define CON_OUT(fmt,...) do {fprintf(stdout, fmt, ##__VA_ARGS__); fprintf(stderr, "\n");} while(0)
#endif
