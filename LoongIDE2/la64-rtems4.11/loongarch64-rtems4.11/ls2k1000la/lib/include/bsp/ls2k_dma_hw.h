/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_DMA_HW_H_
#define _LS2K_DMA_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOCK_DMA
#define UNLOCK_DMA

//-------------------------------------------------------------------------------------------------

/*
 * DMA 操作方向
 */
#define DMA_READ	0x01					/* DMA 读操作 */
#define DMA_WRITE	0x02					/* DMA 写操作 */

/**
 * DMA命令控制寄存器(dma_order)
 */
#define DMA0_BASE		    0x1fe00c00      /* XXX NAND 使用 */
#define DMA1_BASE			0x1fe00c10
#define DMA2_BASE			0x1fe00c20
#define DMA3_BASE			0x1fe00c30
#define DMA4_BASE			0x1fe00c40

#define DMA_ASK_ADDR_MASK	(~0x3full)		/* bit[63:5] 64位地址高59 */
#define DMA_STOP  			bit(4)			/* 停止DMA操作. DMA控制器完成当前数据读写后停止. */
#define DMA_START  			bit(3)			/* 开始DMA操作. DMA控制器读取描述符地址(ask_addr)后将该位清零. */
#define DMA_ASK_VALID  		bit(2)			/* DMA工作寄存器写回到(ask_addr)所指向的内存, 完成后清零. */
#define DMA_AXI_COHERENT  	bit(1)			/* DMA访问地址一致性使能; 1=uncache, 0=cache */
#define DMA_64BIT  			bit(0)			/* DMA控制器64位地址支持 */

//-------------------------------------------------------------------------------------------------

/*
 * DMA Descriptor
 *
 * 第一个描述符: 地址为 ORDER_ADDR_IN.ask_addr << 6, 就是要求 64 字节对齐.
 *
 */
typedef struct
{
	unsigned int next_desc_lo;			    /* 0x00	下一个描述符地址寄存器 - 要求 16 字节对齐 */
	unsigned int mem_addr_lo;			    /* 0x04	DMA操作的内存地址 */
	unsigned int dev_addr;			    	/* 0x08	设备地址寄存器 */
	unsigned int length;				    /* 0x0C	传输数据长度寄存器 - 单位是 4 字节 */
	unsigned int step_length;			    /* 0x10	数据传输间隔长度寄存器 */
										    /*      说明: 间隔长度说明两块被搬运内存数据块之间的长度,
										            前一个step的结束地址与后一个step的开始地址之间的间隔. */
	unsigned int step_times;			    /* 0x14	数据传输循环次数寄存器
										            说明: 循环次数说明在一次DMA操作中需要搬运的块的数目;
										            如果只想搬运一个连续的数据块,循环次数寄存器的值可以赋值为1. */
	unsigned int command;				    /* 0x18	控制寄存器 */
	unsigned int dummy;
	unsigned int next_desc_hi;              /* 0x20 下一个描述符高位地址寄存器 */
	unsigned int mem_addr_hi;               /* 0x24 内存高位地址寄存器 */
} DMA_DESC_t;

/*
 * DMA_ORDER_ADDR_LOW offset: 0x0
 */
#define DESC_NEXT_DESC_ADDR_LO32 (~1UL)         /* 存储器内部下一描述符地址寄存器(低32位) */
#define DESC_NEXT_DESC_EN        1              /* 1=表示下个描述符有效 */

/*
 * DMA_DADDR offset: 0x8
 */
#define DESC_AC97_WR_EN         (1<<31)         /* AC97写使能，1=写操作, 0=读操作 */
#define DESC_AC97_STEREO        (1<<30)         /* =0: mono; =1: 2 channels stereo */
#define DESC_AC97_WRMODE_MASK	(3<<28)	        /* AC97写模式, =0: 1byte, =1: 2byte, =2: 4byte */
#define DESC_AC97_WRMODE_SHIFT  28
#define DESC_AC97_WRMODE_1b	    0
#define DESC_AC97_WRMODE_2b	    (1<<28)
#define DESC_AC97_WRMODE_4b	    (2<<28)

