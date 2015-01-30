/*
 * acq420FMC.h
 *
 *  Created on: Mar 11, 2013
 *      Author: pgm
 */

#ifndef ACQ420FMC_H_
#define ACQ420FMC_H_

#include <asm/uaccess.h>
#include <asm/sizes.h>

#include <linux/dmaengine.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched/rt.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <linux/circ_buf.h>
#include <linux/debugfs.h>
#include <linux/poll.h>
//#include <mach/pl330.h>
#include <linux/amba/pl330.h>
#include <linux/of.h>

#include <asm/dma.h>
#include <asm/mach/dma.h>
#include <asm/io.h>

/* Offsets for control registers in the AXI MM2S FIFO */
#define AXI_FIFO              0x1000
#define AXI_FIFO_LEN          0x1000

#define OF_IRQ_HITIDE		1	/* index of HITIDE in dtb */
#define OF_IRQ_COUNT		3	/* number of items */
#define OF_IRQ_MAGIC		32	/* add to the OF number to get actual */

#define ADC_BASE		0x0000
#define MOD_ID			(ADC_BASE+0x00)
#define ADC_CTRL		(ADC_BASE+0x04)
#define DAC_CTRL		ADC_CTRL
#define MCR			(ADC_BASE+0x04)
#define TIM_CTRL		(ADC_BASE+0x08)
#define ADC_HITIDE		(ADC_BASE+0x0C)
#define DAC_LOTIDE		ADC_HITIDE
#define ADC_FIFO_SAMPLES	(ADC_BASE+0x10)
#define DAC_FIFO_SAMPLES	ADC_FIFO_SAMPLES
#define ADC_FIFO_STA		(ADC_BASE+0x14)
#define DAC_FIFO_STA		ADC_FIFO_STA
#define ADC_INT_CSR		(ADC_BASE+0x18)
#define DAC_INT_CSR		ADC_INT_CSR
#define ADC_CLK_CTR		(ADC_BASE+0x1C)
#define DAC_CLK_CTR		ADC_CLK_CTR
#define ADC_SAMPLE_CTR		(ADC_BASE+0x20)
#define DAC_SAMPLE_CTR		ADC_SAMPLE_CTR
#define ADC_SAMPLE_CLK_CTR	(ADC_BASE+0x24)
#define SW_EMB_WORD1		(ADC_BASE+0x28)
#define SW_EMB_WORD2		(ADC_BASE+0x2c)
#define EVT_SC_LATCH		(ADC_BASE+0x30)

#define ADC_CLKDIV		(ADC_BASE+0x40)
#define DAC_CLKDIV		ADC_CLKDIV
#define ADC_GAIN		(ADC_BASE+0x44)
#define DAC_424_CGEN		(ADC_BASE+0x44)
/* obsolete R3
#define ADC_FORMAT 		(ADC_BASE+0x48)
*/
#define ADC_CONV_TIME 		(ADC_BASE+0x4C) /*(mask 0x000000FF)*/

#define ACQ435_MODE		(ADC_BASE+0x44)
#define AO420_RANGE		(ADC_BASE+0x44)
#define AO420_DACSPI		(ADC_BASE+0x48)
#define ADC_TRANSLEN		(ADC_BASE+0x50)
#define ADC_ACC_DEC		(ADC_BASE+0x54)


#define SPADN(ix)		(ADC_BASE+0x80+(ix)*sizeof(u32))
#define SPADMAX			8

/* doxy says this
//#define ADC_FIFO_SAMPLE_MASK	((1<<14)-1)
 but observation says this:
 */
#define ADC_FIFO_SAMPLE_MASK	0x3ff

#define FIFO_HISTO_SZ	      	(1<<8)
#define STATUS_TO_HISTO(stat)	(((stat)&ADC_FIFO_SAMPLE_MASK)>>(10-8))


#define MOD_ID_TYPE_SHL		24
#define MOD_ID_IS_SLAVE		(1<<23)
#define MOD_ID_VERSION_SHL	16
#define MOD_ID_REV_SHL		0
#define MOD_ID_REV_MASK		0x0000ffff

#define MOD_ID_ACQ420FMC	1
#define MOD_ID_ACQ420FMC_2000	0xa1
#define MOD_ID_ACQ435ELF	2
#define MOD_ID_ACQ430FMC	3
#define MOD_ID_ACQ480FMC	4
#define MOD_ID_ACQ425ELF	5
#define MOD_ID_ACQ425ELF_2000	0xa5
#define MOD_ID_ACQ437ELF	6
#define MOD_ID_DUMMY		0x00ff

#define MOD_ID_AO420FMC		0x40
#define MOD_ID_AO424ELF		0x41

#define MOD_ID_BOLO8		0x60
#define MOD_ID_DIO432FMC	0x61
#define MOD_ID_DIO432PMOD	0x62
#define MOD_ID_PMODADC1		0x63

#define MOD_ID_ACQ2006SC	0x80
#define MOD_ID_ACQ1001SC	0x81
#define MOD_ID_ACQ2106SC	0x82

