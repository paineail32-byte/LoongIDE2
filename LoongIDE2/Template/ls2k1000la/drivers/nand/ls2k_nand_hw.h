/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_nand_hw.h
 *
 * created: 2022-03-04
 *  author: Bian
 */

#ifndef _LS2K_NAND_HW_H
#define _LS2K_NAND_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// NAND 设备
//-------------------------------------------------------------------------------------------------

/*
 * NAND 控制器
 */
typedef struct
{
	volatile unsigned int cmd;				    // 0x00: NAND_CMD, 命令寄存器
	volatile unsigned int addr_c;			    // 0x04: ADDR_C, 页内偏移地址寄存器
	volatile unsigned int addr_r;		    	// 0x08: ADDR_R, 页地址寄存器
	volatile unsigned int timing;		    	// 0x0C: NAND_TIMING, 时序寄存器
	volatile unsigned int id_l;				    // 0x10: ID_L, ID寄存器
	volatile unsigned int id_h;				    // 0x14: STATUS & ID_H, ID[47:32]和状态寄存器
	volatile unsigned int param;			    // 0x18: NAND_PARAMETER, 参数配置寄存器
	volatile unsigned int op_num;			    // 0x1C: NAND_OP_NUM, 操作数量寄存器 - NAND读写操作Byte数; 擦除为块数
	volatile unsigned int cs_rdy_map;		    // 0x20: CS_RDY_MAP, 映射寄存器
	volatile unsigned int rsv[7];
	volatile unsigned int dma_access;			// 0x40: DMA_ADDRESS, DMA读写数据寄存器
} HW_NAND_t;

/*
 * NAND cmd Register
 */
#define NAND_CMD_DMA_REQ		(1<<31)	        // R/- 非ECC模式下NAND发出DMA请求
#define NAND_CMD_ECC_DMA_REQ	(1<<30)		    // R/- ECC模式下NAND发出DMA请求

#define NAND_CMD_STATUS_MASK    (0x3F<<25)      // R/- 内部状态机
#define NAND_CMD_STATUS_SHIFT   25

#define NAND_CMD_CS_MASK		(0x0F<<20)	    // R/- 外部NAND芯片片选情况, 四位分别对应片外四个片选, XXX =0: 表示选中
#define NAND_CMD_CS_SHIFT		20
#define NAND_CMD_CS_0           (1<<20)
#define NAND_CMD_CS_1           (1<<21)
#define NAND_CMD_CS_2           (1<<22)
#define NAND_CMD_CS_3           (1<<23)

#define NAND_CMD_RDY_MASK		(0x0F<<16)	    // R/- 外部NAND芯片RDY情况, 对应关系和NAND_CE一致, =1: 表示准备好
#define NAND_CMD_RDY_SHIFT		16
#define NAND_CMD_RDY_0          (1<<16)
#define NAND_CMD_RDY_1          (1<<17)
#define NAND_CMD_RDY_2          (1<<18)
#define NAND_CMD_RDY_3          (1<<19)

#define NAND_CMD_WAIT_ECC	    (1<<14) 	    // R/W =1: 表示等待ECC读完成(用于ECC读)
#define NAND_CMD_INT_EN			(1<<13)		    // R/W NAND中断使能信号, =1: 表示使能中断
#define NAND_CMD_ECC_WR			(1<<12)		    // R/W =1: 表示写操作时候ECC功能开启
#define NAND_CMD_ECC_RD			(1<<11)		    // R/W =1: 表示读操作时候ECC功能开启
#define NAND_CMD_DONE			(1<<10)		    // R/W =1: 表示操作完成, 需要软件清零
#define NAND_CMD_SPARE			(1<<9)		    // R/W =1: 表示操作发生在NAND的SPARE区
#define NAND_CMD_MAIN			(1<<8)		    // R/W =1: 表示操作发生在NAND的MAIN区
#define NAND_CMD_RD_SR			(1<<7)		    // R/W =1: 表示读NAND的状态操作
#define NAND_CMD_RESET			(1<<6)		    // R/W =1: 表示Nand复位操作
#define NAND_CMD_RD_ID			(1<<5)		    // R/W =1: 表示读ID操作
#define NAND_CMD_ERASE_BLOCKS	(1<<4)		    // R/W =1: 连续擦除, 擦块的数目由nand_op_num决定
#define NAND_CMD_ERASE1  		(1<<3)		    // R/W =1: 表示擦除操作
#define NAND_CMD_WRITE  		(1<<2)		    // R/W =1: 表示写操作
#define NAND_CMD_READ   		(1<<1)		    // R/W =1: 表示读操作
#define NAND_CMD_VALID			(1<<0)		    // R/W =1: 表示命令有效, 操作完成后硬件自动清零

/*
 * NAND addr_c Register
 *
 * 13:0 Nand_Col R/W 读、写、擦除操作起始地址页内地址(必须以字对齐, 为4的倍数), 和页大小对应关系如下:
 * 512Bytes: 需要填充[8:0]
 *       2K: 需要填充[11:0], [11]表示spare区, [10:0]表示页内偏移地址
 *       4K: 需要填充[12:0], [12]表示spare区, [11:0]表示页内偏移地址
 *       8K: 需要填充[13:0], [13]表示spare区, [12:0]表示页内偏移地址
 */
