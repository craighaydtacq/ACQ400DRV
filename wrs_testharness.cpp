/*
 * wrs_testharness.cpp
 *
 *  Created on: 7 Jan 2025
 *      Author: pgm
 *
 *  args: block rx tx
 *  params:
 */

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>


#include "Env.h"
#include "acq-util.h"

//#include "wrs_trigger.h"

#define WRS_DEV	"/dev/acq400.0.wr_pkt_rx"    // bad name, receives and transmits

#define PKT_LW	10
#define WRS_PKT_LW		10
#define WRS_PKT_FULL_READ	((1+WRS_PKT_LW)*sizeof(u32)) /* full count in bytes */


typedef unsigned u32;

namespace G {
	int rx_count;
	int tx_count;
	int usleep;
	int rx_block;
	FILE *fp;
	int fd;

	u32 tx_pkt[PKT_LW];
	u32 read_data[PKT_LW+1];
	u32* rx_pkt;

};

const char* ui(int argc, const char** argv)
{
	G::rx_count = Env::getenv("RX", 1);
	G::tx_count = Env::getenv("TX", 0);
	G::usleep   = Env::getenv("US", 0);
	G::rx_block = Env::getenv("RX_BLOCK", 1);

	const char* mode = G::rx_count&&G::tx_count? "r+": G::tx_count? "w": "r";

	assert(G::rx_count||G::tx_count);


	G::fp = fopen(WRS_DEV, mode);
	assert(G::fp);
	G::fd = fileno(G::fp);

	if (G::rx_block == 0){
		int flags = fcntl(G::fd, F_GETFL, 0);
		int rc = fcntl(G::fd, F_SETFL, flags|O_NONBLOCK);
		assert(rc != -1);
	}
	G::rx_pkt = G::read_data+1;    // first word is TS.

	for (int ii = 0; ii < PKT_LW; ++ii){
		G::tx_pkt[ii] = 0xaabb0000|ii;
	}
	return 0;
}

void dump_pkt(u32* pkt, const char* id){
	printf("%4s: ", id);
	for (int ii = 0; ii < PKT_LW; ++ii){
		printf("%08x,", pkt[ii]);
	}
}
void tx() {
	int rc = write(G::fd, G::tx_pkt, sizeof(u32)*PKT_LW);
	assert(rc == sizeof(u32)*PKT_LW);
	dump_pkt(G::tx_pkt, "TX"); printf("\n");
	G::tx_pkt[PKT_LW-1] += 1;
	G::tx_pkt[0] = (G::tx_pkt[0]&~0x00ff00) | ((G::tx_pkt[0]&0x0ff00)+(1<<8));
}

void rx() {
	int rc = read(G::fd, G::read_data, WRS_PKT_FULL_READ);
	assert(rc == WRS_PKT_FULL_READ);
	dump_pkt(G::rx_pkt, "RX"); printf("TS:%08x", G::read_data[0]); printf("\n");
}
int main(int argc, const char* argv[])
{
	ui(argc, argv);
//	WRS_Trigger* trigger = WRS_Trigger::factory(G::site)l

	while (G::tx_count || G::rx_count){
		if (G::tx_count){
			tx();
			--G::tx_count;
			if (G::usleep){
				usleep(G::usleep);
			}
		}
		if (G::rx_count){
			rx();
			--G::rx_count;
		}
	}
	return 0;
}


