/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_nand.c
 *
 * created: 2022-03-04
 *  author: Bian
 */

/******************************************************************************
 * 本代码不是线程安全的.
 *
 * 如果由yaffs 调用, 需要实现yaffs_lock() 和yaffs_unlock() 函数.
 */

#include "bsp.h"

//-----------------------------------------------------------------------------

#if BSP_USE_NAND

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "osal.h"

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_nand.h"
#include "ls2k_dma_hw.h"
#include "ls2k_nand_hw.h"

#if 0
#define DBG_NAND(...)    printk(__VA_ARGS__)
#else
#define DBG_NAND(...)
#endif

/*
 * NAND with DMA 0, XXX Never used this interrupt,
 */
#define USE_DMA0_INTERRUPT  1

#define PARAM_NAND_SIZE     1

#define NAND_USE_ECC        0

/******************************************************************************
 * ls2k1000 nand priv defination
 */
typedef struct
{
	/* Hardware shortcuts */
	HW_NAND_t     *hwNAND;				/* NAND 寄存器 */
#if PARAM_NAND_SIZE
	unsigned int   nand_size;           /* NAND_SIZE_1Gb ~ NAND_SIZE_8Gb */
#endif
#if NAND_USE_ECC
    int            hwECC;               /* 使用HW ECC 模式 */
#endif
	unsigned int   dmaCtrl;				/* DMA  控制寄存器, XXX 64位地址 */
	DMA_DESC_t    *dmaDesc;				/* DMA  描述符 */
	unsigned char *dmaBuf;				/* DMA  数据缓冲区 */

	int            intialized;			/* Driver state 没有初始化时 ? */

	/* Statics */
#if (USE_DMA0_INTERRUPT)
	unsigned int intr_cnt;
#endif
	unsigned int error_cnt;
	unsigned int read_bytes;
	unsigned int read_cnt;
	unsigned int readerr_cnt;
	unsigned int write_bytes;
	unsigned int write_cnt;
	unsigned int writeerr_cnt;
	unsigned int erase_cnt;
	unsigned int eraseerr_cnt;
} NAND_t;

/*
 * DMA descriptor buffer and DMA transfer buffer.
 */
#define FULL_PAGE_SIZE	(BYTES_OF_PAGE+OOBBYTES_OF_PAGE)

/*
 * DMA ORDER_ADDR_IN Register's addr-ask, must align 64
 */
static DMA_DESC_t dma_desc_base __attribute__((aligned(64)));

/*
 * DMA transfer 4 words once, align 16
 */
static unsigned char page_buf_base[FULL_PAGE_SIZE] __attribute__((aligned(16)));

/*
 * Here is NAND interface defination
 */
static NAND_t ls2k_NAND;

/******************************************************************************
 * Hardware Operating Implement
 ******************************************************************************/

static int NAND_read_id(NAND_t *pNand, uint64_t *id)
{
	unsigned int saved_timing;

    saved_timing = pNand->hwNAND->timing;
	pNand->hwNAND->timing = 0x0412;

	pNand->hwNAND->cmd = 0;
	pNand->hwNAND->cmd = NAND_CMD_RD_ID;
	pNand->hwNAND->cmd = NAND_CMD_RD_ID | NAND_CMD_VALID;
	
    while (!(pNand->hwNAND->cmd & NAND_CMD_DONE))
        ;

    *id = ((uint64_t)(pNand->hwNAND->id_h & NAND_ID_H_MASK) << 32) | pNand->hwNAND->id_l;

    pNand->hwNAND->timing = saved_timing;

    return 0;
}

static int NAND_waitdone(NAND_t *pNand, int timeout_us)
{
 	while (timeout_us > 0)
	{
		delay_us(10);							    /* 一次延时 1us */
		if (pNand->hwNAND->cmd & NAND_CMD_DONE)     // NAND_CMD_RDY_0 ?
		{
            pNand->hwNAND->cmd &= ~NAND_CMD_DONE;   /* 需要软件清零 */
        	return 0;
        }

		timeout_us -= 10;
	}

	/*
	 * If timeout, send cancel command.
	 * Sequence: 0 - 0 - nand_cmd_done?
	 */
	pNand->hwNAND->cmd = 0;
	pNand->hwNAND->cmd = 0;

	return -ETIMEDOUT;
}

static void NAND_chip_reset(NAND_t *pNand)
{
	int rt;

	pNand->hwNAND->cmd = 0;
	pNand->hwNAND->cmd = NAND_CMD_RESET;
	pNand->hwNAND->cmd = NAND_CMD_RESET | NAND_CMD_VALID;

	/* delay 500us, wait */
	rt = -NAND_waitdone(pNand, 500);
	if (0 != rt)
	{
    	pNand->error_cnt++;
    }
}

