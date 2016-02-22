#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <pthread.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LENGTH_OF_LISTEN_QUEUE 20
//LIBMEServerBS -> BS base server
class LIBMEServerBS {
public:
	int mSocket;
	int mPort;
public:
	/*
	 *paramt
	 */
	int CreateServer(int port);
	int GetConnect(struct sockaddr_in *client_addr);
	unsigned long int CreateClientThread(int clinet_socke_id, void *(*start_routine) (void *), void *arg);
public:
	LIBMEServerBS();
	~LIBMEServerBS();
};

class LIBMEClientBS {
	int mSocket_id;
public:
	bool ConServer(char *addr, int port);	
public:
	LIBMEClientBS();
	~LIBMEClientBS();
}
