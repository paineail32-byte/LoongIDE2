/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls1c203_conf_hw.h
 *
 * created: 2025-05-20
 *  author: Bian
 */

#ifndef _LS1C_CONF_HW_H
#define _LS1C_CONF_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// CONF 设备
//-------------------------------------------------------------------------------------------------

#define CONF_BASE		0x0000b000

/*
 * CONF 控制器
 */
typedef struct
{
	volatile unsigned int ien;				/* 0x00 INT_EN 中断使能寄存器 */
	volatile unsigned int rsv0;
	volatile unsigned int iout;				/* 0x08 INT_OUT 中断输出寄存器 */
	volatile unsigned int rsv1;
	volatile unsigned int cpuctrl;			/* 0x10 CpuCtrl CPU 配置 */
	volatile unsigned int rstctrl;			/* 0x14 RstCtrl 复位控制寄存器 */
	volatile unsigned int clkctrl;			/* 0x18 ClkCtrl 时钟控制寄存器 */
	volatile unsigned int srprot;			/* 0x1c SrProt 运行状态及保护寄存器 */
	volatile unsigned int osctrim;			/* 0x20 OscTrim Osc控制寄存器 */
	volatile unsigned int pllctrl;			/* 0x24 PllCtrl PLL控制寄存器 */
} HW_CONF_t;

/**
 * 中断使能寄存器
 */
#define CONF_IEN_ACMP			bit(11)			/* RW ACOMP 中断使能位 */
#define CONF_IEN_CAN			bit(10)			/* RW CAN 中断使能位 */
#define CONF_IEN_DMA3			bit(9)			/* RW DMA 中断使能位 */
#define CONF_IEN_DMA2			bit(8)
#define CONF_IEN_DMA1			bit(7)
#define CONF_IEN_DMA0			bit(6)
#define CONF_IEN_SPI			bit(5)
#define CONF_IEN_FLASH			bit(4)			/* RW Flash 中断使能位 */
#define CONF_IEN_UART0			bit(3)			/* RW UART0 中断使能位 */
#define CONF_IEN_UART1			bit(2)			/* RW UART1 中断使能位 */
#define CONF_IEN_I2C			bit(1)			/* RW I2C 中断使能位 */
#define CONF_IEN_BTIM			bit(0)			/* RW BTIM 中断使能位 */

/**
 * 中断输出寄存器
 */
#define CONF_IOUT_ACMP			bit(11)			/* RO ACOMP 中断 */
#define CONF_IOUT_CAN			bit(10)			/* RO CAN 中断 */
#define CONF_IOUT_DMA3			bit(9)			/* RO DMA 中断 */
#define CONF_IOUT_DMA2			bit(8)
#define CONF_IOUT_DMA1			bit(7)
#define CONF_IOUT_DMA0			bit(6)
#define CONF_IOUT_SPI			bit(5)
#define CONF_IOUT_FLASH			bit(4)			/* RO Flash 中断 */
#define CONF_IOUT_UART0			bit(3)			/* RO UART0 中断 */
#define CONF_IOUT_UART1			bit(2)			/* RO UART1 中断 */
#define CONF_IOUT_I2C			bit(1)			/* RO I2C 中断 */
#define CONF_IOUT_BTIM			bit(0)			/* RO BTIM 中断 */

#define INTC_ACOMP_BIT			CONF_IOUT_ACMP
#define INTC_CAN_BIT			CONF_IOUT_CAN
#define INTC_DMA3_BIT			CONF_IOUT_DMA3
#define INTC_DMA2_BIT			CONF_IOUT_DMA2
#define INTC_DMA1_BIT			CONF_IOUT_DMA1
#define INTC_DMA0_BIT			CONF_IOUT_DMA0
#define INTC_SPI_BIT			CONF_IOUT_SPI
#define INTC_FLASH_BIT			CONF_IOUT_FLASH
#define INTC_UART0_BIT			CONF_IOUT_UART0
#define INTC_UART1_BIT			CONF_IOUT_UART1
#define INTC_I2C_BIT			CONF_IOUT_I2C
#define INTC_BTIM_BIT			CONF_IOUT_BTIM

/**
 * CpuCtrl CPU 配置
 */
#define CPU_TURBOEN				bit(27)			/* RW CPU 加力模式. 1: RAM程序频率提高至 PLL输出频率 */
#define CPU_SPI_START			bit(26)			/* RW SPI 启动速率选择. 发唤醒命令后到 SPI 可用的时间. 0: 0,256us; 1=8us */
#define CPU_IRAM_AS_DRAM		bit(25)			/* RW iram 可复用为dram */
#define CPU_BTIM_STOP			bit(14)			/* RW 基础定时器计数停止 */
#define CPU_GTIM_STOP			bit(13)			/* RW 通用定时器计数停止 */
#define CPU_ATIM_STOP			bit(12)			/* RW 高级定时器计数停止 */
#define CPU_CLK8M_FAIL			bit(11)			/* RO 8M 外部时钟失效 */
#define CPU_CLK8M_SEL			bit(10)			/* RO 8M 时钟选择 */
#define CPU_CLKUP_DLY_MASK		(0x3<<8)		/* RW 高速晶振开启到可以使用的延迟 */
#define CPU_CLKUP_DLY_5140US	(0<<8)			/*    5.14ms */
#define CPU_CLKUP_DLY_480US		(1<<8)			/*    480us */
#define CPU_CLKUP_DLY_1460US	(2<<8)			/*    1.46ms */
#define CPU_CLKUP_DLY_2440US	(3<<8)			/*    2.44ms */
#define CPU_8M_SEL				bit(7)			/* RW OSC 时钟源选择. 0: 内部16M时钟; 1:外部8M时钟 */
#define CPU_OSC8M_EN			bit(6)			/* RW 高速晶体振荡器使能. 仅支持外部有源晶振 */