/****************************************************************************************
 * Initialize
 ****************************************************************************************/

static void NAND_hw_initialize(NAND_t *pNand)
{
    unsigned long apb_address0;

    /*
     * LS2K1000LA 从APB配置头读 NAND IO base
     */
    apb_address0 = READ_REG64(APB_CFG_HEAD_BASE + 0x10) & ~0x0Ful;
    pNand->hwNAND = (HW_NAND_t *)PHYS_TO_UNCACHED(apb_address0 + 0x6000);

	pNand->hwNAND->cs_rdy_map = 0x88442200;
#if PARAM_NAND_SIZE
	pNand->hwNAND->param  = pNand->nand_size;
#else
	pNand->hwNAND->param  = NAND_SIZE_4Gb;
#endif
	pNand->hwNAND->timing = 0x0412;  				    /* Default value is 0x0412. */

#if NAND_USE_ECC
	pNand->hwECC = 1;
#endif

    /*
     * LS2K1000LA 配置 NAND 使用DMA0
     */
    AND_REG64(CHIP_APBDMA_BASE, ~DMA_NAND_SEL_MASK);
    //OR_REG64(CHIP_APBDMA_BASE, DMA_NAND_SEL_0);       /* XXX 如果使用其它的 DMA 通道... */

	pNand->dmaCtrl = DMA0_BASE;                         /* DMA ORDER_ADDR_IN */
    pNand->dmaDesc = (DMA_DESC_t *)(&dma_desc_base);	/* DMA Descriptor, aligned 64 */
	pNand->dmaBuf = (unsigned char *)page_buf_base;     /* DMA Buffer, aligned 32 */

	NAND_chip_reset(pNand);	                            /* Nand chipset reset */

#if (USE_DMA0_INTERRUPT)
    WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);        /* disable irq */
#endif
}

/****************************************************************************************
 * input param:	 1. bufSize  - buffer length of being r/w data
 * 				 2. colAddr  - the r/w offset in destination page
 * 				 3. opFlags  - will be r/w "main" or "spare" or "both"
 * output param: 1. nand_cmd - will be r/w "main" or "spare" or "both"
 * 				 2. offset   - colAddr, when r/w in "spare", maybe minus 2048
 * 				 3. size     - acculate size will be r/w
 *
 *                 0                                              2048        2112
 * PAGE            |----------------------main-----------------------|---spare---|
 *
 * 1."main"        ^       |++++++++++++++++++++++++++++++++++++++++++++|
 *   IN:           ^    colAddr                                       bufSize+
 *   OUT:   nand_cmd = main
 *          offset   = colAddr, (if offset>2048, do nothing anyway)
 *          size     = min(bufSize, 2048-colAddr) --> limit in main
 *
 * 2."spare"                                                         ^   |^^^^^^|
 *   IN:                                                              colAddr  bufSize+
 *   OUT:   nand_cmd = spare
 *          offset   = colAddr, (if offset>64, do nothing anyway)
 *          size     = min(bufSize, 64-colAddr) --> limit in spare
 *
 * 3."both"        ^
 *
 * 3.1 AT "main"   ^   |++++++++++++++++++++++++++++++++++++++|
 *	               ^ colAddr                                bufSize+
 *   OUT:   nand_cmd = main
 *          offset   = colAddr
 *          size     = bufSize
 *
 * 3.2 AT "spare"  ^                                                     |^^^^^^|
 *   IN:           ^                                                  colAddr  bufSize+
 *   OUT:   nand_cmd = spare
 *          offset   = colAddr-2048, (if offset>64, do nothing anyway)
 *          size     = min(bufSize, 64-colAddr) --> limit in spare
 *
 * 3.3 AT "BOTH"   ^
 *   IN:           ^                   |^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|
 *	                                 colAddr                                bufSize+
 *   OUT:   nand_cmd = main+spare
 *          offset   = colAddr
 *          size     = min(bufSize, 2112-colAddr) --> limit in page
 *
 ****************************************************************************************/

