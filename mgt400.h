/*
 * mgt400.h
 *
 *  Created on: 12 Jan 2015
 *      Author: pgm
 */

#ifndef MGT400_H_
#define MGT400_H_

struct mgt400_dev {
	dev_t devno;
	struct of_prams {
		int site;
		int irq;
		int sn;
	} of_prams;
	char devname[16];
	struct cdev cdef;
	struct platform_device *pdev;
	struct resource *mem;
	void *dev_virtaddr;
	struct cdev cdev;
	char* debug_names;
	struct dentry* debug_dir;
	int RW32_debug;
};

extern struct mgt400_dev* mgt400_devices[];

void mgt400wr32(struct mgt400_dev *adev, int offset, u32 value);
u32 mgt400rd32(struct mgt400_dev *adev, int offset);
void mgt400_createDebugfs(struct mgt400_dev* adev);
void mgt400_createSysfs(struct device *dev);
void mgt400_createDebugfs(struct mgt400_dev* adev);
void mgt400_removeDebugfs(struct mgt400_dev* adev);

/* ZYNQ : RW    HOST: RO (at start only) */
#undef MOD_ID
#define MOD_ID		(0x0000)
#define ZDMA_CR		(0x0004)
#define HEART		(0x0008)
#define AURORA_CR	(0x000C)
#define AURORA_SR	(0x0010)
#define COMMS_TXB_FSR	(0x0014)
#define COMMS_TXB_FCR	(0x0018)
#define COMMS_RXB_FSR   (0x001C)
#define COMMS_RXB_FCR	(0x0020)

#define ZDMA_CR_ENABLE		(1<<0)

#define AURORA_CR_ENA		(1<<31)
#define AURORA_CR_CLR		(1<<7)
#define AURORA_CR_PWR_DWN	(1<<4)
#define AURORA_CR_LOOPBACK	(0x7)

#define AURORA_SR_HARD_ERR	(1<<6)
#define AURORA_SR_SOFT_ERR	(1<<5)
#define AURORA_SR_FRAME_ERR	(1<<4)
#define AURORA_SR_CHANNEL_UP	(1<<1)
#define AURORA_SR_LANE_UP	(1<<0)

#define AURORA_SR_ERR_XX \
	(AURORA_SR_HARD_ERR|AURORA_SR_SOFT_ERR|AURORA_SR_FRAME_ERR)

#define AURORA_SR_UP_MASK 	(AURORA_SR_CHANNEL_UP|AURORA_SR_LANE_UP)

#define AURORA_SR_ERR(xx) (((xx)&AURORA_SR_ERR_XX)>>4)
#define AURORA_SR_UP(xx)  (((xx)&AURORA_SR_UP_MASK)>>0)

/** PCIE REGS : ZYNQ:RO HOST: RW */
#define PCIE_CTRL 	(0x1004)
#define PCIE_INTR	(0x1008)
#define PCI_CSR		(0x100C)
#define PCIE_DEV_CSR	(0x1010)
#define PCIE_LINK_CSR	(0x1014)
#define PCIE_CONF	(0x1018)
#define PCIE_BUF_CTRL	(0x101C)

/** DMA REGS : ZYNQ:RO HOST:RW */
#define DMA_TEST		(0x2000)
#define DMA_CTRL		(0x2004)
#define DMA_FIFO_SR		(0x2008)
#define DESC_FIFO_SR		(0x200C)
#define DMA_PUSH_DESC_SR	(0x2010)
#define DMA_PULL_DESC_SR	(0x2014)
#define DMA_PUSH_DESC_FIFO	(0x2040)
#define DMA_PULL_DESC_FIFO	(0x2080)

#define DMA_DATA_PULL_SHL		16
#define DMA_DATA_PUSH_SHL		0

#define DMA_DATA_FIFO_COUNT		0xfff0
#define DMA_DATA_FIFO_COUNT_SHL		4
#define DMA_DATA_FIFO_FLAGS		0x000f

#endif /* MGT400_H_ */
