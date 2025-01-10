/*
 * WrsCast.cpp
 *
 *  Created on: 10 Jan 2025
 *      Author: pgm
 */


/*
 * Multicast.cpp
 *
 *  Created on: 19 Sep 2019
 *      Author: pgm
 */


#include "Multicast.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>



class WrsCastImpl : public WrsCast {

protected:
	const char *group;
	int port;
	struct sockaddr_in addr;
	int sock;
	socklen_t addrlen;
	int verbose;

public:
	WrsCastImpl(const char* _group, int _port):
		group(_group), port(_port), verbose(0)
	{

	}
	virtual int sendto(const void* message, int len) {
		return -1;
	}
	virtual int recvfrom(void* message, int len) {
		return -1;
	}
};

class WrsCastSender : public WrsCastImpl {

public:
	WrsCastSender(const char* _group, int _port):
		WrsCastImpl(_group, _port)
	{}
	virtual int sendto(const void* message, int len) {
		return 0;
	}
};

class WrsCastReceiver : public WrsCastImpl {

public:
	WrsCastReceiver(const char* _group, int _port):
		WrsCastImpl(_group, _port)
	{

	}

	virtual int recvfrom(void* message, int len) {
		return 0;
	}
};

MultiCast& WrsCast::factory(const char* group, int port, enum MC mode)
{
	switch(mode){
	case MC_SENDER:
		return *new WrsCastSender(group, port);
	case MC_RECEIVER:
	default:
		return *new WrsCastReceiver(group, port);
	}
}


