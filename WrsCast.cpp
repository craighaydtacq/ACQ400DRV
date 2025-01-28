/*
 * WrsCast.cpp
 *
 *  Created on: 10 Jan 2025
 *      Author: pgm
 */

#include "Multicast.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <iostream>

#include "knobs.h"
#include "wrs_trigger_int.h"
#include "WrsTriggerDrv.h"


class WrsCastImpl : public WrsCast {

protected:
	const char *group;
	int port;
	struct sockaddr_in addr;
	int sock;
	socklen_t addrlen;
	int verbose;

public:
    int fd;
    u32 interrup = 0;
    WrsTriggerDrv drv;
	WrsCastImpl(const char* _group, int _port):
		group(_group), port(_port), verbose(0)
	{
        if (getenv("WrsCastVerbose")) {
            verbose = atoi(getenv("WrsCastVerbose"));
            drv.verbose = verbose;
        }

        if (verbose > 1) printf("WrsCastImpl()\n");

        // outputs ST on every TS for interrupt latency eval
      
    }

	virtual int sendto(const void* message, int len) {
		if (verbose > 1) printf("WrsCastImpl::sendto()\n");
        return -1;
	}
	virtual int recvfrom(void* message, int len) {
        if (verbose > 1) printf("WrsCastImpl::recvfrom()\n");
		return -1;
	}
};

class WrsCastSender : public WrsCastImpl {

public:
	WrsCastSender(const char* _group, int _port):
		WrsCastImpl(_group, _port)
	{}
	virtual int sendto(const void* message, int len) {
        if (verbose > 1) printf("WrsCastSender()::sendto 99\n");
	int rc = drv.transmit();
	// decrement drv.tx_count
        return 0;
	}
};


class WrsCastReceiver : public WrsCastImpl {
public:
	WrsCastReceiver(const char* _group, int _port):
		WrsCastImpl(_group, _port)
	{
        if (verbose > 1) printf("WrsCastReceiver() 99\n");
	}

	virtual int recvfrom(void* message, int len) {
        if (verbose > 1) printf("WrsCastReceiver::recvfrom() 01\n");
        int rc = -1;
        // drv.check_interrupt();
        // immediately set soft trigger to zero
        
        // drv.pulse_soft_trigger();
        
        // RX side: dump memory 
        // std::cout << "message: " << std::hex << message << std::endl;
        // std::cout << "message address: " << std::hex << &message << std::endl;
        u32 *message_u32 = (u32*)message;
        rc = drv.receive(message_u32);
        drv.dump_pkt(drv.rx_pkt, "RX"); 
        drv.dump_ts("TS");
        //printf("TS:%08x", 32[]); //drv.read_data[0]); 
        printf("\n");
        // TODO: decrement drv.rx_count
        //rc = drv.dump_rx(message);
        if (verbose > 1) printf("got a message\n Also got rc = %d", rc);
        
        // TODO: also got drv.dump_rx();
        // if (verbose > 1) drv.dump_rx();
        

        // std::cout << "Interrupt detected! interrup = " << interrup << std::endl;

        /*
        while (true) {
            //drv.check_interrupt();
            // immediately set soft trigger to zero
            drv.pulse_soft_trigger();
            // RX side: dump memory 
            std::cout << "message: " << std::hex << message << std::endl;
            std::cout << "message address: " << std::hex << &message << std::endl;
            drv.hello();
            rc = drv.receive();
	        drv.dump_pkt(drv.rx_pkt, "RX"); 
            printf("TS:%08x", drv.read_data[0]); 
            printf("\n");
	    // TODO: decrement drv.rx_count
	    //rc = drv.dump_rx(message);
            if (verbose > 1) printf("got a message\n Also got rc = %d", rc);
            if (verbose > 1) drv.dump_rx();
            // std::cout << "Interrupt detected! interrup = " << interrup << std::endl;
        }
        */
        if (rc < 0) {
            perror("recvfrom");
            exit(1);
        
        
        if (rc == 1) {
            perror("recvfrom returns 1");
            exit(1);
        }

        if (verbose > 1) printf("WrsCastReceiver()::recvfrom 99, rc = %d\n", rc);
		return rc;
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
