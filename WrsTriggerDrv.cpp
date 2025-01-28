/*
 * WrsTriggerDrv.cpp: driver for WR streamers
 *
 *  Created on: 8 Jan 2025
 *      Author: cph 
 *
 *  Open and hold a mapping on SYSDEV (one page)
 *  Create structure pointers to TX, RX at WRS_PKT_BASE_TX,WRS_PKT_BASE_RX
 *  Block on WRS_DEV, read one u32.
 *  Time critical:Immediately, set soft_trigger to zero,
 *  Then print the TS from WRS_DEV and then
 *  Dump RX on RX dev receipt
 *  Toggle an output. Compare SOFT TRIGGER to AUX to find max delay in app layer.
 */
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "Env.h"
#include "acq-util.h"
#include "wrs_trigger_int.h"
#include "WrsTriggerDrv.h"
#include "knobs.h"



#define PAGESIZE 400
#define RX_OFFSET WRS_PKT_BASE_RX
#define TX_OFFSET WRS_PKT_BASE_TX

WrsTriggerDrv::WrsTriggerDrv()
{


    rx_target_count = rx_count = Env::getenv("RX", 1);
	tx_count = Env::getenv("TX", 0);
	usleep_time   = Env::getenv("US", 0);
	rx_block = Env::getenv("RX_BLOCK", 1);
    rt_prio = Env::getenv("RTPRIO", 0);

	const char* mode = rx_count&&tx_count ? "r+": tx_count ? "w": "r";

	assert(rx_count||tx_count);

    // TODO: not compiling with this, remove or fix
    //signal(SIGINT, get_status);


	fp = fopen(WRS_DEV, mode);
	assert(fp);
	fd = fileno(fp);

	if (rx_block == 0){
		int flags = fcntl(fd, F_GETFL, 0);
		int rc = fcntl(fd, F_SETFL, flags|O_NONBLOCK);
		assert(rc != -1);
	}


	for (int ii = 0; ii < PKT_LW; ++ii){
		tx_pkt[ii] = 0xaabb0000|ii;
	}

    if (rt_prio) {
        goRealTime(rt_prio);
    }
    /*
//    sysdev_fd = open(SYSDEV, O_RDWR | O_SYNC);
//    if (sysdev_fd < 0) {
//        perror("Failed to open SYSDEV");
//        exit(EXIT_FAILURE);
//    }
//    off_t offset = 0;
    
//    std::cout << PAGESIZE << sysdev_fd << std::hex << offset;

    // map one page of memory
//    mapped_base = (u32*)mmap(nullptr, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, sysdev_fd, offset);
//    if (mapped_base == MAP_FAILED) {
//        perror("Failed to mmap SYSDEV");
//        close(sysdev_fd);
//        exit(EXIT_FAILURE);
//    }
    // TODO: update this so that these start at the first byte
    // dump_rx() function should dump the whole packet rather than 1 long word
//    rx_mem = mapped_base + (WRS_PKT_BASE_RX / sizeof(u32));
   // tx_mem = mapped_base + (WRS_PKT_BASE_TX / sizeof(u32));

//    std::cout << "Driver initialized. RX at " << rx_mem << std::endl;
        //", TX at " << tx_mem << std::endl;
*/
}

WrsTriggerDrv::~WrsTriggerDrv() {
    /* Unmap memory and close SYSDEV
    if (mapped_base != MAP_FAILED) {
        munmap(mapped_base, PAGESIZE);
    }
    close(sysdev_fd);
*/
}

void WrsTriggerDrv::get_status(int sig){
	fprintf(stderr, "rx_count to %d left out of %d\n", rx_count, rx_target_count);
}

int WrsTriggerDrv::check_interrupt() {
    unsigned interrup = 0;
    interrupt_fd = open("/dev/acq400.0.wr_pkt_rx", O_RDONLY);
    if (interrupt_fd < 0) {
        std::cerr << "Failed to open file. Possible bad file descriptor" << std::endl;
        return 1;
    }
    read(interrupt_fd, &interrup, sizeof(interrup));
    if (interrup != 0) printf("interrup %zu \n", interrup);
    if (interrupt_fd < 0) {
        std::cerr << "Failed to open device to check interrupt" << std::endl;
        return 1;
    }
    return 0;
}


void WrsTriggerDrv::copy_integers(uint32_t* dest, const uint32_t* src, size_t count) {
    unsigned int microseconds = 10000;
    int sleepflag = 0;
    for (size_t i = 0; i < count; ++i) {
        dest[i] = src[i];
        printf("writing %d", i);
    }
}