/**
 * RstCtrl 复位控制寄存器
 */
#define RSTCTRL_RST_CMP			bit(12)			/* RW cmp 复位 */
#define RSTCTRL_RST_CAN			bit(11)			/* RW can 复位 */
#define RSTCTRL_RST_ADC			bit(10)			/* RW adc 复位 */
#define RSTCTRL_RST_CRC			bit(9)			/* RW crc 复位 */
#define RSTCTRL_RST_ATIM		bit(8)			/* RW atim 复位 */
#define RSTCTRL_RST_GTIM		bit(7)			/* RW gtim 复位 */
#define RSTCTRL_RST_BTIM		bit(6)			/* RW btim 复位 */
#define RSTCTRL_RST_I2C			bit(5)			/* RW i2c 复位 */
#define RSTCTRL_RST_UART1		bit(4)			/* RW uart1 复位 */
#define RSTCTRL_RST_UART0		bit(3)			/* RW uart0 复位 */
#define RSTCTRL_RST_DMA			bit(2)			/* RW dma 复位 */
#define RSTCTRL_RST_SPI			bit(1)			/* RW spi 复位 */
#define RSTCTRL_RST_FLASH		bit(0)			/* RW flash 复位 */

/**
 * ClkCtrl 时钟控制寄存器
 */
#define CLK_CG_CMP				bit(12)			/* RW cmp 时钟 */
#define CLK_CG_CAN				bit(11)			/* RW can 时钟 */
#define CLK_CG_ADC				bit(10)			/* RW adc 时钟 */
#define CLK_CG_CRC				bit(9)			/* RW crc 时钟 */
#define CLK_CG_ATIM				bit(8)			/* RW atim 时钟 */
#define CLK_CG_GTIM				bit(7)			/* RW gtim 时钟 */
#define CLK_CG_BTIM				bit(6)			/* RW btim 时钟 */
#define CLK_CG_I2C				bit(5)			/* RW i2c 时钟 */
#define CLK_CG_UART1			bit(4)			/* RW uart1 时钟 */
#define CLK_CG_UART0			bit(3)			/* RW uart0 时钟 */
#define CLK_CG_DMA				bit(2)			/* RW dma 时钟 */
#define CLK_CG_SPI				bit(1)			/* RW spi 时钟 */
#define CLK_CG_FLASH			bit(0)			/* RW flash 时钟 */

/**
 * SrProt 运行状态及保护寄存器
 */
#define SRPROT_ADDR_CHECK_EN	bit(7)			/* RO 非法地址检查中断使能 */
												/* XXX 向该寄存器连续写入0x00 0x5a 0xa5 打开该位写使能 */

#define SRPROT_JTAG_LOCK		bit(5)			/* RO JTAG 锁定 */
#define SRPROT_OTP_LOCK			bit(4)			/* RO OTP 锁定 */
#define SRPROT_CJTAG_SEL		bit(2)			/* RO CJTAG 模式选择 */
#define SRPROT_INSTALL_MODE		bit(1)			/* RO 安装模式 */
#define SRPROT_BOOT_SPI			bit(0)			/* RO 1=SPI 启动 */

/**
 * OscTrim Osc控制寄存器
 */
#define OSC_TRIM_DONE			bit(8)			/* RO TRIM 完成. 1=trim_r可用 */
#define OSC_TRIM_R_MASK			0xFF			/* RO 当前TRIM 校准值 */

/**
 * PllCtrl PLL控制寄存器
 *
 * PLLout = PLLref * M / N;
 *
 *   其中 2.5M <= PLLref <= 25M;
 *       100M <= PLLout <= 200M
 *       M >= 4; N >= 1
 */
#define PLL_SEL					bit(16)			/* RW 系统时钟源选择. 0=OSC; 1=PLL */
#define PLL_M_MASK				(0x3F<<8)		/* RW pll 倍频系数 */
#define PLL_M_SHIFT				8
#define PLL_M_MIN               4
#define PLL_M_MAX               0x3F
#define PLL_N_MASK				(0x0F<<4)		/* RW pll 参考时钟分频系数 */
#define PLL_N_SHIFT				4
#define PLL_N_MIN               1
#define PLL_N_MAX               0x0F
#define PLL_LOCK				bit(2)			/* RO PLL 锁定 */
#define PLL_BYPASS				bit(1)			/* RW PLL BYPASS */
#define PLL_PD					bit(0)			/* RW PLL 关断 */


#ifdef __cplusplus
}
#endif

#endif // _LS1C_CONF_HW_H