static void NAND_calc_rw_size(unsigned int bufSize, unsigned int colAddr,
							  unsigned int opFlags, unsigned int *cmd,
							  int *offset, int *rwsize)
{
	int remain_size=0;

	/*
	 * XXX 读写数据必须4字节的倍数
	 */

	*rwsize = 0;
	if ((bufSize == 0) ||
	   ((opFlags == NAND_OP_MAIN) && (colAddr >= BYTES_OF_PAGE)) ||
	   ((opFlags == NAND_OP_SPARE) && (colAddr >= OOBBYTES_OF_PAGE)) ||
	   (((opFlags == (NAND_OP_MAIN | NAND_OP_SPARE)) && (colAddr >= FULL_PAGE_SIZE))))
	{
		return;
	}

	switch (opFlags)
	{
		case NAND_OP_MAIN:
			*cmd |= NAND_CMD_MAIN;
			*offset = colAddr;
			remain_size = BYTES_OF_PAGE - colAddr;
			*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			break;

		case NAND_OP_SPARE:
			*cmd |= NAND_CMD_SPARE;
			*offset = colAddr;
			remain_size = OOBBYTES_OF_PAGE - colAddr;
			*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			break;

		case NAND_OP_MAIN | NAND_OP_SPARE:
			if (colAddr + bufSize < BYTES_OF_PAGE)      /* High-boundary in "main" area */
			{
				*cmd |= NAND_CMD_MAIN;
				*offset = colAddr;
				remain_size = BYTES_OF_PAGE - colAddr;
				*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			}
			else                                        /* high-boundary in "spare" area */
			{
				if (colAddr < BYTES_OF_PAGE)            /* Low-boundary in "main" area */
				{
					*cmd |= NAND_CMD_MAIN | NAND_CMD_SPARE;
					*offset = colAddr;
					remain_size = FULL_PAGE_SIZE - colAddr;
					*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
				}
				else                                    /* Low-boundary in "spare" area */
				{
					*cmd |= NAND_CMD_SPARE;
					*offset = colAddr - BYTES_OF_PAGE;
					remain_size = OOBBYTES_OF_PAGE - (colAddr - BYTES_OF_PAGE);
					*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
				}
			}
			break;

		default:
			break;
	}
}

#if 0
static const char cap2cs[16] = {
    [0] = 16, [1] = 17, [2] = 18, [3] = 19, [4] = 19, [5] = 19, [6] = 20, [7] = 21,
	[9] = 14, [10] = 15, [11] = 16, [12] = 17, [13] = 18
};
#endif

static void NAND_send_rw_command(NAND_t *pNand, unsigned int pageNum,
							     unsigned int offset, unsigned int nand_cmd)
{
	pNand->hwNAND->cmd = 0;

	if (NAND_CMD_SPARE & nand_cmd)
	{
		pNand->hwNAND->addr_r = pageNum; // | (NAND_CS << 18);
		if (NAND_CMD_MAIN & nand_cmd)
		{
        	pNand->hwNAND->addr_c = offset;
        }
		else
		{
        	pNand->hwNAND->addr_c = offset + NAND_ADDR_C_2K_BIT;
        }
	}
	else
	{
		pNand->hwNAND->addr_r = pageNum; // | (NAND_CS << 18);
		pNand->hwNAND->addr_c = offset;
	}

    /* XXX haven't started yet */
}

static inline void NAND_cmd_start(NAND_t *pNand, unsigned int nand_cmd)
{
    /*
     * Only Command main/spare/write/read: 0x0306
     */
    nand_cmd &= (NAND_CMD_SPARE | NAND_CMD_MAIN | NAND_CMD_WRITE | NAND_CMD_READ);
    pNand->hwNAND->cmd = nand_cmd;
    pNand->hwNAND->cmd = nand_cmd | NAND_CMD_VALID;
}

static void NAND_dma_desc_init(NAND_t *pNand, void *mem_addr)
{
	pNand->dmaDesc->next_desc_lo = 0;
	pNand->dmaDesc->next_desc_hi = 0;

	pNand->dmaDesc->length = 0;
	pNand->dmaDesc->step_length = 0;
	pNand->dmaDesc->step_times = 1;
	pNand->dmaDesc->command = 0;

	/*
     * Must be physical address
     */
	pNand->dmaDesc->mem_addr_lo = VA_TO_PHYS(mem_addr);
	pNand->dmaDesc->mem_addr_hi = 0;

	/*
     * DMA操作的APB设备地址, 这个地址是28位: bit[27:0], 但实际地址有29位 ???
     */
	pNand->dmaDesc->dev_addr = 0x1FF58040; // & DESC_DADDR_MASK;
}