void WrsTriggerDrv::dump_chars(const void* start_address) {
    const char* ptr = static_cast<const char*>(start_address);
    int cnt = 1;
    for (int i = 0; i < cnt; ++i) {
        const uint32_t* long_word = reinterpret_cast<const uint32_t*>(ptr);
        printf("Long word %d: 0x%08X\n", i, *long_word);
        ptr += sizeof(uint32_t);
    }
}
void WrsTriggerDrv::dump_chars(const void* start_address, int cnt) {
    const char* ptr = static_cast<const char*>(start_address);

    for (int i = 0; i < cnt; ++i) {
        const uint32_t* long_word = reinterpret_cast<const uint32_t*>(ptr);
        printf("Long word %d: 0x%08X\n", i, *long_word);
        ptr += sizeof(uint32_t);
    }
}

int WrsTriggerDrv::sync_mem() {
    
    constexpr size_t word_count = 16;
    constexpr size_t bytes_per_word = sizeof(u32);
    constexpr size_t total_bytes = word_count * bytes_per_word;
    constexpr size_t total_mapped_size = PAGESIZE;
    int sync_check = msync(mapped_base, total_mapped_size, MS_ASYNC | MS_INVALIDATE);
    
    if (sync_check != 0) {
        std::cerr << "sync failed" << std::endl;
        printf("%d, %d", sync_check, errno);
        exit(0);
    }
    return sync_check;
}

int WrsTriggerDrv::dump_rx() {

    // Dump RX memory content
    constexpr size_t word_count = 16;
    constexpr size_t bytes_per_word = sizeof(u32);
    constexpr size_t total_bytes = word_count * bytes_per_word;

    // Copy 10 long words from rx_mem to dest
    u32* rx_mem_u32 = static_cast<u32*>(rx_mem);

    std::cout << "casted rx_mem addr: " << std::hex << rx_mem_u32 << std::endl;
    
    std::cout << "rx_mem " << std::endl;
    for (size_t i = 0; i < word_count; ++i) {
        std::cout << std::hex << rx_mem_u32[i] << " ";
    }
    std::cout << std::endl;

    return total_bytes;
    // std::cout << "RX: " << std::hex << *rx_mem << std::endl;
}

int WrsTriggerDrv::dump_rx(void* dest) {
    // Dump RX memory content
    constexpr size_t word_count = 16;
    constexpr size_t bytes_per_word = sizeof(u32);
    constexpr size_t total_bytes = word_count * bytes_per_word;


    std::cout << "dest: " << std::hex << dest << std::endl;
    std::cout << "rx_mem: " << std::hex << rx_mem << std::endl;
    std::cout << "mapped_base: " << std::hex << mapped_base << std::endl;
    if (!dest) {
        std::cerr << "Error: destination pointer is null" << std::endl;
        return 0;
    }

    // Copy 10 long words from rx_mem to dest
    u32* dest_u32 = static_cast<u32*>(dest);
    u32* rx_mem_u32 = static_cast<u32*>(rx_mem);
    u32* start_u32 = rx_mem_u32 - 0x440;


    std::cout << "casted dest addr: " << std::hex << dest_u32 << std::endl;
    std::cout << "casted rx_mem addr: " << std::hex << rx_mem_u32 << std::endl;
    



    /*
    for (size_t i = 0; i < word_count; ++i) {
        dest_u32[i] = rx_mem_u32[i];
    }
    
    for (size_t i = 0; i < word_count; ++i) {
        std::cout << std::hex << dest_u32[i] << " ";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < word_count; ++i) {
        std::cout << std::hex << rx_mem_u32[i] << " ";
    }
    std::cout << std::endl;
    */
    copy_integers(dest_u32, rx_mem_u32, 10);
    sync_mem();
    unsigned int microseconds = 10000;
    std::cout << "destination " << std::endl;
    for (size_t i = 0; i < word_count; ++i) {
        std::cout << std::hex << dest_u32[i] << " ";
        usleep(microseconds);
    }
    std::cout << std::endl;

    std::cout << "rx_mem " << std::endl;
    for (size_t i = 0; i < word_count; ++i) {
        std::cout << std::hex << rx_mem_u32[i] << " ";
        usleep(microseconds);
    }
    std::cout << std::endl;

    std::cout << "dumping loads: " << std::endl;
    for (size_t i = 0; i < 110; ++i) {
        std::cout << std::hex << start_u32[i] << " ";
        usleep(microseconds);
    }
    std::cout << std::endl;
   
    // write to file
    write_to_file(rx_mem_u32, 10,"/tmp/my_zero_page");

    return total_bytes;
    // std::cout << "RX: " << std::hex << *rx_mem << std::endl;
}

