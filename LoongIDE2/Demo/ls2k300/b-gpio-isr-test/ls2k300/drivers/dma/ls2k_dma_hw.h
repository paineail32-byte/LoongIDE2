/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dma_hw.h
 *
 * created: 2024-06-11
 *  author: Bian
 */

#ifndef _LS2K_DMA_HW_H
#define _LS2K_DMA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// DMA 设备
//-------------------------------------------------------------------------------------------------

#define DMA_BASE        0x1612c000

#define CHNL_COUNT      8

/*
 * DMA 控制器
 */
typedef struct
{
	volatile unsigned int isr;				/* 0x00 DMA_ISR DMA 中断状态寄存器 */
	volatile unsigned int iclr;				/* 0x04 DMA_IFCR DMA 中断标志清除寄存器 */

	struct
	{
		volatile unsigned int ccr;			/* 0x08 DMA_CCR DMA 通道配置寄存器 */
		volatile unsigned int cndtr;		/* 0x0c DMA_CNDTR DMA 通道传输数量寄存器 */
		volatile unsigned int cpar;			/* 0x10 DMA_CPAR DMA 通道外设地址寄存器 */
		volatile unsigned int cmar;			/* 0x14 DMA_CMAR DMA 通道储存地址寄存器 */
		volatile unsigned int rsv;			// 0x18
	} Channels[CHNL_COUNT];

} HW_DMA_t;


/**
 * DMA 中断状态寄存器(DMA_ISR)
 *
 * 偏移量:  0x00
 */
#define DMA_ISR_TEIF(x)			bit(1<<(4*(x)+3))	/* R 通道x 传输错误标志. 0: 通道x 无传输错误事件; 1: 通道x 有传输错误事件. */
#define DMA_ISR_HTIF(x)			bit(1<<(4*(x)+2))	/* R 通道x 传输过半标志. 0: 通道x 无传输过半事件; 1: 通道x 有传输过半事件.
 	 	 	 	 	 	 	 	 	 	 	 	 	 * 注: 该标志位仅在传输个数为偶数时有效. */
#define DMA_ISR_TCIF(x)			bit(1<<(4*(x)+1))	/* R 通道x 传输完成标志. 0: 通道x 无传输完成事件; 1: 通道x 有传输完成事件. */
#define DMA_ISR_GIF(x)			bit(1<<(4*(x)+0))	/* R 通道x 全局中断标志. 0: 通道x 无传输错误/过半/完成事件; 1: 通道x 有传输错误/过半/完成事件. */

//----------------------
// 移位到右边后的 SR
//----------------------

#define DMA_ISR_TE 			    bit(3)
#define DMA_ISR_HT 			    bit(2)
#define DMA_ISR_TC 		        bit(1)
#define DMA_ISR_G 			    bit(0)

/**
 * DMA 中断标志清除寄存器(DMA_IFCR)
 *
 * 偏移量:  0x04
 */
#define DMA_ICLR_CTEIF(x)		bit(1<<(4*(x)+3))	/* RW 清除通道x 传输错误标志. 0: 无效; 1: 清除DMA_ISR 寄存器中对应的传输错误事件标志. */
#define DMA_ICLR_CHTIF(x)		bit(1<<(4*(x)+2))	/* RW 清除通道x 传输过半标志. 0: 无效; 1: 清除DMA_ISR 寄存器中对应的传输过半事件标志. */
#define DMA_ICLR_CTCIF(x)		bit(1<<(4*(x)+1))	/* RW 清除通道x 传输完成标志. 0: 无效; 1: 清除DMA_ISR 寄存器中对应的传输完成事件标志. */
#define DMA_ICLR_CGIF(x)		bit(1<<(4*(x)+0))	/* RW 清除通道x 全局中断标志. 0: 无效; 1: 清除DMA_ISR 寄存器中对应的传输错误/过半/完成事件标志. */

/**
 * DMA 通道x 配置寄存器(DMA_CCRx)
 *
 * 偏移量:  0x08 + 0x14*x
 */

#define DMA_CCR_MEM2MEM			bit(14)			/* RW 存储器到存储器模式, 该位由软件配置和清除.
												 * 0: 非存储器(设备)到存储器(内存)模式;
												 * 1: 启动存储器(内存)到存储器(内存)模式.
												 */
#define DMA_CCR_PL_MASK			(0x03<<12)		/* RW bit[13:12] 通道优先级, 该位域由软件配置和清除. */
#define DMA_CCR_PL_SHIFT		12
#define DMA_CCR_PL_LOW			0
#define DMA_CCR_PL_MID			1
#define DMA_CCR_PL_HIGH			2
#define DMA_CCR_PL_HIGHEST		3

#define DMA_CCR_MSIZE_MASK		(0x03<<10)		/* RW bit[11:10] 存储器数据宽度, 该位域由软件配置和清除. */
#define DMA_CCR_MSIZE_SHIFT		10
#define DMA_CCR_MSIZE_8b		0
#define DMA_CCR_MSIZE_16b		1
#define DMA_CCR_MSIZE_32b		2

