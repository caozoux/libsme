#ifndef _SERIALME_H_
#define _SERIALME_H_

#define FIFO "/tmp/serialfifo" /*使用宏定义路径*/ 

// mesg id
#define MSGKEY 1024

enum MSG_DEF {
	MSG_U_BOOT_CMD =1,
	MSG_LOGIN,
	MSG_NORMAL,
	MSG_REBOOT,
};

struct msgstru  
{  
	long msgtype;  
	char msgtext[2048];   
};

#endif