static void NAND_dma_start(NAND_t *pNand, void *mem_addr, unsigned int size, int rw)
{
	uint64_t dma_cmd;

	NAND_dma_desc_init(pNand, mem_addr);            /* Set DMA descriptor */

	pNand->dmaDesc->length = (size + 3) >> 2;		/* transfer 4 bytes once */
	pNand->dmaDesc->step_times = 1;

	if (DMA_READ == rw) 	/* READ */
	{
    	pNand->dmaDesc->command = DESC_CMD_INT_EN;
    }
	else 					/* WRITE */
	{
    	pNand->dmaDesc->command = DESC_CMD_INT_EN | DESC_CMD_R_W;
    }

	/*
     * Start DMA control, physical address
     */
    dma_cmd = VA_TO_PHYS(pNand->dmaDesc) & DMA_ASK_ADDR_MASK;
	dma_cmd |= DMA_START | DMA_ASK_VALID;
	dma_cmd |= DMA_AXI_COHERENT | DMA_64BIT;    /* TODO test */

    LOCK_DMA;
	WRITE_REG64(pNand->dmaCtrl, dma_cmd);
}

#if USE_DMA0_INTERRUPT
/*
 * 防止中断被禁止
 */
#include <larchintrin.h>

static int NAND_check_dma0_irq_occurred(NAND_t *pNand)
{
    unsigned int r;

    r = __csrrd_d(LA_CSR_CRMD);
    if (r & CSR_CRMD_IE)
        return 0;

    r = READ_REG32(INTC1_SR_BASE);
    if (r & INTC1_DMA0_BIT)
    {
        WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);
	    WRITE_REG64(pNand->dmaCtrl, DMA_STOP);			/* Stop DMA */
        UNLOCK_DMA;
        WRITE_REG32(INTC1_SET_BASE, INTC1_DMA0_BIT);
        return 1;
    }

    return 0;
}

#endif // #if USE_DMA0_INTERRUPT

static int NAND_read_page(NAND_t    *pNand,
						  unsigned int   pageNum,
						  unsigned int   colAddr,
						  unsigned char *dataBuf,
						  unsigned int   bufSize,
						  unsigned int   opFlags,
						  int           *rdSize)
{
	int rt, rd_size = 0, offset = 0;
	unsigned int nand_cmd = NAND_CMD_READ;
	unsigned int saved_timing;
	unsigned char *rd_buf;

	*rdSize = 0;

	if (pageNum >= PAGES_OF_CHIP)
    {
    	return -EINVAL;
    }

	if (dataBuf == NULL)
    {
    	return -EFAULT;
    }

	/*
	 * Calculate read parameters
	 */
	NAND_calc_rw_size(bufSize, colAddr, opFlags, &nand_cmd, &offset, &rd_size);

	if (0 == rd_size)
	{
    	return -ENODATA;
    }

    /*
     * 如果传入的 dataBuf 和 bufSize是对齐的, 就不使用内置 dmaBuf
     */
    if ((((unsigned long)dataBuf & 0x0F) == 0) && ((bufSize & 0x03) == 0))
    {
        rd_buf = dataBuf;
    }
    else
    {
        rd_buf = pNand->dmaBuf;
    }

#if USE_DMA0_INTERRUPT
    WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);	        /* 清中断标志 */
#endif

	/*
	 * Begin do read work
	 */
	WRITE_REG64(pNand->dmaCtrl, DMA_STOP);

    saved_timing = pNand->hwNAND->timing;
	pNand->hwNAND->op_num = rd_size;
	pNand->hwNAND->timing = 0x0206;
#if PARAM_NAND_SIZE
    pNand->hwNAND->param  = (rd_size << NAND_PARAM_OPSCOPE_SHIFT) | pNand->nand_size;
#else
    pNand->hwNAND->param  = (rd_size << NAND_PARAM_OPSCOPE_SHIFT) | NAND_SIZE_4Gb;
#endif

	NAND_send_rw_command(pNand, pageNum, offset, nand_cmd);	/* Send NAND commmand */
    NAND_cmd_start(pNand, nand_cmd);

	NAND_dma_start(pNand, rd_buf, rd_size, DMA_READ);		/* Start DMA command */

	/*
	 * Wait DMA transfer done.
	 */
#if (!USE_DMA0_INTERRUPT)
	while (READ_REG64(pNand->dmaCtrl) & DMA_START)
        ;

  #if BSP_USE_OS
    SLEEP(1);                   /* RTOS 切换出时间? */
  #else
    delay_us(rd_size / 4);      /* XXX 没有等待DMA完成操作的方法, 延时 */
  #endif

#else
    while (!(READ_REG64(pNand->dmaCtrl) & DMA_STOP))
    {
        delay_us(1);

        if (NAND_check_dma0_irq_occurred(pNand))
            break;
    }
    