#define DESC_DADDR_MASK         0x0FFFFFFF      /* bit[27:0] DMA操作的APB设备地址 */

/*
 * DESC_CMD offset: 0x18
 */
/* bit defination */
#define DESC_CMD_ADDR_GEN_MASK  (3<<13)		    /* 源/目的地址生成方式 */
#define DESC_CMD_ADDR_GEN_SHIFT 13
#define DESC_CMD_R_W			(1<<12)			/* DMA操作类型, 1=读ddr写设备; 0=读设备写ddr */

/* write status */
#define DESC_CMD_WR_STATUS  	(0xF<<8)		/* DMA写数据状态 */
#define DESC_CMD_WR_SHIFT       8
#define DESC_CMD_WR_IDLE		0		        /* 写状态正处于空闲状态 */
#define DESC_CMD_WR_DDR_WAIT	(1<<8)		    /* 内存还没准备好响应请求,因此DMA一直在等待内存的响应 */
#define DESC_CMD_WR_DDR			(2<<8)		    /* 内存接收了DMA写请求,但是还没有执行完写操作 */
#define DESC_CMD_WR_DDR_END	    (3<<8)		    /* 内存接收了DMA写请求,并完成写操作;此时DMA处于写内存操作完成状态 */
#define DESC_CMD_WR_DMA_WAIT	(4<<8)		    /* DMA发出将DMA状态寄存器写回内存的请求,等待内存接收请求 */
#define DESC_CMD_WR_DMA			(5<<8)		    /* 内存接收写DMA状态请求,但是操作还未完成 */
#define DESC_CMD_WR_DMA_END		(6<<8)		    /* 内存完成写DMA状态操作 */
#define DESC_CMD_WR_STEP_END	(7<<8)		    /* DMA完成一次length长度的操作(也就是说完成一个step) */

/* read status */
#define DESC_CMD_RD_STATUS	   	(0xF<<4)		/* DMA读数据状态 */
#define DESC_CMD_RD_SHIFT       4
#define DESC_CMD_RD_IDLE		0		        /* 读状态正处于空闲状态 */
#define DESC_CMD_RD_READY		(1<<4)		    /* 接收到开始DMA操作的start信号后,进入准备好状态,开始读描述符 */
#define DESC_CMD_RD_GET_ORDER   (2<<4)		    /* 向内存发出读描述符请求,等待内存应答 */
#define DESC_CMD_RD_ORDER		(3<<4)		    /* 内存接收读描述符请求,正在执行读操作 */
#define DESC_CMD_RD_ORDER_END   (4<<4)		    /* 内存读完DMA描述符 */
#define DESC_CMD_RD_DDR_WAIT	(5<<4)		    /* DMA向内存发出读数据请求,等待内存应答 */
#define DESC_CMD_RD_DDR			(6<<4)		    /* 内存接收DMA读数据请求,正在执行读数据操作 */
#define DESC_CMD_RD_DDR_END		(7<<4)		    /* 内存完成DMA的一次读数据请求 */
#define DESC_CMD_RD_DEV			(8<<4)		    /* DMA进入读设备状态 */
#define DESC_CMD_RD_DEV_END		(9<<4)		    /* 设备返回读数据,结束此次读设备请求 */
#define DESC_CMD_RD_STEP_END	(0xA<<4)		/* 结束一次step操作, step times减1 */

#define DESC_CMD_ALL_TRANS_OVER	   (1<<3)	    /* DMA执行完被配置的所有描述符操作 */
#define DESC_CMD_SINGLE_TRANS_OVER (1<<2)		/* DMA执行完一次描述符操作 */
#define DESC_CMD_INT			   (1<<1)		/* DMA中断信号 */
#define DESC_CMD_INT_EN            1			/* DMA中断是否被允许 */

#ifdef __cplusplus
}
#endif

#endif /* _LS2K_DMA_HW_H_ */