#define ADC_CTRL_RGM_GATE_HI    (1 << 15)       /* 0x00008000 */
#define ADC_CTRL_RGM_GATE       (7 << 12)       /* 0x00007000 */
#define ADC_CTRL_RGM_MODE_SHL   (10)
#define ADC_CTRL_RGM_MODE_MASK  0x3
#define ADC_CTRL_435_GATE_SYNC	(1 << 10)	/* special resync mode */

#define ADC_CTRL32B_data	(1 << 7)	/* ACQ420FMC */
#define ADC_CTRL_420_18B	(1 << 6)	/* ACQ420FMC */
#define ADC_CTRL_435_EMBED_STR	(1 << 6)	/* ACQ435 bitslice data */
#define ADC_CTRL_RAMP_EN 	(1 << 5)	/* Deprecated, sadly. Use SPAD */
#define ADC_CTRL_ADC_EN		(1 << 4)

#define DAC_CTRL_LL		(1 << 8)	/* AO420FMC  */

#define ADC_CTRL_ADC_RST	(1 << 3)
#define ADC_CTRL_FIFO_EN	(1 << 2)
#define ADC_CTRL_FIFO_RST	(1 << 1)
#define ADC_CTRL_MODULE_EN	(1 << 0)	/* enable at enumeration, leave up */

#define DAC_CTRL_DAC_EN		ADC_CTRL_ADC_EN
#define DAC_CTRL_DAC_RST	ADC_CTRL_ADC_RST
#define DAC_CTRL_FIFO_EN	ADC_CTRL_FIFO_EN
#define DAC_CTRL_FIFO_RST	ADC_CTRL_FIFO_RST
#define DAC_CTRL_MODULE_EN	ADC_CTRL_MODULE_EN

#define ADC_CTRL_RGM_GATE_SHL	12

#define ADC_CTRL_RST_ALL 	(ADC_CTRL_ADC_RST | ADC_CTRL_FIFO_RST)
#define ADC_CTRL_ENABLE_CAPTURE (ADC_CTRL_ADC_EN | ADC_CTRL_FIFO_EN)
#define ADC_CTRL_ENABLE_ALL	(ADC_CTRL_ADC_EN | ADC_CTRL_FIFO_EN|ADC_CTRL_MODULE_EN)


#define TIM_CTRL_EVENT1_SHL	28
#define TIM_CTRL_EVENT0_SHL	24
#define TIM_CTRL_TRIG_SHL	20
#define TIM_CTRL_SYNC_SHL	16
#define TIM_CTRL_CLK_SHL	12
#define TIM_CTRL_MODE_SHL	0

#define CTRL_SIG_MASK	0xf
#define CTRL_SIG_RISING	0x8
#define CTRL_SIG_SEL	0x7

#define TIM_CTRL_MODE_EV1_EN	(1 << (4+TIM_CTRL_MODE_SHL))
#define TIM_CTRL_MODE_EV0_EN	(1 << (3+TIM_CTRL_MODE_SHL))
#define TIM_CTRL_MODE_HW_TRG	(1 << (2+TIM_CTRL_MODE_SHL))
#define TIM_CTRL_MODE_SYNC	(1 << (1+TIM_CTRL_MODE_SHL))
#define TIM_CTRL_MODE_HW_CLK	(1 << (0+TIM_CTRL_MODE_SHL))



#define ADC_FIFO_STA_CLK	(1<<7)
#define ADC_FIFO_STA_TRG	(1<<6)
#define ADC_FIFO_STA_ACC	(1<<5)
#define ADC_FIFO_STA_ACTIVE	(1<<4)
#define ADC_FIFO_STA_FULL	(1<<3)
#define ADC_FIFO_STA_EMPTY	(1<<2)
#define ADC_FIFO_STA_OVER	(1<<1)
#define ADC_FIFO_STA_UNDER	(1<<0)

#define ADC_FIFO_STA_ERR \
	(ADC_FIFO_STA_UNDER|ADC_FIFO_STA_OVER|ADC_FIFO_STA_FULL)

#define ADC_FIFO_FLAGS 		(ADC_FIFO_STA_ERR|ADC_FIFO_STA_EMPTY)

#define ADC_INT_CSR_COS		(1<<9)
#define ADC_INT_CSR_HITIDE	(1<<8)

#define ADC_INT_CSR_COS_EN	(1<<1)
#define ADC_INT_CSR_HITIDE_EN	(1<<0)


#define ADC_CLK_CTR_SRC_SHL	28
#define ADC_CLK_CTR_SRC_MASK	0xf		/* 1:16 sources */
#define ADC_CLK_CTR_MASK	0x0fffffff	/* 28 bit count */

#define ADC_SAMPLE_CTR_MASK	0x0fffffff

#define ADC_CLK_DIV_MASK	0x0000ffff

#define ADC_CONV_TIME_500	0x96
#define ADC_CONV_TIME_1000	0x36

#define ADC_HT_INT		91
#define HITIDE			2048


#define ACQ435_MODE_HIRES_512	(1<<4)
#define ACQ435_MODE_B3DIS	(1<<3)
#define ACQ435_MODE_B2DIS	(1<<2)
#define ACQ435_MODE_B1DIS	(1<<1)
#define ACQ435_MODE_B0DIS	(1<<0)

