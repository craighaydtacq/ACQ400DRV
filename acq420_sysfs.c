/* ------------------------------------------------------------------------- */
/* rtm-t-sysfs.c RTM-T PCIe Host Side driver, sysfs (knobs)	             */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2010 Peter Milne, D-TACQ Solutions Ltd
 *                      <Peter dot Milne at D hyphen TACQ dot com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of Version 2 of the GNU General Public License
    as published by the Free Software Foundation;

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */


/** @file rtm-t-sysfs.c D-TACQ PCIe RTM_T driver, sysfs (knobs) */



#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/timex.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>  /* VERIFY_READ|WRITE */

#include "lk-shim.h"


#include <linux/device.h>
#include <linux/module.h>
#include <linux/user.h>

#include "acq420FMC.h"

#define DEVICE_CREATE_FILE(dev, attr) 							\
	do {										\
		if (device_create_file(dev, attr)){ 					\
			dev_warn(dev, "%s:%d device_create_file", __FILE__, __LINE__); 	\
		} 									\
	} while (0)


static ssize_t show_clkdiv(
	struct device * dev,
	struct device_attribute *attr,
	char * buf)
{
	u32 clkdiv = acq420rd32(acq420_devices[dev->id], ALG_CLKDIV);
	return sprintf(buf, "%u\n", clkdiv);
}

static ssize_t store_clkdiv(
	struct device * dev,
	struct device_attribute *attr,
	const char * buf,
	size_t count)
{
	u32 clkdiv;
	if (sscanf(buf, "%u", &clkdiv) == 1 &&
	    clkdiv > 1 &&
	    clkdiv < 100000){
		acq420wr32(acq420_devices[dev->id], ALG_CLKDIV, clkdiv);
		return count;
	}else{
		return -1;
	}
}

static DEVICE_ATTR(clkdiv, S_IRUGO|S_IWUGO, show_clkdiv, store_clkdiv);

unsigned get_gain(u32 gains, int chan)
{
	return (gains >> chan*2) & 0x3;
}

unsigned set_gain(u32 gains, int chan, u32 value)
{
	u32 mask = 0x3 << chan*2;
	gains &= ~mask;
	gains |= (value&0x3) << chan*2;
	return gains;
}
static ssize_t show_gains(
	struct device * dev,
	struct device_attribute *attr,
	char * buf)
{
	u32 gains = acq420rd32(acq420_devices[dev->id], ALG_GAIN);
	return sprintf(buf, "%u%u%u%u\n",
			get_gain(gains, 0), get_gain(gains, 1),
			get_gain(gains, 2), get_gain(gains, 3));
}

static ssize_t store_gains(
	struct device * dev,
	struct device_attribute *attr,
	const char * buf,
	size_t count)
{
	char gx[4];

	if (sscanf(buf, "%c%c%c%c", gx+0, gx+1, gx+2, gx+3) == 4){
		u32 gains = acq420rd32(acq420_devices[dev->id], ALG_GAIN);
		int ic;
		for (ic = 0; ic < 4; ++ic){
			if (gx[ic] >= '0' && gx[ic] <= '3'){
				gains = set_gain(gains, ic, gx[ic]-'0');
			}else{
				switch(gx[ic]) {
				case '-':
				case 'x':
				case 'X':
					break;
				default:
					dev_warn(dev, "bad value at %c at %d", gx[ic], ic);
				}
			}
		}
		dev_dbg(dev, "set gains: %02x", gains);
		acq420wr32(acq420_devices[dev->id], ALG_GAIN, gains);
		return count;
	}else{
		dev_warn(dev, "rejecting input args != 4");
		return -1;
	}
}

static DEVICE_ATTR(gains, S_IRUGO|S_IWUGO, show_gains, store_gains);


static ssize_t show_gain(
	int chan,
	struct device * dev,
	struct device_attribute *attr,
	char * buf)
{
	u32 gains = acq420rd32(acq420_devices[dev->id], ALG_GAIN);

	return sprintf(buf, "%u\n", get_gain(gains, chan));
}

