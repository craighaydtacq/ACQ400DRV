/*
 * wrs_trigger.cpp: driver for WR streamers
 *
 *  Created on: 8 Jan 2025
 *      Author: cph 
 *
 *  Open and hold a mapping on SYSDEV (one page)
 *  Create structure pointers to TX, RX at WRS_PKT_BASE_TX,WRS_PKT_BASE_RX
 *  Block on RXDEV, read one u32.
 *  Time critical:Immediately, set soft_trigger to zero,
 *  Then print the TS from RXDEV and then
 *  Dump RX on RX dev receipt
 *  Toggle an output. Compare SOFT TRIGGER to AUX to find max delay in app layer.
 */

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "wrs_trigger_int.h"
#include "knobs.h"

#define PAGESIZE 4096

typedef unsigned u32;

// TODO: move this definition into header file
class WRS_TriggerDrv {
public:
	WRS_TriggerDrv();
    ~WRS_TriggerDrv();

    void dump_rx();
    void set_wr_ts_drives_soft_trigger();
    void pulse_soft_trigger();

private:
    int sysdev_fd;
    u32 *mapped_base; // Base address of mapped memory
	u32 *tx_mem;
	u32 *rx_mem;
	FILE *rx_feed;
};


WRS_TriggerDrv::WRS_TriggerDrv()
{
    sysdev_fd = open(SYSDEV, O_RDWR | O_SYNC);
    if (sysdev_fd < 0) {
        perror("Failed to open SYSDEV");
        exit(EXIT_FAILURE);
    }

    // map one page of memory
    mapped_base = (u32*)mmap(nullptr, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, sysdev_fd, 0);
    if (mapped_base == MAP_FAILED) {
        perror("Failed to mmap SYSDEV");
        close(sysdev_fd);
        exit(EXIT_FAILURE);
    }

    rx_mem = mapped_base + (WRS_PKT_BASE_RX / sizeof(u32));
    tx_mem = mapped_base + (WRS_PKT_BASE_TX / sizeof(u32));

    std::cout << "Driver initialized. RX at " << rx_mem << ", TX at " << tx_mem << std::endl;
}

WRS_TriggerDrv::~WRS_TriggerDrv() {
    // Unmap memory and close SYSDEV
    if (mapped_base != MAP_FAILED) {
        munmap(mapped_base, PAGESIZE);
    }
    close(sysdev_fd);
}

void WRS_TriggerDrv::dump_rx() {
    // Dump RX memory content
    std::cout << "RX: " << std::hex << *rx_mem << std::endl;
}

void WRS_TriggerDrv::set_wr_ts_drives_soft_trigger() {
    // soft trigger
    // write a zero to a file at the place
    setKnob(0, "/sys/module/acq420fmc/parameters/wr_ts_drives_soft_trigger", 1);
    //std::cout << "reset trigger" << std::endl;
}

void WRS_TriggerDrv::pulse_soft_trigger() {
    // write to the place to pulse soft trigger
    // write to "/dev/acq400.0.knobs/soft_trigger"
    setKnob(0, "/dev/acq400.0.knobs/soft_trigger", 1);
    //setKnob(0, "/dev/acq400.0.knobs/soft_trigger", 0);
}


int main(int argc, const char* argv[])
{
    WRS_TriggerDrv drv;
    
    // outputs ST on every TS for interrupt latency eval
    drv.set_wr_ts_drives_soft_trigger();
    
    u32 interrup = 0;
    int fd = open("/dev/acq400.0.wr_pkt_rx", O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open device" << std::endl;
        return 1;
    }

    std::cout << "Waiting for interrupt..." << std::endl;

    while (true) {
        read(fd, &interrup, sizeof(interrup));
        // immediately set soft trigger to zero
        drv.pulse_soft_trigger();
        // RX side: dump memory 
        drv.dump_rx();
        // std::cout << "Interrupt detected! interrup = " << interrup << std::endl;
    }
    close(fd);
    return 0;
}