#define ACQ435_MODE_BXDIS	0xf
#define ACQ430_BANKSEL	\
	(ACQ435_MODE_B3DIS|ACQ435_MODE_B2DIS|ACQ435_MODE_B1DIS)

#define MODULE_NAME             "acq420"


#define AO420_DACSPI_CW		(1U<<31)
#define AO420_DACSPI_WC		(1U<<30)



#define ADC_ACC_DEC_LEN		0x001f		/* 0:x1, 1..31: x2..32 */
#define ADC_ACC_DEC_SHIFT_MASK	0x1f00

#define ADC_ACC_DEC_SHIFT_5	0x5		/*  /32 */
#define ADC_ACC_DEC_SHIFT_4	0x4		/*  /16 */
#define ADC_ACC_DEC_SHIFT_3	0x3		/*  /8  */
#define ADC_ACC_DEC_SHIFT_2	0x2		/*  /4  */
#define ADC_ACC_DEC_SHIFT_1	0x1		/*  /2  */
#define ADC_ACC_DEC_SHIFT_0	0x0		/*  /1  */

/* AO420FMC */

#define DAC_FIFO_SAMPLES_MASK	0x0000ffff

/*
 *  Minor encoding
 *  0 : the original device
 *  100..164 : buffers
 *  200..231 : channels when available
 */
#define ACQ420_MINOR_0	        0
#define ACQ420_MINOR_CONTINUOUS	1
#define ACQ420_MINOR_HISTO	2
#define ACQ420_MINOR_HB0	3	// block on HB0 fill
#define ACQ420_MINOR_SIDEPORTED 4	// enable, but data flow elsewhere
#define ACQ420_MINOR_GPGMEM	5	// mmap 4K of this
#define ACQ420_MINOR_EVENT	6	// blocks on event
#define ACQ420_MINOR_STREAMDAC	7
#define ACQ420_MINOR_BOLO_AWG	8
#define AO420_MINOR_HB0_AWG_ONCE	9
#define AO420_MINOR_HB0_AWG_LOOP	10
#define ACQ420_MINOR_RESERVE_BLOCKS	11
#define ACQ420_MINOR_SEW1_FIFO	12
#define ACQ420_MINOR_SEW2_FIFO	13
#define AO420_MINOR_HB0_AWG_ONCE_RETRIG	14
#define ACQ420_MINOR_BUF	1000
#define ACQ420_MINOR_BUF2	2000
#define ACQ420_MINOR_MAX	ACQ420_MINOR_BUF2
#define ACQ420_MINOR_CHAN	200
#define ACQ420_MINOR_CHAN2	232	// in reality 203 of course, but looking ahead ..


#define IS_BUFFER(minor) \
	((minor) >= ACQ420_MINOR_BUF && (minor) <= ACQ420_MINOR_BUF2)
#define BUFFER(minor) 		((minor) - ACQ420_MINOR_BUF)

#define AO_CHAN	4
/** acq400_dev one descriptor per device */

void event_isr(unsigned long data);

extern int event_isr_msec;

#define MAXSITES 	6

enum DIO432_MODE { DIO432_DISABLE, DIO432_IMMEDIATE, DIO432_CLOCKED };

#define AO424_MAXCHAN		32



inline static const char* dio32mode2str(enum DIO432_MODE mode)
{
	switch(mode){
	case DIO432_IMMEDIATE:
		return "IMMEDIATE";
	case DIO432_CLOCKED:
		return "CLOCKED";
	default:
		return "DISABLE";
	}
}

struct acq400_dev {
	dev_t devno;
	struct mutex mutex;
	struct mutex awg_mutex;
	struct cdev cdev;
	struct platform_device *pdev;
	struct dentry* debug_dir;
	char *debug_names;

	u32 mod_id;
	wait_queue_head_t waitq;

	struct pl330_client_data *client_data;

	struct OF_PRAMS {
		u32 site;
		u32 dma_channel;
		u32 fifo_depth;
		u32 burst_length;
		u32 irq;
	} of_prams;

	wait_queue_head_t DMA_READY;
	int dma_callback_done;
	int fifo_isr_done;

	struct dma_chan* dma_chan[2];
	int dma_cookies[2];
	struct task_struct* w_task;
	wait_queue_head_t w_waitq;
	int task_active;

	u32 samples_at_event;
	u32 sample_clocks_at_event;
	wait_queue_head_t event_waitq;

	/* fake event isr @@removeme */
	struct timer_list event_timer;

	/* Current DMA buffer information */
	/*dma_addr_t buffer_d_addr;
	void *buffer_v_addr;*/
	size_t count;
	size_t this_count;
	int busy;

	/* Hardware device constants */
	u32 dev_physaddr;
	void *dev_virtaddr;
	u32 dev_addrsize;

	/* Driver reference counts */
	u32 writers;

	struct STATS {
	/* Driver statistics */
		u32 bytes_written;
		u32 writes;
		u32 reads;
		u32 opens;
		u32 closes;
		u32 errors;

		u32 fifo_interrupts;
		u32 dma_transactions;
		int shot;
		int run;
		int fifo_errors;
	} stats;

