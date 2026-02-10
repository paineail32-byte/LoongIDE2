/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2s_hw.h
 *
 * created: 2024-06-11
 *  author: Bian
 */

#ifndef _LS2K_I2S_HW_H
#define _LS2K_I2S_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// I2S 设备
//-------------------------------------------------------------------------------------------------

#define I2S_BASE		0x16114000

/*
 * I2S 控制器
 */
typedef struct
{
	volatile unsigned int version;			/* 0x0000 R/W I2S 标识寄存器 32'h0 */
	volatile unsigned int config;			/* 0x0004 R/W I2S 配置寄存器 32'h0 */
	volatile unsigned int control;			/* 0x0008 R/W I2S 控制寄存器 32'h0 */
	volatile unsigned int rxdata;			/* 0x000c R/W I2S 接收数据寄存器(用于DMA 接收数据) 32'h0 */
	volatile unsigned int txdata;			/* 0x0010 R/W I2S 发送数据寄存器(用于DMA 发送数据) 32'h0 */
	volatile unsigned int config1;			/* 0xd014 R/W I2S 配置寄存器1 32'h0 */
} HW_I2S_t;

/**
 * I2S 标识寄存器
 */
#define I2S_VER_ADDR_W_MASK			(0x03<<8)	/* bit[9:8] 2'h0 地址总线宽度 */
#define I2S_VER_ADDR_W_SHIFT		8
#define I2S_VER_ADDR_W_8			(0<<8)		/* 00: 地址宽度8 位 */
#define I2S_VER_ADDR_W_16			(1<<8)		/* 01: 地址宽度16 位 */
#define I2S_VER_ADDR_W_32			(2<<8)		/* 10: 地址宽度32 位 */
#define I2S_VER_ADDR_W_64			(3<<8)		/* 11: 地址宽度64 位 */

#define I2S_VER_DATA_W_MASK			(0x03<<4)	/* bit[5:4] 2'h0数据宽度 */
#define I2S_VER_DATA_W_SHIFT		4
#define I2S_VER_DATA_W_8			(0<<4)		/* 00: 数据宽度8 位 */
#define I2S_VER_DATA_W_16			(1<<4)		/* 01: 数据宽度16 位 */
#define I2S_VER_DATA_W_32			(2<<4)		/* 10: 数据宽度32 位 */
#define I2S_VER_DATA_W_64			(3<<4)		/* 11: 数据宽度64 位 */

#define I2S_VERSION_MASK			0x0F		/* bit[3:0] 4'h0 I2S 版本号 */

/**
 * I2S 配置寄存器0
 */
#define I2S_CFG_LR_LEN_MASK			(0xFF<<24)	/* bit[31:24] 'h0 左右声道处理的字长. */
#define I2S_CFG_LR_LEN_SHIFT		24
#define I2S_CFG_TX_RES_DEPTH_MASK	(0xFF<<16)	/* bit[23:16] 'h0 TX 采样深度设置 */
#define I2S_CFG_TX_RES_DEPTH_SHIFT	16			/* IIS 采样数据长度, 有效范围为8-32,如果发送的数据宽度小于采样数据长度,
												 * 则低位补0; 如果发送的数据宽度大于采样数据长度, 则低位忽略.
												 */
#define I2S_CFG_BCLK_RATIO_MASK		(0xFF<<8)	/* bit[15:8] 'h0 位时钟(BCLK)分频系数 */
#define I2S_CFG_BCLK_RATIO_SHIFT	8			/* 位时钟分频系数, 分频数为MCLK 时钟频率除以2x(RATIO+1) */
#define I2S_CFG_RX_RES_DEPTH_MASK	0xFF		/* bit[7:0] 'h0 RX 采样深度设置
 	 	 	 	 	 	 	 	 	 	 	 	 * IIS 采样数据长度, 有效范围为8-32,如果接收到的数据宽度小于采样数据长度,
 	 	 	 	 	 	 	 	 	 	 	 	 * 则低位补0; 如果接收到的数据宽度大于采样数据长度, 则低位忽略.
 	 	 	 	 	 	 	 	 	 	 	 	 */

/**
 * I2S 控制寄存器
 */
#define I2S_CTRL_MCLK_READY		bit(16)		/* R 系统时钟(MCLK)输出稳定标志, 为1 时时钟稳定输出, 为0 时输出时钟不可用 */
#define I2S_CTRL_MASTER			bit(15)		/* 'h0 1: IIS 工作于主模式 */
#define I2S_CTRL_MSB_LSB		bit(14)		/* 'h0 1: 高位在左端 0: 高位在右端 */
#define I2S_CTRL_RX_EN			bit(13)		/* 'h0 控制器接收使能, 为1 时有效, 开始接收数据 */
#define I2S_CTRL_TX_EN			bit(12)		/* 'h0 控制器发送使能, 为1 时有效, 开始发送数据 */
#define I2S_CTRL_RX_DMA_EN		bit(11)		/* 'h0 DMA 接收使能, 为1 时有效 */
#define I2S_CTRL_CLK_READY		bit(8)		/* R 位时钟和声道选择时钟输出稳定标志, 为1 时时钟稳定输出, 为0 时输出时钟不可用 */
#define I2S_CTRL_TX_DMA_EN		bit(7)		/* 'h0 DMA 发送使能, 为1 时有效 */
#define I2S_CTRL_RESETn			bit(4)		/* 'h0 IIS 控制器软复位 */
#define I2S_CTRL_MCLK_EN		bit(3)		/* 'h0 使能时钟输出 */
#define I2S_CTRL_RX_INT_EN		bit(1)		/* 'h0 RX 中断使能, 为1 时使能中断, 为0 时禁止 */
#define I2S_CTRL_TX_INT_EN		bit(0)		/* 'h0 TX 中断使能, 为1 时使能中断, 为0 时禁止 */

#define I2S_RX_EN               (I2S_CTRL_RX_EN | I2S_CTRL_RX_DMA_EN)
#define I2S_TX_EN               (I2S_CTRL_TX_EN | I2S_CTRL_TX_DMA_EN)
#define I2S_EN_RXTX             (I2S_RX_EN | I2S_TX_EN)

/**
 * I2S 配置寄存器1
 */
#define I2S_CFG1_MCLK_RATIO_FRAC_MASK	(0xFFFF<<16)	/* bit[32:16] 'h0 系统时钟(MCLK)分频系数小数部分:
														 * 分频数为总线时钟频率除以系统时钟的小数部分乘以2^16
														 */
#define I2S_CFG1_MCLK_RATIO_FRAC_SHIFT	16
#define I2S_CFG1_MCLK_RATIO_MASK		0xFFFF			/* bit[15:0] 'h0 系统时钟(MCLK)分频系数整数部分: 分频数为总线时钟频率
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * 除以系统时钟向下取整的值, 系统时钟作为Codec 的sysclk
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */

#ifdef __cplusplus
}
#endif

#endif // _LS2K_I2S_HW_H