#define NAND_ADDR_C_2K_BIT      (1<<11)         /* ==1 & nand_cmd_spare==1:  only op spare
											     * ==0 & nand_cmd_spare==0:  only op main
											     * ==X & nand_cmd_spare==X:  op main & spare
											     */
#define NAND_ADDR_C_4K_BIT      (1<<12)
#define NAND_ADDR_C_8K_BIT      (1<<13)

/*
 * NAND addr_r Register
 *
 * 24:0 Nand_Row R/W 读、写、擦除操作起始地址页地址, 地址组成如下: { 片选, 页数 }
 * 其中片选固定为2位, 页数根据实际的单片颗粒容量确定, 如1M页则为[19:0], [21:20]用于选择4片中的第几片
 */

/*
 * NAND timing Register
 */
#define NAND_TIMING_HOLD_CYCLE_MASK  0xFF00     // NAND命令有效需等待的周期数, 缺省4
#define NAND_TIMING_HOLD_CYCLE_SHIFT 8
#define NAND_TIMING_WAIT_CYCLE_MASK  0xFF       // NAND一次读写所需总时钟周期数, 缺省18

/*
 * NAND id_h Register
 */
#define NAND_STATUS_MASK	    0xFF0000		// bit[23:16], NAND设备当前的读写完成状态
#define NAND_STATUS_SHIFT	    16
#define NAND_ID_H_MASK		    0xFFFF			// bit[15:0], ID高16位

/*
 * NAND param Register
 *
 * PMON 配置这个参数为: 0x0800 5 000: 2K+5byte+1GB
 */
#define NAND_PARAM_OPSCOPE_MASK	(0x3FFF<<16)	/* bit[29:16] R/W 每次能操作的范围, 配置如下:
												   1. 操作main区, 配置为一页的main区大小
												   2. 操作spare区, 配置为一页的spare区大小
												   3. 操作main加spare区, 配置为一页的main区加上spare区大小
                                                 */
#define NAND_PARAM_OPSCOPE_SHIFT 16

#define NAND_PARAM_ID_MASK		(0x07<<12)		/* bit[14:12] R/W ID号的字节数 */
#define NAND_PARAM_ID_SHIFT		12

#define NAND_PARAM_SIZE_MASK	(0x0F<<8)		// bit[11:8] 外部颗粒容量大小
#define NAND_PARAM_SIZE_SHIFT	8
#define NAND_SIZE_1Gb			(0x00<<8)		// 0: 1Gb(2K页)
#define NAND_SIZE_2Gb			(0x01<<8)		// 1: 2Gb(2K页)
#define NAND_SIZE_4Gb			(0x02<<8)		// 2: 4Gb(2K页)
#define NAND_SIZE_8Gb			(0x03<<8)		// 3: 8Gb(2K页)
#define NAND_SIZE_16Gb			(0x04<<8)		// 4: 16Gb(4K页)
#define NAND_SIZE_32Gb			(0x05<<8)		// 5: 32Gb(8K页)
#define NAND_SIZE_64Gb			(0x06<<8)		// 6: 64Gb(8K页)
#define NAND_SIZE_128Gb			(0x07<<8)		// 7: 128Gb(8K页)
#if 0
#define NAND_SIZE_64Mb			(0x09<<8)		// 9: 64Mb(512B页)
#define NAND_SIZE_128Mb			(0x0A<<8)		// a:128Mb(512B页)
#define NAND_SIZE_256Mb			(0x0B<<8)		// b:256Mb(512B页)
#define NAND_SIZE_512Mb			(0x0C<<8)		// c:512Mb(512B页)
#define NAND_SIZE_1Gb		 	(0x0D<<8)		// d:1Gb(512B页)
#endif

/*
 * NAND cs_rdy_map Register
 */
#define NAND_RDY3_MASK	        (0x0F<<28)
#define NAND_RDY3_SHIFT         28
#define NAND_CS3_MASK	        (0x0F<<24)
#define NAND_CS3_SHIFT	        24

#define NAND_RDY2_MASK          (0x0F<<20)
#define NAND_RDY2_SHIFT         20
#define NAND_CS2_MASK	        (0x0F<<16)
#define NAND_CS2_SHIFT	        16

#define NAND_RDY1_MASK          (0x0F<<12)
#define NAND_RDY1_SHIFT         12
#define NAND_CS1_MASK	        (0x0F<<8)
#define NAND_CS1_SHIFT	        8

#define NAND_RDY_0		        (1<<0)
#define NAND_RDY_1		        (1<<1)
#define NAND_RDY_2		        (1<<2)
#define NAND_RDY_3	        	(1<<3)

#define NAND_CS_0		        (1<<0)
#define NAND_CS_1	        	(1<<1)
#define NAND_CS_2	        	(1<<2)
#define NAND_CS_3		        (1<<3)

#ifdef __cplusplus
}
#endif

#endif // _LS2K_NAND_HW_H