	int ramp_en;

	enum  { SP_OFF, SP_EN, SP_FRAME } SpadEn;
	enum  { SD_SEW, SD_DI4, SD_DI32 } SpadDix;
	struct Spad {
		unsigned spad_en;	/* 0: off 1: spad 2:frame */
		unsigned len;		/* 1..8 */
		unsigned diX;		/* 0 : off 1: di4 2: di32 */
	} spad;				/** scratchpad enable */
	int data32;
	int adc_18b;			/* @@todo set on probe() */
	int nchan_enabled;
	int word_size;
	int is_slave;			/** @@todo how does this get set? */
	int RW32_debug;

	struct mutex list_mutex;
	struct list_head EMPTIES;	/* empties waiting isr       */
	struct list_head INFLIGHT;	/* buffers in Q 	     */
	struct list_head REFILLS;	/* full buffers waiting app  */
	struct list_head OPENS;		/* buffers in use by app (1) */
	struct HBM** hb;
	int nbuffers;			/* number of buffers available */
	int bufferlen;
	int hitide;
	int lotide;
	int sysclkhz;			/* system clock rate */

	int oneshot;
	struct proc_dir_entry *proc_entry;
	struct CURSOR {
		struct HBM* hb;
		int offset;
	} cursor;
	wait_queue_head_t refill_ready;
	wait_queue_head_t hb0_marker;

	unsigned char *gpg_buffer;
	unsigned *gpg_base;
	unsigned gpg_cursor;		/* bytes .. */

	unsigned *fifo_histo;

	struct RUN_TIME {
		int refill_error;
		int buffers_dropped;		/* a watning, error if quit_on_buffer_exhaustion set*/
		int please_stop;
		unsigned nget;
		unsigned nput;
		unsigned hb0_count;

		unsigned hb0_ix[2];		/* [0]: previous, [1] : crnt  */
		unsigned long hb0_last;
		struct HBM* hbm_m1;		/* previous hbm for hb0 usage */
	} rt;

	struct XO {
		unsigned max_fifo_samples;
		unsigned hshift;		/* scale to histo 256 elems */
		int (*getFifoSamples)(struct acq400_dev* adev);
		/* physchan is offset 0..N-1, lchan is faceplate 1..N */
		int (*physchan)(int lchan);
	} xo;
	struct AO_Immediate {
		union {
			short ch[AO424_MAXCHAN];
			unsigned lw[AO424_MAXCHAN/2];
		} _u;
	} AO_immediate;

	struct AOPlayloop {
		unsigned length;
		unsigned cursor;
		unsigned one_shot;
	} AO_playloop;

	struct CURSOR stream_dac_producer;	/* acq400_streamdac_write */
	struct CURSOR stream_dac_consumer;	/* ao420_isr 		  */

	u32 fake_spad[SPADMAX];

	void (*onStart)(struct acq400_dev *adev);
	void (*onStop)(struct acq400_dev *adev);

	struct Bolo8 {
		char* awg_buffer;
		int awg_buffer_max;
		int awg_buffer_cursor;
		short offset_dacs[8];
	} bolo8;

	/* valid sc only */
	bool is_sc;
	struct acq400_dev* aggregator_set[MAXSITES];

	struct DIO432 {
		enum DIO432_MODE mode;
		unsigned byte_is_output;	/* 1:byte[0], 2:byte[1] 4:byte[2], 8:byte[3] */
		unsigned DI32;
		unsigned DO32;
	} dio432;

	struct AO424 {
		union {
			volatile u32 lw[AO424_MAXCHAN];
			struct {
				u16 ao424_spans[AO424_MAXCHAN];
				u16 ao424_initvals[AO424_MAXCHAN];
			} ch;
		} u;
	} ao424_device_settings;

	struct SewFifo {
		struct mutex sf_mutex;
		struct circ_buf sf_buf;
		struct task_struct* sf_task;
		wait_queue_head_t sf_waitq;
		struct acq400_dev* adev;
		int regoff;
	} sewFifo[2];
};


/** acq400_path_descriptor - one per open path */
struct acq400_path_descriptor {
	struct acq400_dev* dev;
	int minor;
	struct list_head RESERVED;
};

#define PD(filp)		((struct acq400_path_descriptor*)filp->private_data)
#define PDSZ			(sizeof (struct acq400_path_descriptor))
#define ACQ400_DEV(filp)	(PD(filp)->dev)
#define DEVP(adev)		(&(adev)->pdev->dev)


#define MIN_DMA	256

#define MAXDEVICES 6
extern struct acq400_dev* acq400_devices[];
extern const char* acq400_names[];

void acq400_createSysfs(struct device *dev);
void acq400_delSysfs(struct device *dev);

void acq400_module_init_proc(void);
void acq400_module_remove_proc(void);
void acq400_init_proc(struct acq400_dev* adev);
void acq400_del_proc(struct acq400_dev* adev);

void acq400wr32(struct acq400_dev *adev, int offset, u32 value);
u32 acq400rd32(struct acq400_dev *adev, int offset);
u32 acq400rd32_upcount(struct acq400_dev *adev, int offset);