#endif

	rt = -NAND_waitdone(pNand, 400);   	        /* delay 400us */
	if (0 == rt)
	{
		if ((rd_buf != dataBuf) && (dataBuf != pNand->dmaBuf))  /* Copy-back data to user. */
		{
			memcpy((void *)dataBuf, (void *)pNand->dmaBuf, rd_size);
		}

        *rdSize = rd_size;					    /* Return the accurate read data size */
	}
	else
	{
		printk("NAND read timeout.\r\n");
		pNand->readerr_cnt++;
	}

    pNand->hwNAND->timing = saved_timing;
#if (!USE_DMA0_INTERRUPT)
	WRITE_REG64(pNand->dmaCtrl, DMA_STOP);      /* Stop DMA */
	UNLOCK_DMA;
#endif

	return rt;
}

static int NAND_write_page(NAND_t    *pNand,
						   unsigned int   pageNum,
						   unsigned int   colAddr,
						   unsigned char *dataBuf,
						   unsigned int   bufSize,
						   unsigned int   opFlags,
						   int           *wrSize)
{
	int rt, wr_size = 0, offset = 0;
	unsigned int nand_cmd = NAND_CMD_WRITE;
	unsigned int saved_timing;
	unsigned char *wr_buf;

	*wrSize = 0;

	if (pageNum >= PAGES_OF_CHIP)
	{
    	return -EINVAL;
    }

	if (dataBuf == NULL)
	{
    	return -EFAULT;
    }

	/*
	 * Calculate write parameters
	 */
	NAND_calc_rw_size(bufSize, colAddr, opFlags, &nand_cmd, &offset, &wr_size);

	if (0 == wr_size)
	{
    	return -ENODATA;
    }

    /*
     * 如果传入的 dataBuf 和 bufSize是对齐的, 就不使用内置 dmaBuf
     */
    if ((((unsigned long)dataBuf & 0x0F) == 0) && ((bufSize & 0x03) == 0))
    {
        wr_buf = dataBuf;
    }
    else
    {
        memcpy((void *)pNand->dmaBuf, (void *)dataBuf, wr_size);
        wr_buf = pNand->dmaBuf;
    }

#if USE_DMA0_INTERRUPT
    WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);        /* 清中断标志 */
#endif

	WRITE_REG64(pNand->dmaCtrl, DMA_STOP);  			/* Begin to write work */

    saved_timing = pNand->hwNAND->timing;
	pNand->hwNAND->op_num = wr_size;
	pNand->hwNAND->timing = 0x0206;
#if PARAM_NAND_SIZE
    pNand->hwNAND->param  = (wr_size << NAND_PARAM_OPSCOPE_SHIFT) | pNand->nand_size;
#else
    pNand->hwNAND->param  = (wr_size << NAND_PARAM_OPSCOPE_SHIFT) | NAND_SIZE_4Gb;
#endif

	NAND_send_rw_command(pNand, pageNum, offset, nand_cmd);	    /* Send NAND command */
    NAND_cmd_start(pNand, nand_cmd);

	NAND_dma_start(pNand, wr_buf, wr_size, DMA_WRITE);			/* Start DMA command */

	/*
	 * Wait DMA transfer done.
	 */
#if (!USE_DMA0_INTERRUPT)
	while (READ_REG64(pNand->dmaCtrl) & DMA_START)
        ;

  #if BSP_USE_OS
    SLEEP(1);                   /* RTOS 切换出时间 */
  #else
    delay_us(wr_size / 2);      /* XXX 没有等待DMA完成操作的方法, 延时 */
  #endif
#else
    while (!(READ_REG64(pNand->dmaCtrl) & DMA_STOP))
    {
        delay_us(1);

        if (NAND_check_dma0_irq_occurred(pNand))
            break;
    }
#endif

	*wrSize = bufSize;						    /* Return the accurate write data size */

//	rt = -NAND_waitdone(pNand, 1000);  	        /* delay 1000us */
	rt = -NAND_waitdone(pNand, 5000);  	        /* delay 5000us */
	
	if (0 != rt)
	{
		printk("NAND write timeout.\r\n");
		pNand->writeerr_cnt++;
	}

    pNand->hwNAND->timing = saved_timing;
#if (!USE_DMA0_INTERRUPT)
	WRITE_REG64(pNand->dmaCtrl, DMA_STOP);      /* Stop DMA */
	UNLOCK_DMA;
#endif

	return rt;
}