void WrsTriggerDrv::write_to_file(const uint32_t* data, size_t count, const char* filepath) {
    //TODO: add error handling
    FILE* file = fopen(filepath, "wb");
    size_t written = fwrite(data, sizeof(uint32_t), count, file);
    fclose(file);
}

void WrsTriggerDrv::set_wr_ts_drives_soft_trigger() {
    // soft trigger
    // write a zero to a file at the place
    setKnob(0, "/sys/module/acq420fmc/parameters/wr_ts_drives_soft_trigger", 1);
    //std::cout << "reset trigger" << std::endl;
}

void WrsTriggerDrv::pulse_soft_trigger() {
    // write to the place to pulse soft trigger
    // write to "/dev/acq400.0.knobs/soft_trigger"
    setKnob(0, "/dev/acq400.0.knobs/soft_trigger", 1);
    //setKnob(0, "/dev/acq400.0.knobs/soft_trigger", 0);
}

const char* WrsTriggerDrv::ui(int argc, const char** argv)
{
	
    rx_target_count = rx_count = Env::getenv("RX", 1);
	tx_count = Env::getenv("TX", 0);
	usleep_time   = Env::getenv("US", 0);
	rx_block = Env::getenv("RX_BLOCK", 1);
    rt_prio = Env::getenv("RTPRIO", 0);

	const char* mode = rx_count&&tx_count ? "r+": tx_count ? "w": "r";

	assert(rx_count||tx_count);

    // TODO: not compiling with this, remove or fix
    //signal(SIGINT, get_status);


	fp = fopen(WRS_DEV, mode);
	assert(fp);
	fd = fileno(fp);

	if (rx_block == 0){
		int flags = fcntl(fd, F_GETFL, 0);
		int rc = fcntl(fd, F_SETFL, flags|O_NONBLOCK);
		assert(rc != -1);
	}


	for (int ii = 0; ii < PKT_LW; ++ii){
		tx_pkt[ii] = 0xaabb0000|ii;
	}

    if (rt_prio) {
        goRealTime(rt_prio);
    }
	return 0;
}

void WrsTriggerDrv::dump_pkt(u32* pkt, const char* id){
    printf("%4s: ", id);
	for (int ii = 0; ii < PKT_LW; ++ii){
		printf("%08x,", pkt[ii]);
	}
}

void WrsTriggerDrv::dump_ts(const char* id){
    printf("%4s: ", id);
	printf("%08x,", ts);
	
}

int WrsTriggerDrv::transmit() {
	int rc = write(fd, tx_pkt, sizeof(u32)*PKT_LW);
	assert(rc == PKT_LW*sizeof(u32));
	dump_pkt(tx_pkt, "TX"); printf("\n");
	tx_pkt[PKT_LW-1] += 1;
    tx_pkt[0] = (tx_pkt[0]&~0x00ff00) | ((tx_pkt[0]&0x0ff00)+(1<<8));
    return rc;
}

int WrsTriggerDrv::receive(u32 *buf) {
    if (verbose) printf("Entering WrsTriggerDrv::receive()");
    int rc = -1;
    u32 temp_read_buf[WRS_PKT_FULL_READ];
	int bytes_read = read(fd, temp_read_buf, WRS_PKT_FULL_READ);
    if (bytes_read == WRS_PKT_FULL_READ) {
        u32 timestamp;
        memcpy(buf, &temp_read_buf[1], 10 * sizeof(u32));
        rx_pkt = buf;
        ts = temp_read_buf[0];
        rc = 40;
    }
    else {
        if (verbose) {
            perror("Error reading data");
        }
    }
    if (bytes_read == 44) {
        rc = 40;
    }
	//assert(rc == WRS_PKT_FULL_READ);
    return rc;
}

void WrsTriggerDrv::hello() {
    printf("hello");
}
/*
int main(int argc, const char* argv[])
{
    WrsTriggerDrv drv;
	drv.ui(argc, argv);
//	WRS_Trigger* trigger = WRS_Trigger::factory(site)l

	while (drv.tx_count || drv.rx_count){
		if (drv.tx_count){
			drv.transmit();
            // TODO: move decrement into tx() function or its own function
			--drv.tx_count;
			if (drv.usleep_time){
				usleep(100);
			}
		}
		if (drv.rx_count){
			drv.receive();
            // TODO: move decrement into rx() function or its own function
			--drv.rx_count;
		}
	}
	return 0;
}
*/

/*
int main(int argc, const char* argv[])
{
    WrsTriggerDrv drv;
    
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
*/