u32 acq420_set_fmt(struct acq400_dev *adev, u32 adc_ctrl);

int getHeadroom(struct acq400_dev *adev);

#define MAXDMA	0x4000

#define GET_FULL_OK		0
#define GET_FULL_DONE 		1
#define GET_FULL_REFILL_ERR	2

#define FIFO_PA(adev)  ((adev)->dev_physaddr + AXI_FIFO)

#define NCHAN	4
#define BYTES_PER_CHANNEL(adev) ((adev)->data32? 4: 2)

#define GET_MOD_ID(adev) 	((adev)->mod_id>>MOD_ID_TYPE_SHL)
#define GET_MOD_ID_VERSION(adev) (((adev)->mod_id>>MOD_ID_VERSION_SHL)&0xff)

#define GET_FPGA_REV(adev)	((adev)->mod_id&0x0000ffff)

static inline int _is_acq42x(struct acq400_dev *adev) {
	switch(GET_MOD_ID(adev)){
	case MOD_ID_ACQ420FMC:
	case MOD_ID_ACQ420FMC_2000:
	case MOD_ID_ACQ425ELF:
	case MOD_ID_ACQ425ELF_2000:
		return true;
	default:
		return false;
	}

}

/** WARNING: assumes CH01 bipolar, ALL bipolar */
#define SPAN_IS_BIPOLAR(adev)	((adev)->ao424_device_settings.u.ch.ao424_spans[0] >= 2)

#define IS_ACQ420(adev) \
	(GET_MOD_ID(adev) == MOD_ID_ACQ420FMC || GET_MOD_ID(adev) == MOD_ID_ACQ420FMC_2000)
#define IS_ACQ425(adev)	\
	(GET_MOD_ID(adev) == MOD_ID_ACQ425ELF || GET_MOD_ID(adev) == MOD_ID_ACQ425ELF_2000)

#define IS_ACQ42X(adev) _is_acq42x(adev)

#define IS_ACQ435(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ435ELF)
#define IS_ACQ430(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ430FMC)
#define IS_ACQ437(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ437ELF)
#define IS_ACQ43X(adev)	(IS_ACQ435(adev) || IS_ACQ430(adev) || IS_ACQ437(adev))

#define IS_ACQ480(adev)	(GET_MOD_ID(adev) == MOD_ID_ACQ480FMC)

#define IS_AO420(adev)  (GET_MOD_ID(adev) == MOD_ID_AO420FMC)
#define IS_AO424(adev)  (GET_MOD_ID(adev) == MOD_ID_AO424ELF)
#define IS_AO42X(adev) 	(IS_AO420(adev) || IS_AO424(adev))

#define IS_ACQ2006SC(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ2006SC)
#define IS_ACQ2006B(adev)  (IS_ACQ2006SC(adev) && GET_MOD_ID_VERSION(adev) != 0)
#define IS_ACQ2106SC(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ2106SC)

#define IS_ACQ2X06SC(adev) (IS_ACQ2006SC(adev) || IS_ACQ2106SC(adev))
#define IS_ACQ1001SC(adev) (GET_MOD_ID(adev) == MOD_ID_ACQ1001SC)

#define IS_ACQx00xSC(adev) (IS_ACQ2X06SC(adev)||IS_ACQ1001SC(adev))

#define IS_AI(adev)	(IS_ACQ42X(adev) || IS_ACQ43X(adev) || IS_ACQ480(adev))
#define IS_BOLO8(adev)	(GET_MOD_ID(adev) == MOD_ID_BOLO8)

#define IS_DIO432FMC(adev)	(GET_MOD_ID(adev) == MOD_ID_DIO432FMC)
#define IS_DIO432PMOD(adev)	(GET_MOD_ID(adev) == MOD_ID_DIO432PMOD)
#define IS_DIO432X(adev)	(IS_DIO432FMC(adev)||IS_DIO432PMOD(adev))

#define IS_PMODADC1(adev)	(GET_MOD_ID(adev) == MOD_ID_PMODADC1)

#define HAS_HDMI_SYNC(adev)	(IS_ACQ1001SC(adev)||IS_ACQ2006B(adev)||IS_ACQ2106SC(adev))
#define IS_DUMMY(adev) 	((adev)->mod_id>>MOD_ID_TYPE_SHL == MOD_ID_DUMMY)

#define FPGA_REV(adev)	((adev)->mod_id&0x00ff)

void xo400_reset_playloop(struct acq400_dev* adev, unsigned playloop_length);

#define SYSCLK_M100	100000000
#define SYSCLK_M66       66000000

#define ACQ2006_COUNTERS	0x100

#define ACQ2006_CLK_COUNT(n)	(ACQ2006_COUNTERS+((n)+ 0)*sizeof(u32))
#define ACQ2006_CLK_COUNT_MASK	0x0fffffff
#define ACQ2006_TRG_COUNT(n) 	(ACQ2006_COUNTERS+((n)+ 8)*sizeof(u32))
#define ACQ2006_EVT_COUNT(n) 	(ACQ2006_COUNTERS+((n)+16)*sizeof(u32))
#define ACQ2006_SYN_COUNT(n) 	(ACQ2006_COUNTERS+((n)+24)*sizeof(u32))

