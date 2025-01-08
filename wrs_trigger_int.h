/*
 * wrs_trigger.h
 *
 *  Created on: 7 Jan 2025
 *      Author: pgm
 */

#ifndef WRS_TRIGGER_INT_H_
#define WRS_TRIGGER_INT_H_

#define WRS_PKT_BASE_TX 	0x400
#define WRS_PKT_BASE_RX		0x440

#define SYSDEV			"/dev/acq400.0"
#define RXDEV			"/dev/acq400.0.wr_pkt_rx"

/*
+++ /mnt/local/rc.user complete
[  168.893723] acq420 40000000.acq2006sc: getWCfromMinor: BAD MINOR 43
[  168.910620] acq420 40000000.acq2006sc: acq400_open_ui FAIL minor:43 rc:-19
[  168.924584] acq420 40000000.acq2006sc: acq400_open FAIL minor:43 rc:-19
[  172.126918] acq420 40000000.acq2006sc: getWCfromMinor: BAD MINOR 43
[  172.143785] acq420 40000000.acq2006sc: acq400_open_ui FAIL minor:43 rc:-19
[  172.152030] acq420 40000000.acq2006sc: acq400_open FAIL minor:43 rc:-19
*/

#endif /* WRS_TRIGGER_INT_H_ */
