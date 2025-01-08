/*
 * wrs_trigger.cpp: driver for WR streamers
 *
 *  Created on: 8 Jan 2025
 *      Author: pgm
 *
 *  Open and hold a mapping on SYSDEV (one page)
 *  Create structure pointers to TX, RX at WRS_PKT_BASE_TX,WRS_PKT_BASE_RX
 *  Block on RXDEV
 *  Dump RX on RX dev receipt
 *  Toggle an output. Compare SOFT TRIGGER to AUX to find max delay in app layer.
 */

#include "wrs_trigger_int.h"
#include <stdio.h>

typedef unsigned u32;

class WRS_TriggerDrv {
public:
	WRS_TriggerDrv();

	u32 *tx_mem;
	u32 *rx_mem;
	FILE *rx_feed;
};

WRS_TriggerDrv::WRS_TriggerDrv()
{
	// open rx_feed
	// mmap SYSDEV
	// set pointers
}

int main(int argc, const char* argv[])
{
/*
	WRS_TriggerDrv drv;

	while(getline(drv.rx_feed)){
		dump(drv.rx_mem);
		toggle output
	}

	return 0;
*/
}