#define ACQ2006_TRG_COUNT_MASK	0x0000ffff
#define ACQ2006_SYN_COUNT_MASK  0x0000ffff
#define ACQ2006_EVT_COUNT_MASK	0x0000ffff

/* SC "site 0" registers */
#define MOD_CON			(0x0004)
#define AGGREGATOR		(0x0008)
#define AGGSTA			(0x000c)
#define DATA_ENGINE_0		(0x0010)
#define DATA_ENGINE_1		(0x0014)
#define DATA_ENGINE_2		(0x0018)
#define DATA_ENGINE_3		(0x001c)
#define DATA_ENGINE(e)		(0x0010+((e)*4))
#define GPG_CTRL		(0x0020)  /* per telecon with John 201402061145 */
#define GPG_CONTROL		GPG_CTRL
#define HDMI_SYNC_DAT		(0x0024)
#define HDMI_SYNC_OUT_SRC	(0x0028)   	/* HSO_xx	*/
#define EVT_BUS_SRC		(0x002c)	/* EBS_xx	*/
#define SIG_SRC_ROUTE		(0x0030)	/* SSR_xx	*/
/* scratchpad */


#define DATA_ENGINE_SELECT_AGG	(1<<14)
#define DE_ENABLE		(1<<0)


#define AGG_SNAP_DI_SHL		30
#define AGG_SNAP_DI_MASK	0x3
#define AGG_SNAP_DI_OFF		0
#define AGG_SNAP_DI_4		2
#define AGG_SNAP_DI_32		3

#define AGG_DECIM_MASK		0xf
#define AGG_DECIM_SHL		24

#define AGG_SPAD_EN		(1<<17)
#define AGG_SPAD_FRAME		(1<<16)
#define AGG_SPAD_LEN_SHL	4
#define AGG_SPAD_LEN_MASK	0x7

#define AGG_SPAD_ALL_MASK \
	(AGG_SPAD_LEN_MASK<<AGG_SPAD_LEN_SHL|\
	  AGG_SPAD_FRAME|AGG_SPAD_EN| AGG_SNAP_DI_MASK<<AGG_SNAP_DI_SHL)

#define SET_AGG_SPAD_LEN(len)	((((len)-1)&AGG_SPAD_LEN_MASK)<<AGG_SPAD_LEN_SHL)
#define SET_AGG_SNAP_DIx(dix)	(((dix)&AGG_SNAP_DI_MASK)<<AGG_SNAP_DI_SHL)

#define AGG_FIFO_RESET		(1<<1)
#define AGG_ENABLE		(1<<0)

/* s=1..6 */
#define AGG_MOD_EN(s, shift)	(1 << ((s)-1+(shift)))
#define AGGREGATOR_MSHIFT	18
#define DATA_ENGINE_MSHIFT	 8
#define AGGREGATOR_ENABLE(s)	AGG_MOD_EN(s, AGGREGATOR_MSHIFT)
#define DATA_ENGINE_ENABLE(s)	AGG_MOD_EN(s,  DATA_ENGINE_MSHIFT)

#define AGG_SITES_MASK		0x3f
#define DATA_MOVER_EN		0x1

#define AGG_SIZE_MASK		0xff
#define AGG_SIZE_SHL		8
#define AGG_SIZE_UNIT_OLD	512
#define AGG_SIZE_UNIT_NEW	128

#define AGG_SIZE_UNIT(adev)	\
	(IS_ACQ2006SC(adev)&&FPGA_REV(adev)<=8? \
		AGG_SIZE_UNIT_OLD: AGG_SIZE_UNIT_NEW)









#define AGGSTA_FIFO_COUNT	0x000000ff
#define AGGSTA_FIFO_STAT	0x00000f00
#define AGGSTA_ENGINE_STAT	0x000f0000


#define MCR_MOD_EN			(1<<0)
#define MCR_PSU_SYNC 			(1<<1)
#define MCR_FAN_EN			(1<<2)
#define ACQ1001_MCR_CELF_PSU_EN		(1<<3)
#define MCR_SOFT_TRIG			(1<<4)
#define ACQ1001_MCR_PWM_BIT		8
#define ACQ1001_MCR_PWM_MAX		0x40
#define ACQ1001_MCR_PWM_MASK		((ACQ1001_MCR_PWM_MAX-1)<<ACQ1001_MCR_PWM_BIT)
#define ACQ1001_MCR_PWM_MIN		0x04


#define GPG_CTRL_TOPADDR		0x3ff00000
#define GPG_CTRL_TRG_SEL		0x000e0000
#define GPG_CTRL_TRG_RISING		0x00010000
#define GPG_CTRL_SYNC_SEL		0x0000e000
#define GPG_CTRL_SYNC_RISING		0x00001000
#define GPG_CTRL_CLK_SEL		0x00000e00
#define GPG_CTRL_CLK_RISING		0x00000100
#define GPG_CTRL_EXT_TRG		0x00000040
#define GPG_CTRL_EXT_SYNC		0x00000020
#define GPG_CTRL_EXTCLK			0x00000010
#define GPG_CTRL_MODE			0x00000006
#define GPG_CTRL_ENABLE			0x00000001

