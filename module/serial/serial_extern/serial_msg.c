#include<sys/types.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<fcntl.h> 
#include<sys/stat.h> 
#include<memory.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <getopt.h>
#include "../serial_example/serialme.h"

int main(int argc, char *argv[]) 
{
	char *arg_p, c, ret;
	int option_index, msgid;
	struct msgstru msgs;
	static const char short_options[] = "hnlLD:qt:c:f:r:d:MNF:A:R:T:B:vV:IPCi";
	static const struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"reboot", 0, 0, 'r'},
		{"list-devnames", 0, 0, 'n'},
		{"list-devices", 0, 0, 'l'},
		{},
	};

	if (argc < 2) {
		printf("please input some argument\n");
		return 0;
	}
	msgid = msgget(MSGKEY,IPC_EXCL);/*检查消息队列是否存在 */
	if (msgid < 0) {
			printf("no find msg queue\n");
			return 0;
	}


	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch(c) {
			case 'r':
				msgs.msgtype = MSG_REBOOT;
				ret = msgsnd(msgid,&msgs,sizeof(struct msgstru),IPC_NOWAIT);
				printf("send msg reboot:%d\n",ret);
				break;

			default: 
				break;
		}
	}
	return 0;
   
} 