static int NAND_is_blank_page(NAND_t *pNand, unsigned int pageNum)
{
	unsigned int size = FULL_PAGE_SIZE / 4;
	unsigned int *data = (unsigned int *)pNand->dmaBuf;
	int rt, rdSize;
	
	rt = NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf,
						FULL_PAGE_SIZE, NAND_OP_MAIN | NAND_OP_SPARE, &rdSize);

	if (0 == rt)
	{
		while (size--)
		{
			if ((*data) ^ 0xFFFFFFFF)
			{
				rt = -1;
				break;
			}
			data++;
		}
	}
	else
	{
    	rt = -EIO;
    }

    if (rt < 0)
    {
        return false;
    }

    return true;
}

static int NAND_verify_page(NAND_t *pNand, unsigned int pageNum,
						    unsigned char *buf, unsigned int bufSize)
{
	unsigned char *data = pNand->dmaBuf;
	int rt, rdSize;

	rt = NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf,
						FULL_PAGE_SIZE, NAND_OP_MAIN | NAND_OP_SPARE, &rdSize);

	if (0 == rt)
	{
		while (bufSize--)
		{
			if (*data != *buf)
			{
				rt = -1;
				break;
			}
			data++;
			buf++;
		}
	}
	else
	{
    	rt = -EIO;
    }

	return rt;
}

static int NAND_mark_badblock(NAND_t *pNand, unsigned int blkNum);

static int NAND_erase_block(NAND_t *pNand, unsigned int blkNum, int checkbad)
{
	unsigned int rt=0, pageNum;

	if (blkNum >= BLOCKS_OF_CHIP)
	{
		printk("Try erase blkNum=0x%08X out of range\r\n", blkNum);
		return -EINVAL;
	}

	pageNum = BLOCK_TO_PAGE2K(blkNum);

	pNand->hwNAND->cmd    = 0;
	pNand->hwNAND->op_num = 1;
	pNand->hwNAND->addr_c = 0;
	pNand->hwNAND->addr_r = pageNum | (NAND_CS << 18);
#if PARAM_NAND_SIZE
    pNand->hwNAND->param  = (1 << NAND_PARAM_OPSCOPE_SHIFT) | pNand->nand_size;
#else
    pNand->hwNAND->param  = (1 << NAND_PARAM_OPSCOPE_SHIFT) | NAND_SIZE_4Gb;
#endif

    /*
     * command erase block, 2 step
     */
    pNand->hwNAND->cmd = NAND_CMD_ERASE1;
    pNand->hwNAND->cmd = NAND_CMD_ERASE1 | NAND_CMD_VALID;

    osal_msleep(5);                 /* RTOS 切换出时间 */

	rt = -NAND_waitdone(pNand, 20000);	            /* delay 10000us */
	if (0 == rt)
	{
    	pNand->erase_cnt++;
    	DBG_NAND("NAND block %i erase successful.\r\n", blkNum);

	    /*
         * verify and mark the bad block
         */
    	if (checkbad != 0)
        {
            unsigned char tmpbuf[FULL_PAGE_SIZE];

            DBG_NAND("Checking bad of block %i ...\r\n", blkNum);
            memset(tmpbuf, 0xFF, BYTES_OF_PAGE+OOBBYTES_OF_PAGE);   // fill 0xFF

        	for (pageNum = 0; pageNum < PAGES_OF_BLOCK; pageNum++)
	        {
	            if (NAND_verify_page(pNand,
                                     BLOCK_TO_PAGE2K(blkNum) + pageNum,
                                     tmpbuf,
                                     FULL_PAGE_SIZE) != 0)
                {
                    NAND_mark_badblock(pNand, blkNum);
                    break;
                }
	        }
        }
    }
	else
	{
		printk("NAND erase timeout.\r\n\r\n");
		pNand->eraseerr_cnt++;
	}

	return rt;
}

static int NAND_erase_chip(NAND_t *pNand, int checkbad)
{
	unsigned int blkNum;

	for (blkNum = 0; blkNum < BLOCKS_OF_CHIP; blkNum++)
	{
		if (0 != NAND_erase_block(pNand, blkNum, checkbad))
		{
		    printk("Erase Nand block %i timeout, fail.\r\n", (int)blkNum);
        	return -ETIMEDOUT;
        }
	}

    DBG_NAND("Erase Nand done.\r\n");

	return 0;
}