#define GPG_CTRL_TOPADDR_SHL		24
#define GPG_CTRL_TRG_SHL		16
#define GPG_CTRL_SYNC_SHL		12
#define GPG_CTRL_CLK_SHL		8
#define GPG_CTRL_MODE_SHL		1

#define GPG_MEM_BASE			0xf000
#define GPG_MEM_SIZE			0x1000
#define GPG_MEM_ACTUAL			0x0800

/* HDMI_SYNC_DAT bits */
#define HDMI_SYNC_IN_SYNCb		15
#define HDMI_SYNC_IN_TRGb		14
#define HDMI_SYNC_IN_GPIOb		13
#define HDMI_SYNC_IN_CLKb		12
#define HDMI_SYNC_OUT_CABLE_DETNb	 8
#define HDMI_SYNC_OUT_SYNCb		 3
#define HDMI_SYNC_OUT_TRGb		 2
#define HDMI_SYNC_OUT_GPIOb		 1
#define HDMI_SYNC_OUT_CLKb		 0

#define HDMI_SYNC_IN_SHL		12
#define HDMI_SYNC_OUT_SHL		0
#define HDMI_SYNC_MASK			0x0f

/* HDMI_SYNC_OUT_SRC */
#define HSO_SYNC_SHL			24
#define HSO_TRG_SHL			16
#define HSO_GPIO_SHL			 8
#define HSO_CLK_SHL			 0

#define HSO_SS_SEL_SHL			3
#define HSO_XX_SEL_MASK			(0x3)
#define HSO_DO_SEL			(0x0)
#define HSO_SIG_BUS_SEL			(0x2)
#define HSO_GPG_BUS_SEL			(0x3)
#define HSO_DX_MASK			0x7

/* EVENT BUS SOURCE */
#define EBS_MASK	0xf
#define EBS_TRG		0x0
#define EBS_GPG		0x1
#define EBS_HDMI	0x2
#define EBS_CELF	0x3

#define EBS_7_SHL	28
#define EBS_6_SHL	24
#define EBS_5_SHL	20
#define EBS_4_SHL	16
#define EBS_3_SHL	12
#define EBS_2_SHL	 8
#define EBS_1_SHL	 4
#define EBS_0_SHL	 0

/* SIG SRC ROUTE */
#define SSR_MASK	0xf
#define SSR_EXT		0x0
#define SSR_HDMI	0x1
#define SSR_CELF	0x2

#define SSR_SYNC_1_SHL	20
#define SSR_SYNC_0_SHL	16
#define SSR_TRG_1_SHL	12
#define SSR_TRG_0_SHL	 8
#define SSR_CLK_1_SHL	 4
#define SSR_CLK_0_SHL	 0

#define EXT_DX	0			/* External clock */
#define MB_DX	1 			/* Motherboard clock */
#define SITE2DX(site) 	((site)+1)

/* HDMI_SYNC_CON */
#define SYNC_CON
int ao420_physChan(int lchan /* 1..4 */ );

static inline void set_gpg_top(struct acq400_dev* adev, u32 gpg_top)
{
	u32 gpg_ctrl = acq400rd32(adev, GPG_CTRL);
	gpg_top <<= GPG_CTRL_TOPADDR_SHL;
	gpg_top &= GPG_CTRL_TOPADDR;
	gpg_ctrl &= ~GPG_CTRL_TOPADDR;
	gpg_ctrl |= gpg_top;
	acq400wr32(adev, GPG_CTRL, gpg_ctrl);
}

#define AOSAMPLES2BYTES(adev, xx) ((xx)*(adev)->nchan_enabled*(adev)->word_size)
#define AOBYTES2SAMPLES(adev, xx) ((xx)/(adev)->nchan_enabled/(adev)->word_size)


static inline unsigned xo400_getFillThreshold(struct acq400_dev *adev)
{
	return adev->xo.max_fifo_samples/128;
}
#define MAX_LOTIDE(adev) \
	(adev->xo.max_fifo_samples - xo400_getFillThreshold(adev)*2)




static inline int ao420_getFifoHeadroom(struct acq400_dev* adev) {
	/* pgm: don't trust it to fill to the top */
	unsigned samples = adev->xo.getFifoSamples(adev);
	unsigned maxsam = adev->xo.max_fifo_samples - 8;

	if (samples > maxsam){
		return 0;
	}else{
		return maxsam - samples;
	}
}

void set_spadN(struct acq400_dev* adev, int n, u32 value);
u32 get_spadN(struct acq400_dev* adev, int n);

struct acq400_dev* acq400_lookupSite(int site);

/* AO424 */

#define DAC_424_CGEN_ODD_CHANS		(1<<4)
#define DAC_424_CGEN_DISABLE_D		(1<<3)
#define DAC_424_CGEN_DISABLE_C		(1<<2)
#define DAC_424_CGEN_DISABLE_B		(1<<1)
#define DAC_424_CGEN_DISABLE_A		(1<<0)

#define DAC_424_CGEN_DISABLE_X		(0xf)