#define DMA_CCR_PSIZE_MASK		(0x03<<8)		/* RW bit[9:8] 外设数据宽度, 该位域由软件配置和清除. */
#define DMA_CCR_PSIZE_SHIFT		8
#define DMA_CCR_PSIZE_8b		0
#define DMA_CCR_PSIZE_16b		1
#define DMA_CCR_PSIZE_32b		2

#define DMA_CCR_MINC			bit(7)			/* RW 存储器地址增量模式, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_PINC			bit(6)			/* RW 外设地址增量模式, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_CIRC			bit(5)			/* RW 循环模式, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_DIR				bit(4)			/* RW 数据传输方向, 该位域由软件配置和清除. 0: 从外设读; 1: 从存储器读. */
#define DMA_CCR_TEIE			bit(3)			/* RW 传输错误中断使能, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_HTIE			bit(2)			/* RW 传输过半中断使能, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_TCIE			bit(1)			/* RW 传输完成中断使能, 该位域由软件配置和清除. 0: 无效; 1: 有效. */
#define DMA_CCR_EN				bit(0)			/* RW 通道开启, 该位域由软件配置和清除. 0: 无效; 1: 有效. */

/**
 * DMA 通道x 传输数量寄存器(DMA_CNDTRx)
 *
 * 偏移量:  0x0c + 0x14 * x
 */
/*
 * 31:0 NDT RW 数据传输个数, 这个寄存器只能通道停用时写入. 通道开启后变为只读, 此时读数为待传输个数.
 * 注: 	DMA 单次传输最大支持范围为4294967295, 当msize/psize 均为0 时, NDT 最大可配置为4294967295;
 * 		当msize/psize 任一为1 时, NDT 最大可配置为2147483647;
 * 		当msize/psize 任一为1 时, NDT 最大可配置为1073741823.
 */

/**
 * DMA 通道x 外设地址寄存器(DMA_CPARx)
 *
 * 偏移量:  0x10 + 0x14 * x
 */
/*
 * 31:0 PADDR RW 外设地址, 外设数据起始地址. 通道开启后变为只读.
 */

/**
 * DMA 通道x 储存器地址寄存器(DMA_CMARx)
 *
 * 偏移量:  0x14 + 0x14 * x
 */
/*
 * 31:0 MADDR RW 存储器地址, 存储器数据起始地址. 通道开启后变为只读.
 */

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/*
 	外设请求的通道映射

	 -------------------------------------------------------
	|  		| 					DMA 通道					|
	|  外设	|-----------------------------------------------|
    |    	| CH0 | CH1 | CH2 | CH3 | CH4 | CH5 | CH6 | CH7 |
    |-------|-----|-----|-----|-----|-----|-----|-----|-----|
	| UART0 | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| UART1 | RX  | TX  | RX* | TX* | RX  | TX  | RX  | TX  |
	| UART2 | RX  | TX  | RX  | TX  | RX* | TX* | RX  | TX  |
	| UART3 | RX  | TX  | RX  | TX  | RX  | TX  | RX* | TX* |
	| UART4 | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| UART5 | RX  | TX  | RX* | TX* | RX  | TX  | RX  | TX  |
	| UART6 | RX  | TX  | RX  | TX  | RX* | TX* | RX  | TX  |
	| UART7 | RX  | TX  | RX  | TX  | RX  | TX  | RX* | TX* |
	| UART8 | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| UART9 | RX  | TX  | RX* | TX* | RX  | TX  | RX  | TX  |
	| I2C0  | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| I2C1  | RX  | TX  | RX* | TX* | RX  | TX  | RX  | TX  |
	| I2C2  | RX  | TX  | RX  | TX  | RX* | TX* | RX  | TX  |
	| I2C3  | RX  | TX  | RX  | TX  | RX  | TX  | RX* | TX* |
	| SPI2  | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| SPI3  | RX  | TX  | RX* | TX* | RX  | TX  | RX  | TX  |
	| I2S   | RX* | TX* | RX  | TX  | RX  | TX  | RX  | TX  |
	| ADC   | RX* | RX  | RX  | RX  | RX  | RX  | RX  | RX  |
	| CAN0  | RX* | RX  | RX  | RX  | RX  | RX  | RX  | RX  |
	| CAN1  | RX  | RX* | RX  | RX  | RX  | RX  | RX  | RX  |
	| CAN2  | RX  | RX  | RX* | RX  | RX  | RX  | RX  | RX  |
	| CAN3  | RX  | RX  | RX  | RX* | RX  | RX  | RX  | RX  |
	| ATIM  | CH1 | CH2 | CH3 | CH4 | COM | UP  | TRG |  -  |
	| GTIM  | CH1 | CH2 | CH3 | CH4 |  -  | UP  | TRG |  -  |
	| BTIM  |  -  |  -  |  -  |  -  |  -  | UP  |  -  |  -  |
     -------------------------------------------------------
*/

#ifdef __cplusplus
}
#endif

#endif // _LS2K_DMA_HW_H