static int NAND_mark_badblock(NAND_t *pNand, unsigned int blkNum)
{
	unsigned int	pageNum;
	int				rt, wrSize;
	unsigned char	oobbuf[8];

	/* Initialize oob as bad */
	memset(oobbuf, 0xFF, 8);
	oobbuf[0] = 0x01;

	/* 1st page number in this block. */
	pageNum = BLOCK_TO_PAGE2K(blkNum);
	rt = NAND_write_page(pNand, pageNum, 0, oobbuf, 8, NAND_OP_SPARE, &wrSize);
	if (0 != rt)
	{
    	return rt;
    }

	/* 2st page number in this block. */
	pageNum += 1;
	rt = NAND_write_page(pNand, pageNum, 0, oobbuf, 8, NAND_OP_SPARE, &wrSize);

    printk("Mark block %i bad.\r\n", (int)blkNum);

	return rt;
}

static int NAND_is_badblock(NAND_t *pNand, unsigned int blkNum)
{
	unsigned int  pageNum;
	int           rt, rdSize;
	unsigned char ch0, ch1;
    unsigned char *tmpbuf = pNand->dmaBuf;

	pageNum = BLOCK_TO_PAGE2K(blkNum);			/* 1st page number in this block. */

	rt = NAND_read_page(pNand, pageNum, 0, tmpbuf, 8, NAND_OP_SPARE, &rdSize);
	ch0 = tmpbuf[0];						    /* first byte of spare */
	if (0 != rt)
	{
    	return rt;
    }

	pageNum += 1;								/* 2st page number in this block. */
	rt = NAND_read_page(pNand, pageNum, 0, tmpbuf, 8, NAND_OP_SPARE, &rdSize);
	ch1 = tmpbuf[0];						    /* first byte of spare */
	if (0 != rt)
	{
    	return rt;
    }

	if ((ch0 != 0xFF) || (ch1 != 0xFF))			/* judge it is bad... */
	{
    	return true;
    }

	return false;
}

/******************************************************************************
 * Interrupt Implement
 ******************************************************************************/

#if (USE_DMA0_INTERRUPT)
static void DMA0_interrupt_handler(int vector, void *arg)
{
	NAND_t *pNand = (NAND_t *)arg;

	WRITE_REG64(pNand->dmaCtrl, DMA_STOP);			/* Stop DMA */
    UNLOCK_DMA;

	if (pNand->dmaDesc->command & DESC_CMD_R_W)	    /* According dma-desc-cmd to know r/w done */
	{
		pNand->write_cnt++;
		pNand->write_bytes += pNand->dmaDesc->length * 4;
	}
	else
	{
		pNand->read_cnt++;
		pNand->read_bytes += pNand->dmaDesc->length * 4;
	}

    WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);    /* 清中断标志 */

	pNand->intr_cnt++;
}
#endif

/******************************************************************************
 * NAND Driver Implement
 ******************************************************************************/

STATIC_DRV int NAND_initialize(const void *dev, void *arg)
{
    NAND_t *pNand = (NAND_t *)dev;

    if (dev == NULL)
    {
        return -1;
    }

	if (pNand->intialized == 1)
	{
    	return 0;
    }

	memset((void *)pNand, 0, sizeof(NAND_t));

#if PARAM_NAND_SIZE

    unsigned int flash_size = (long)arg;

    switch (flash_size)
    {
        case NAND_FLASH_1Gb: pNand->nand_size = NAND_SIZE_1Gb;   break;
        case NAND_FLASH_2Gb: pNand->nand_size = NAND_SIZE_2Gb;   break;
        case NAND_FLASH_4Gb: pNand->nand_size = NAND_SIZE_4Gb;   break;
        case NAND_FLASH_8Gb: pNand->nand_size = NAND_SIZE_8Gb;   break;
        default:
            printk("NAND flash size: %i is not supported.\r\n", flash_size);
	        return -1;
    }

#endif

    /*
     * Initialize nand and dma
     */
	NAND_hw_initialize(pNand);

#if (USE_DMA0_INTERRUPT)
	/*
	 * Install DMA0 interrupt handler
	 */

    ls2k_install_irq_handler(INTC1_DMA0_IRQ, DMA0_interrupt_handler, (void *)dev);
    ls2k_set_irq_routeip(INTC1_DMA0_IRQ, INT_ROUTE_IP0);

#endif

	pNand->intialized = 1;

    DBG_NAND("NAND controller initialized.\r\n");

	return 0;
}