static ssize_t store_gain(
		int chan,
		struct device * dev,
		struct device_attribute *attr,
		const char * buf,
		size_t count)
{
	char gx;

	if (sscanf(buf, "%c", &gx) == 1){
		u32 gains = acq420rd32(acq420_devices[dev->id], ALG_GAIN);
		if (gx >= '0' && gx <= '3'){
			gains = set_gain(gains, chan, gx-'0');
		}else{
			switch(gx) {
			case '-':
			case 'x':
			case 'X':
				break;
			default:
				dev_warn(dev, "bad value at %c at %d", gx, chan);
			}
		}

		dev_dbg(dev, "set gain: %02x", gains);
		acq420wr32(acq420_devices[dev->id], ALG_GAIN, gains);
		return count;
	}else{
		dev_warn(dev, "rejecting input args != 4");
		return -1;
	}
}

#define MAKE_GAIN(CH)							\
static ssize_t show_gain##CH(						\
	struct device * dev,						\
	struct device_attribute *attr,					\
	char * buf)							\
{									\
	return show_gain(CH - 1, dev, attr, buf);			\
}									\
									\
static ssize_t store_gain##CH(						\
	struct device * dev,						\
	struct device_attribute *attr,					\
	const char * buf,						\
	size_t count)							\
{									\
	return store_gain(CH - 1, dev, attr, buf, count);		\
}									\
static DEVICE_ATTR(gain##CH, S_IRUGO|S_IWUGO, show_gain##CH, store_gain##CH)

MAKE_GAIN(1);
MAKE_GAIN(2);
MAKE_GAIN(3);
MAKE_GAIN(4);


static ssize_t show_simulate(
	struct device * dev,
	struct device_attribute *attr,
	char * buf)
{
	u32 simulate = acq420_devices[dev->id]->ramp_en != 0;
	return sprintf(buf, "%u\n", simulate);
}

static ssize_t store_simulate(
	struct device * dev,
	struct device_attribute *attr,
	const char * buf,
	size_t count)
{
	u32 simulate;
	if (sscanf(buf, "%u", &simulate) == 1){
		acq420_devices[dev->id]->ramp_en = simulate != 0;
		return count;
	}else{
		return -1;
	}
}

static DEVICE_ATTR(simulate, S_IRUGO|S_IWUGO, show_simulate, store_simulate);

static ssize_t show_stats(
	struct device * dev,
	struct device_attribute *attr,
	char * buf)
{
	struct STATS *stats = &acq420_devices[dev->id]->stats;
	return sprintf(buf, "fifo_ints=%u dma_transactions=%u\n",
			stats->fifo_interrupts, stats->dma_transactions);
}

static ssize_t store_stats(
	struct device * dev,
	struct device_attribute *attr,
	const char * buf,
	size_t count)
{
	struct STATS *stats = &acq420_devices[dev->id]->stats;
	u32 clr;

	if (sscanf(buf, "%u", &clr) == 1){
		if (clr){
			memset(stats, 0, sizeof(struct STATS));
		}
		return count;
	}else{
		return -1;
	}
}

static DEVICE_ATTR(stats, S_IRUGO|S_IWUGO, show_stats, store_stats);


void acq420_createSysfs(struct device *dev)
{
	DEVICE_CREATE_FILE(dev, &dev_attr_clkdiv);
	DEVICE_CREATE_FILE(dev, &dev_attr_gains);
	DEVICE_CREATE_FILE(dev, &dev_attr_simulate);
	DEVICE_CREATE_FILE(dev, &dev_attr_stats);
	DEVICE_CREATE_FILE(dev, &dev_attr_gain1);
	DEVICE_CREATE_FILE(dev, &dev_attr_gain2);
	DEVICE_CREATE_FILE(dev, &dev_attr_gain3);
	DEVICE_CREATE_FILE(dev, &dev_attr_gain4);
}

void acq420_delSysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_clkdiv);
	device_remove_file(dev, &dev_attr_gains);
	device_remove_file(dev, &dev_attr_simulate);
	device_remove_file(dev, &dev_attr_stats);
	device_remove_file(dev, &dev_attr_gain1);
	device_remove_file(dev, &dev_attr_gain2);
	device_remove_file(dev, &dev_attr_gain3);
	device_remove_file(dev, &dev_attr_gain4);
}


