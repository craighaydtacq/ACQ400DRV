/*
 * wrs_trigger.cpp: driver for WR streamers
 *
 *  Created on: 8 Jan 2025
 *      Author: pgm
 *
 *  Open and hold a mapping on SYSDEV (one page)
 *  Create structure pointers to TX, RX at WRS_PKT_BASE_TX,WRS_PKT_BASE_RX
 *  Block on RXDEV, read one u32.
 *  Time critical:Immediately, set soft_trigger to zero,
 *  Then print the TS from RXDEV and then
 *  Dump RX on RX dev receipt
 *  Toggle an output. Compare SOFT TRIGGER to AUX to find max delay in app layer.
 *
 * mockup:
 *
TX:
init:
acq2106_133> map /usr/local/acq2106.map

per packet .. change value to see ..

acq2106_133> mm $sc+424 c0de1236;md $sc+400


RX.
acq1102_009> cat /dev/acq400.0.wr_pkt_rx  | hexdump -ve '1/4 "%08x\n"'
677e708d

acq1102_009> mmap -f /dev/acq400.0 -l 4096 | hexdump | grep -A 4 0000440
0000440 ba11 ba5e ba11 ba5e ba11 ba5e ba11 ba5e
0000450 1111 1111 2222 2222 3333 3333 4444 4444
0000460 8888 8888 8888 8888 8888 8888 8888 8888
0000470 1236 c0de 0000 0000 0000 0000 0000 0000
0000480 cafe baad cafe baad cafe baad cafe baad

TIMING:
echo 1 > /sys/module/acq420fmc/parameters/wr_ts_drives_soft_trigger

soft trigger will PULSE on PKT rx

Then PULSE it again as soon as the app has the event, time the difference
So hit the trigger:
write 0 to /dev/acq400.0.knobs/soft_trigger

This interface actually makes a pulse for one write.


try knobs.h:
int setKnob(int idev, const char* knob, int value);
.. rather than code from scratch.

 * a second flavour of this app (use command line switch):
 * tx a series of packets at a timed interval.
 * please use popt for option handling, plenty examples in project.
 */

#include "wrs_trigger_int.h"
#include <stdio.h>
#include "popt.h"

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