STATIC_DRV int NAND_open(const void *dev, void *arg)
{
    NAND_t *pNand = (NAND_t *)dev;

    if (pNand == NULL)
    {
        return -1;
    }

#if PARAM_NAND_SIZE

    unsigned int flash_size = (long)arg;

    switch (flash_size)
    {
        case NAND_FLASH_1Gb: pNand->nand_size = NAND_SIZE_1Gb;   break;
        case NAND_FLASH_2Gb: pNand->nand_size = NAND_SIZE_2Gb;   break;
        case NAND_FLASH_4Gb: pNand->nand_size = NAND_SIZE_4Gb;   break;
        case NAND_FLASH_8Gb: pNand->nand_size = NAND_SIZE_8Gb;   break;
    }

#endif

#if USE_DMA0_INTERRUPT
    WRITE_REG32(INTC1_SET_BASE, INTC1_DMA0_BIT);    /* enable interrupt */
#endif

	return 0;
}

STATIC_DRV int NAND_close(const void *dev, void *arg)
{
    if (dev == NULL)
    {
        return -1;
    }

#if USE_DMA0_INTERRUPT
    WRITE_REG32(INTC1_CLR_BASE, INTC1_DMA0_BIT);    /* disable interrupt */
#endif

	return 0;
}

STATIC_DRV int NAND_read(const void *dev, void *buf, int size, void *arg)
{
    int rt, rdSize = 0;
    NAND_t *pNand = (NAND_t *)dev;
	NAND_PARAM_t *rdParam = (NAND_PARAM_t *)arg;

    if ((dev == NULL) || (buf == NULL))
    {
        return -1;
    }

	if (!rdParam)
	{
    	return -EINVAL;
    }

	if (size & 0x03)
	{
    	return -EFAULT;
    }

	/*
	 * Read one page
	 */
	rt = NAND_read_page(pNand,
						rdParam->pageNum,
						rdParam->colAddr,
						(unsigned char *)buf,
						size,
						rdParam->opFlags,
						&rdSize);

    if (0 == rt)
    {
        return rdSize;
    }

	return rt;
}

STATIC_DRV int NAND_write(const void *dev, void *buf, int size, void *arg)
{
    int rt, wrSize = 0;
    NAND_t *pNand = (NAND_t *)dev;
	NAND_PARAM_t *wrParam = (NAND_PARAM_t *)arg;

    if ((dev == NULL) || (buf == NULL))
    {
        return -1;
    }

	if (!wrParam)
	{
    	return -EINVAL;
    }

	if (size & 0x03)
	{
    	return -EFAULT;
    }

	/*
	 * Write one page
	 */
	rt = NAND_write_page(pNand,
						 wrParam->pageNum,
						 wrParam->colAddr,
						 (unsigned char *)buf,
						 size,
						 wrParam->opFlags,
						 &wrSize);

	if (0 == rt)
	{
    	return wrSize;
    }

	return rt;
}

STATIC_DRV int NAND_ioctl(const void *dev, int cmd, void *arg)
{
	int rt = 0;
	NAND_t *pNand = (NAND_t *)dev;

    if (dev == NULL)
    {
        return -1;
    }

	switch (cmd)
	{
		case IOCTL_NAND_RESET:
			NAND_chip_reset(pNand);
			break;

		case IOCTL_NAND_GET_ID:
			rt = NAND_read_id(pNand, (uint64_t *)arg);
			break;

		case IOCTL_NAND_ERASE_BLOCK:
			rt = NAND_erase_block(pNand, (unsigned long)arg, 0);
			break;

		case IOCTL_NAND_ERASE_CHIP:
			rt = NAND_erase_chip(pNand, (unsigned long)arg);
			break;

		case IOCTL_NAND_PAGE_BLANK:
			rt = NAND_is_blank_page(pNand, (unsigned long)arg);
			break;

		case IOCTL_NAND_MARK_BAD_BLOCK:
			rt = NAND_mark_badblock(pNand, (unsigned long)arg);
			break;

		case IOCTL_NAND_IS_BAD_BLOCK:
			rt = NAND_is_badblock(pNand, (unsigned long)arg);
			break;

		case IOCTL_NAND_GET_STATS:
		case IOCTL_NAND_PAGE_VERIFY:
		default:
			rt = -1;    // not implement
			break;
	}

	return rt;
}

/******************************************************************************
 * NAND device.
 */
const void *devNAND = (void *)&ls2k_NAND;

#if (PACK_DRV_OPS)
/******************************************************************************
 * NAND driver operators
 */
static const driver_ops_t ls2k_nand_drv_ops =
{
    .init_entry  = NAND_initialize,
    .open_entry  = NAND_open,
    .close_entry = NAND_close,
    .read_entry  = NAND_read,
    .write_entry = NAND_write,
    .ioctl_entry = NAND_ioctl,
};

const driver_ops_t *nand_drv_ops = &ls2k_nand_drv_ops;
#endif

#endif

