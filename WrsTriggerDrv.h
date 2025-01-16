/*
 * wrs_trigger.h : White Rabbit Streamers public interface
 *
 *  Created on: 7 Jan 2025
 *      Author: pgm
 */

#ifndef WRS_TRIGGER_DRV_H_
#define WRS_TRIGGER_DRV_H_

struct wrtd_message;

typedef unsigned u32;


class WrsTriggerDrv {
public:
	WrsTriggerDrv();
    ~WrsTriggerDrv();

    int interrupt_fd;

    int check_interrupt();
    int dump_rx();
    int dump_rx(void*);
    void set_wr_ts_drives_soft_trigger();
    void pulse_soft_trigger();

private:
    int sysdev_fd;
    u32 *mapped_base;
	u32 *tx_mem;
	u32 *rx_mem;
	FILE *rx_feed;
    void copy_integers(uint32_t*, const uint32_t*, size_t);
    void dump_chars(const void*);
    void dump_chars(const void*, int);
    void write_to_file(const uint32_t*, size_t, const char*);
    int sync_mem();
};
#endif /* WRS_TRIGGER_DRV_H_ */