/* DIO432 */

#define DIO432_MOD_ID		0x00
#define DIO432_DIO_CTRL		0x04
#define DIO432_TIM_CTRL		0x08
#define DIO432_DI_HITIDE	0x0C
#define DIO432_DI_FIFO_COUNT	0x10
#define DIO432_DI_FIFO_STATUS	0x14
#define DIO432_DIO_ICR		0x18	/* same as regular ICR */

#define DIO432_DIO_SAMPLE_COUNT 0x40
#define DIO432_DIO_CPLD_CTRL	0x44

#define DIO432_DO_LOTIDE	0x8c
#define DIO432_DO_FIFO_COUNT	0x90
#define DIO432_DO_FIFO_STATUS	0x94



#define DIO432_FIFO		0x1000


#define DIO432_CTRL_SHIFT_DIV_SHL	(9)
#define DIO432_CTRL_EXT_CLK_SYNC (1<<7)
#define DIO432_CTRL_LL		(1 << 8)
#define DIO432_CTRL_RAMP_EN 	(1 << 5)	/* Deprecated, sadly. Use SPAD */
#define DIO432_CTRL_DIO_EN	(1 << 4)
#define DIO432_CTRL_DIO_RST	(1 << 3)
#define DIO432_CTRL_FIFO_EN	(1 << 2)
#define DIO432_CTRL_FIFO_RST	(1 << 1)
#define DIO432_CTRL_MODULE_EN	(1 << 0)	/* enable at enumeration, leave up */

#define DIO432_CTRL_RST	(DIO432_CTRL_DIO_RST|DIO432_CTRL_FIFO_RST)

#define DIO432_CTRL_SHIFT_DIV_FMC	(0<<DIO432_CTRL_SHIFT_DIV_SHL)
#define DIO432_CTRL_SHIFT_DIV_PMOD	(1<<DIO432_CTRL_SHIFT_DIV_SHL)

#define PMODADC1_CTRL_EXT_CLK_FROM_SYNC (1<<7)
#define PMODADC1_CTRL_DIV		(3<<DIO432_CTRL_SHIFT_DIV_SHL)


#define DIO432_FIFSTA_FULL	(1<<3)
#define DIO432_FIFSTA_EMPTY	(1<<2)
#define DIO432_FIFSTA_OVER	(1<<1)
#define DIO432_FIFSTA_UNDER	(1<<0)

#define DIO432_FIFSTA_CLR	0xf

#define DIO432_CPLD_CTRL_COMMAND_COMPLETE	(1<<9)
#define DIO432_CPLD_CTRL_COMMAND_WRITE		(1<<8)
#define DIO432_CPLD_CTRL_COMMAND_DATA		0xff

#define DIO432_CPLD_CTRL_OUTPUT(byte)		(1<<(byte))


void write32(volatile u32* to, volatile u32* from, int nwords);

void dio432_set_mode(struct acq400_dev* adev, enum DIO432_MODE mode);
/* immediate, clocked */
void dio432_init_clocked(struct acq400_dev* adev);
void dio432_disable(struct acq400_dev* adev);

extern int a400fs_init(void);
extern void a400fs_exit(void);
extern const char* devname(struct acq400_dev *adev);


#define AO424_DAC_CTRL_SPAN	(1<<5)
#define AO424_DAC_FIFO_STA_SWC	(1<<8)

int ao424_set_spans(struct acq400_dev* adev);

void acq400_sew_fifo_init(struct acq400_dev* adev, int ix);
int acq400_sew_fifo_destroy(struct acq400_dev* adev, int ix);
int acq400_sew_fifo_write_bytes(
		struct acq400_dev* adev, int ix, const char __user *buf, size_t count);
void measure_ao_fifo(struct acq400_dev *adev);

static inline void x400_enable_interrupt(struct acq400_dev *adev)
{
	u32 int_ctrl = acq400rd32(adev, ADC_INT_CSR);
	acq400wr32(adev, ADC_INT_CSR,	int_ctrl|0x1);
}

static inline void x400_disable_interrupt(struct acq400_dev *adev)
{
	acq400wr32(adev, ADC_INT_CSR, 0x0);
}


static inline u32 x400_get_interrupt(struct acq400_dev *adev)
{
	return acq400rd32(adev, ADC_INT_CSR);
}

static inline void x400_clr_interrupt(struct acq400_dev *adev, u32 int_csr)
{
	acq400wr32(adev, ADC_INT_CSR, int_csr);
}

static inline void x400_set_interrupt(struct acq400_dev *adev, u32 int_csr)
{
	acq400wr32(adev, ADC_INT_CSR, int_csr);
}

short ao424_fixEncoding(struct acq400_dev *adev, int pchan, short value);
/* LTC2752 data is always unsigned. But bipolar ranges are represented ext as
 * signed this converts, dependent on range
 */

extern int acq400_set_bufferlen(struct acq400_dev *adev, int _bufferlen);

enum AO_playloop_oneshot { AO_continuous, AO_oneshot, AO_oneshot_rearm };

void acq2006_estop(struct acq400_dev *adev);

#endif /* ACQ420FMC_H_ */
