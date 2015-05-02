#ifndef _IN_MSG_H_
#define _IN_MSG_H_

struct insock_data
{
	struct insock_data *next;
	//the socket send data
    int time;
	//key code
    unsigned short key;
	//some usefil flag
    unsigned short flag;
};

#endif
