/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k301.h
 *
 * created: 2024-06-06
 *  author: Bian
 */

#ifndef _LS2K301_H
#define _LS2K301_H

#include "cpu.h"

#define bit(x)                  (1<<(x))

//-------------------------------------------------------------------------------------------------

/**
 * 中断向量类型
 *
 * 参数:    vector  中断编号
 *          arg     安装中断向量时传入的参数
 */
typedef void (*irq_handler_t)(int vector, void *arg);

//-------------------------------------------------------------------------------------------------

/*
 * 使用扩展中断时共有 128 个中断可响应
 *
 * 使用传统中断时共有  64 个中断可响应
 *
 */
#define USE_EXTINT              1           /* 1: 使用扩展中断, 0: 使用传统中断  */

//-------------------------------------------------------------------------------------------------
// 寄存器 Read/Write 操作, 地址转换为 loongarch64 uncached
//-------------------------------------------------------------------------------------------------

/*
 * 8 Bits
 */
#define READ_REG8(addr)         (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG8(addr, v)     (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG8(addr, v)        (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG8(addr, v)       (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 16 Bits
 */
#define READ_REG16(addr)        (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG16(addr, v)    (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG16(addr, v)       (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG16(addr, v)      (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 32 Bits
 */
#define READ_REG32(addr)        (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG32(addr, v)    (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG32(addr, v)       (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG32(addr, v)      (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 64 Bits
 */
#define READ_REG64(addr)        (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG64(addr, v)    (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG64(addr, v)       (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG64(addr, v)      (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)) &= (v))

//-------------------------------------------------------------------------------------------------
// 地址空间
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// 全局控制寄存器
//-------------------------------------------------------------------------------------------------

/**
 * Chip Control0
 */
#define CHIP_CTRL0_BASE                 0x16000100
#define CTRL0_USB_PREFETCH				bit(31)				// RW 0x0 USB接口总线使能读预取
#define CTRL0_USB_FLUSH_WR				bit(30)				// RW 0x0 USB接口总线设置写命令发出后是否清空read buffer
#define CTRL0_USB_STOP_WAW				bit(29)				// RW 0x0 USB接口总线是否允许在上一个写完成前发出写命令
#define CTRL0_USB_STOP_RAW				bit(28)				// RW 0x1 USB接口总线是否允许在上一个写完成前发出读命令
#define CTRL0_UART1_EN_MASK				(0x0F<<24)			// RW 0x0 bit[27:24] UART1对应的UART控制器模式及引脚复用关系
#define CTRL0_UART1_EN_SHIFT			24
#define CTRL0_UART1_8LINES				0
#define CTRL0_UART1_4LINES				(0x02<<24)			// bit[25]==1: uart7; bit[26]==0 4线模式
#define CTRL0_UART1_2LINES				(0x0E<<24)			// bit[25]==1: uart7; bit[26]==1: uart8; bit[27]==1: uart9
#define CTRL0_UART0_EN_MASK				(0x0F<<20)			// RW 0x0 bit[23:20] UART0对应的UART控制器模式及引脚复用关系
#define CTRL0_UART0_EN_SHIFT			20
#define CTRL0_UART0_8LINES				0
#define CTRL0_UART0_4LINES				(0x02<<20)			// bit[21]==1: uart4; bit[22]==0 4线模式
#define CTRL0_UART0_2LINES				(0x0E<<20)			// bit[21]==1: uart4; bit[22]==1: uart5; bit[23]==1: uart6
#define CTRL0_EXTIOINT_EN				bit(19)				// RW 0x0 XXX 扩展中断使能控制位.
															// 		  1: 打开扩展中断(传统/扩展中断同时有效); 0: 关闭扩展中断(仅传统中断有效)
#define CTRL0_RTC_HSPEED				bit(17)				// RW 0x1 RTC计数器值快速访问使能配置位. 1: 开启快速访问; 0: 关闭快速访问.
#define CTRL0_OPT_CTRL_EN				bit(16)				// RW 0x0 OTP模块使能位
#define CTRL0_HPET_INT_SHARE_MASK		(0x0F<<12)			// RW 0x0 bit[15:12] hpet0~3中断输出模式配置:
															//		  0: 单中断模式(3个计数器共享1个中断); 1: 三中断模式(3个计数器3个中断)
#define CTRL0_DDR4_ECC_EN				bit(9)				// RW 0x0 DDR ECC使能位
#define CTRL0_DDR4_SHUT					bit(8)				// RW 0x0 DDR模块关闭控制位
#define CTRL0_DDR4_LPCONF_EN			bit(7)				// RW 0x0 DDR低功耗软件控制使能位
#define CTRL0_DDR4_LPMC_EN				bit(6)				// RW 0x0 DDR低功耗软件控制位
#define CTRL0_DDR4_REGS_DEFAULT			bit(5)				// RW 0x0 窗口不命中处理.
															// 		  0: 关闭访问内存控制器默认路由响应功能,
															// 		  1: 当访问内存控制器所有窗口不命中时, 由内存配置空间默认给出响应, 防止内存访问卡死
#define CTRL0_DDR4_REGS_DISABLE			bit(4)				// RW 0x0 DDR配置空间关闭, 高有效
#define CTRL0_GMAC1_TEST_LPBK			bit(3)				// RW 0x0 GMAC1接口loopback环回测试模式使能: 1: 使能loopback环回测试模式.
#define CTRL0_GMAC0_TEST_LPBK			bit(2)				// RW 0x0 GMAC0接口loopback环回测试模式使能: 1: 使能loopback环回测试模式.
#define CTRL0_GMAC1_MII_SEL				bit(1)				// RW 0x0 GMAC1接口MII模式选择: 1: MII接口模式; 0: RGMII接口模式
#define CTRL0_GMAC0_MII_SEL				bit(0)				// RW 0x0 GMAC0接口MII模式选择: 1: MII接口模式; 0: RGMII接口模式

/**
 * Chip Control1
 */
#define CHIP_CTRL1_BASE   				0x16000104
#define CTRL1_LIO_IO_WIDTH				bit(25)				// RW 0x0 IO空间访问16位数据位宽配置位: 1: 16位模式; 0: 8位模式.
#define CTRL1_LIO_IO_COUNT_INIT_MASK	(0x1F<<20)			// RW 0x0 bit[24:20] IO空间访问延迟初始值
#define CTRL1_LIO_IO_COUNT_INIT_SHIFT	20
#define CTRL1_LIO_ROM_WIDTH				bit(19)				// RW 0x0 ROM空间访问16位数据位宽配置位: 1: 16位模式; 0: 8位模式.
#define CTRL1_LIO_ROM_COUNT_INIT_MASK	(0x1F<<14)			// RW 0x0 bit[18:14] ROM空间访问延迟初始值
#define CTRL1_LIO_ROM_COUNT_INIT_SHIFT	14
#define CTRL1_LIO_CLK_PERIOD_MASK		(0x03<<12)			// RW 0x0 bit[13:12] LIO总线访问单位延迟的时钟周期数.
#define CTRL1_LIO_CLK_PERIOD_SHIFT		12					// 00: 4; 01: 1; 10: 2; 11: 4.
#define CTRL1_LIO_CLK_PERIOD_4			0
#define CTRL1_LIO_CLK_PERIOD_1			(0x1<<12)
#define CTRL1_LIO_CLK_PERIOD_2			(0x2<<12)
#define CTRL1_IODMA_SPARE_MASK			(0xFF<<4)			// RW 0x0 bit[11:4] iodma读操作最大数设置
#define CTRL1_IODMA_SPARE_SHIFT			4
#define CTRL1_USB_FLUSH_IDLE_MASK		0x0F				// RW 0xf bit[3:0] 设置清空write buffer前空闲周期数

/**
 * Chip Control2
 */
#define CHIP_CTRL2_BASE   				0x16000108
#define CTRL2_PMU_DVFS_CLKDIV_MASK		(0x0F<<4)			// RW 0x0 bit[11:4] PLL时钟输出分频参数配置, 最高位CLKDIV[7]为软件参数配置使能位(高有效):
#define CTRL2_PMU_DVFS_CLKDIV_SHIFT		4					//					 对应PLL时钟配置参数CLKDIV[6:0]: 0~127
#define CTRL2_PMU_STPCLK				bit(2)				// RW 0x0 时钟关断配置位, 高电平时钟关断.
#define CTRL2_PMU_DVFS_CLKBYPASS		bit(1)				// RW 0x0 时钟BYPASS配置位: 1: 时钟BYPASS为系统参考时钟; 0: 时钟为PLL输出时钟.
#define CTRL2_PMU_DVFS_EN				bit(0)				// RW 0x0 动态调频调压(DVFS)使能位, 高电平有效.

/**
 * Chip Control3
 */
#define CHIP_CTRL3_BASE   				0x1600010c
#define CTRL3_APB_READ_UPGRADE			bit(29)				// RW 0x0 APB设备DMA内部互联总线读请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_DC_READ_UPGRADE			bit(28)				// RW 0x0 dc内部互联总线读请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_GMAC1_READ_UPGRADE		bit(27)				// RW 0x0 GMAC1内部互联总线读请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_GMAC0_READ_UPGRADE		bit(26)				// RW 0x0 GMAC0内部互联总线读请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_USB_READ_UPGRADE			bit(25)				// RW 0x0 USB0~1内部互联总线读请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_APB_WRITE_UPGRADE			bit(21)				// RW 0x0 APB设备DMA内部互联总线写请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_DC_WRITE_UPGRADE			bit(20)				// RW 0x0 dc内部互联总线写请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_GMAC1_WRITE_UPGRADE		bit(19)				// RW 0x0 GMAC1内部互联总线写请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_GMAC0_WRITE_UPGRADE		bit(18)				// RW 0x0 GMAC0内部互联总线写请求优先级使能配置位(高电平有效, 默认按序访问)
#define CTRL3_USB_WRITE_UPGRADE			bit(17)				// RW 0x0 USB0~1内部互联总线写请求优先级使能配置位(高电平有效, 默认按序访问)
/*
 * 1: 开启CACHE加速访问; 0: 关闭CACHE加速访问 (开启IO设备CACHE使能位后配置有效)
 */
#define CTRL3_APB_COHERENT				bit(13)				// RW 0x0 APB设备DMA内部互联总线CACHE访问配置位
#define CTRL3_DC_COHERENT				bit(12)				// RW 0x0 dc内部互联总线CACHE访问配置位
#define CTRL3_GMAC1_COHERENT			bit(11)				// RW 0x0 GMAC1内部互联总线CACHE访问配置位
#define CTRL3_GMAC0_COHERENT			bit(10)				// RW 0x0 GMAC0内部互联总线CACHE访问配置位
#define CTRL3_USB_COHERENT				bit(9)				// RW 0x0 USB0~1内部互联总线CACHE访问配置位
/*
 * 1: 使能设备CACHE访问配置有效, 配置对应设备coherent位开启CACHE加速访问;
 * 0: 关闭设备CACHE访问配置, 对应设备coherent位配置无效, 此时设备可通过内部总线地址最高位
 * 	  (第32位)选择是否CACHE访问(1:开启, 0:关闭)
 */
#define CTRL3_APB_COHERENT_EN			bit(5)				// RW 0x0 APB设备DMA内部互联CACHE访问使能位
#define CTRL3_DC_COHERENT_EN			bit(4)				// RW 0x0 dc内部互联CACHE访问使能位
#define CTRL3_GMAC1_COHERENT_EN			bit(3)				// RW 0x0 GMAC1内部互联CACHE访问使能位
#define CTRL3_GMAC0_COHERENT_EN			bit(2)				// RW 0x0 GMAC0设备内部互联CACHE访问使能位
#define CTRL3_USB_COHERENT_EN			bit(1)				// RW 0x0 USB0~1设备内部互联CACHE访问使能位:

/**
 * Chip Control4
 */
#define CHIP_CTRL4_BASE   				0x16000110
#define CTRL4_PAD_EMMC_MASK				(0x07<<26)			// RW 0x2 bit[28:26] EMMC PAD驱动类型参数配置
#define CTRL4_PAD_EMMC_SHIFT			26
#define CTRL4_PAD_USB_MASK				(0x03<<24)			// RW 0x1 bit[25:24] USB PAD类驱动类型参数配置
#define CTRL4_PAD_USB_SHIFT				24
#define CTRL4_PAD_TIMER_MASK			(0x03<<22)			// RW 0x1 bit[23:22] TIMER PAD驱动类型参数配置
#define CTRL4_PAD_TIMER_SHIFT			22
#define CTRL4_PAD_I2S_MASK				(0x03<<20)			// RW 0x1 bit[21:20] I2S PAD驱动类型参数配置
#define CTRL4_PAD_I2S_SHIFT				20
#define CTRL4_PAD_SPI_MASK				(0x03<<18)			// RW 0x1 bit[19:18] SPI PAD驱动类型参数配置
#define CTRL4_PAD_SPI_SHIFT				18
#define CTRL4_PAD_SDIO_MASK				(0x03<<16)			// RW 0x2 bit[17:16] SDIO PAD驱动类型参数配置
#define CTRL4_PAD_SDIO_SHIFT			16
#define CTRL4_PAD_GMAC_MASK				(0x03<<14)			// RW 0x2 bit[15:14] GMAC PAD驱动类型参数配置
#define CTRL4_PAD_GMAC_SHIFT			14
#define CTRL4_PAD_UART_MASK				(0x03<<12)			// RW 0x0 bit[13:12] UART PAD驱动类型参数配置
#define CTRL4_PAD_UART_SHIFT			12
#define CTRL4_PAD_DVO_MASK				(0x03<<10)			// RW 0x2 bit[11:10] DVO PAD驱动类型参数配置
#define CTRL4_PAD_DVO_SHIFT				10
#define CTRL4_PAD_JTAG_MASK				(0x03<<8)			// RW 0x0 bit[9:8] JTAG PAD驱动类型参数配置
#define CTRL4_PAD_JTAG_SHIFT			8
#define CTRL4_APB_ORDER_EN				bit(5)				// RW 0x0 APB设备DMA内部互联读写请求按序执行使能位, 高电平有效
#define CTRL4_DC_ORDER_EN				bit(4)				// RW 0x0 DC内部互联读写请求按序执行使能位, 高电平有效
#define CTRL4_GMAC1_ORDER_EN			bit(3)				// RW 0x0 GMAC1内部互联读写请求按序执行使能位, 高电平有效
#define CTRL4_GMAC0_ORDER_EN			bit(2)				// RW 0x0 GMAC0内部互联读写请求按序执行使能位, 高电平有效
#define CTRL4_USB_ORDER_EN				bit(1)				// RW 0x0 USB0~1内部互联读写请求按序执行使能位, 高电平有效
#define CTRL4_CPU_ORDER_EN				bit(0)				// RW 0x0 CPU内部互联读写请求按序执行使能位, 高电平有效

/**
 * Chip Control5
 */
#define CHIP_CTRL5_BASE   				0x16000114
/*
 * 1: 时钟门控打开; 0: 时钟门控关闭.
 */
#define CTRL5_ATIMER_CLK_CTRL			bit(31)				// RW 0x1 atimer模块时钟门控
#define CTRL5_HPET3_CLK_CTRL			bit(30)				// RW 0xf hpet0~3模块时钟门控
#define CTRL5_HPET2_CLK_CTRL			bit(29)
#define CTRL5_HPET1_CLK_CTRL			bit(28)
#define CTRL5_HPET0_CLK_CTRL			bit(27)
#define CTRL5_I2C3_CLK_CTRL				bit(26)				// RW 0xf i2c0~3模块时钟门控
#define CTRL5_I2C2_CLK_CTRL				bit(25)
#define CTRL5_I2C1_CLK_CTRL				bit(24)
#define CTRL5_I2C0_CLK_CTRL				bit(23)
#define CTRL5_CAN3_CLK_CTRL				bit(22)				// RW 0xf can0~3模块时钟门控
#define CTRL5_CAN2_CLK_CTRL				bit(21)
#define CTRL5_CAN1_CLK_CTRL				bit(20)
#define CTRL5_CAN0_CLK_CTRL				bit(19)
#define CTRL5_SPI3_CLK_CTRL				bit(18)				// RW 0x3 spi2~3模块时钟门控
#define CTRL5_SPI2_CLK_CTRL				bit(17)
#define CTRL5_DMA_CLK_CTRL				bit(16)				// RW 0x1 dma模块时钟门控
#define CTRL5_OTP_CLK_CTRL				bit(15)				// RW 0x1 otp模块时钟门控
#define CTRL5_RTC_CLK_CTRL				bit(14)				// RW 0x1 rtc模块时钟门控
#define CTRL5_WDT_CLK_CTRL				bit(13)				// RW 0x1 wdt模块时钟门控
#define CTRL5_ADC_CLK_CTRL				bit(12)				// RW 0x1 adc模块时钟门控
#define CTRL5_I2S_CLK_CTRL				bit(11)				// RW 0x1 i2s模块时钟门控
#define CTRL5_GPIO_CLK_CTRL				bit(10)				// RW 0x1 gpio模块时钟门控
#define CTRL5_OTG_CLK_CTRL				bit(9)				// RW 0x1 usb0(otg)模块时钟门控
#define CTRL5_USB_CLK_CTRL				bit(8)				// RW 0x1 usb1模块时钟门控
#define CTRL5_USBM_CLK_CTRL				bit(7)				// RW 0x1 usb0~1模块时钟门控
#define CTRL5_GMAC1_CLK_CTRL			bit(6)				// RW 0x1 gmac1模块时钟门控
#define CTRL5_GMAC0_CLK_CTRL			bit(5)				// RW 0x1 gmac0模块时钟门控
#define CTRL5_DC_CLK_CTRL				bit(4)				// RW 0x1 dc模块时钟门控
#define CTRL5_LIO_CLK_CTRL				bit(3)				// RW 0x1 lio模块时钟门控
#define CTRL5_SPI1_CLK_CTRL				bit(2)				// RW 0x1 spi1模块时钟门控
#define CTRL5_SPI0_CLK_CTRL				bit(1)				// RW 0x1 spi0模块时钟门控
#define CTRL5_DDR_CLK_CTRL				bit(0)				// RW 0x1 ddr模块时钟门控

/**
 * Chip Control6
 */
#define CHIP_CTRL6_BASE   				0x16000118
/*
 * 1: 时钟门控打开; 0: 时钟门控关闭.
 */
#define CTRL6_APBM_CLK_CTRL				bit(28)				// RW 0x1 apb全部模块时钟门控
#define CTRL6_SDIOM_CLK_CTRL			bit(27)				// RW 0x1 sdio全部模块时钟门控
#define CTRL6_SM4_CLK_CTRL				bit(26)				// RW 0x1 sm4模块时钟门控
#define CTRL6_SM3_CLK_CTRL				bit(25)				// RW 0x1 sm3模块时钟门控
#define CTRL6_DES_CLK_CTRL				bit(24)				// RW 0x1 des模块时钟门控
#define CTRL6_AES_CLK_CTRL				bit(23)				// RW 0x1 aes模块时钟门控
#define CTRL6_RNG_CLK_CTRL				bit(22)				// RW 0x1 rng模块时钟门控
#define CTRL6_SM2_CLK_CTRL				bit(21)				// RW 0x1 sm2模块时钟门控
#define CTRL6_ENCDMA_CLK_CTRL			bit(20)				// RW 0x1 encdma模块时钟门控
#define CTRL6_CANRAM_CLK_CTRL			bit(19)				// RW 0x1 canram模块时钟门控
#define CTRL6_CANBUF3_CLK_CTRL			bit(18)				// RW 0xf canbuf0~3模块时钟门控
#define CTRL6_CANBUF2_CLK_CTRL			bit(17)
#define CTRL6_CANBUF1_CLK_CTRL			bit(16)
#define CTRL6_CANBUF0_CLK_CTRL			bit(15)
#define CTRL6_SDIO1_CLK_CTRL			bit(14)				// RW 0x3 sdio0~1模块时钟门控
#define CTRL6_SDIO0_CLK_CTRL			bit(13)
#define CTRL6_UART9_CLK_CTRL			bit(12)				// RW 0x3ff uart0~9模块时钟门控
#define CTRL6_UART8_CLK_CTRL			bit(11)
#define CTRL6_UART7_CLK_CTRL			bit(10)
#define CTRL6_UART6_CLK_CTRL			bit(9)
#define CTRL6_UART5_CLK_CTRL			bit(8)
#define CTRL6_UART4_CLK_CTRL			bit(7)
#define CTRL6_UART3_CLK_CTRL			bit(6)
#define CTRL6_UART2_CLK_CTRL			bit(5)
#define CTRL6_UART1_CLK_CTRL			bit(4)
#define CTRL6_UART0_CLK_CTRL			bit(3)
#define CTRL6_PWM_CLK_CTRL				bit(2)				// RW 0x1 pwm模块时钟门控
#define CTRL6_BTIMER_CLK_CTRL			bit(1)				// RW 0x1 btimer模块时钟门控
#define CTRL6_GTIMER_CLK_CTRL			bit(0)				// RW 0x1 gtimer模块时钟门控

/**
 * Chip Control7
 */
#define CHIP_CTRL7_BASE   				0x1600011c
/*
 * 1: 软件复位无效; 0: 软件复位有效.
 */
#define CTRL7_ATIMER_RST_CTRL			bit(31)				// RW 0x1 atimer模块软件复位
#define CTRL7_HPET3_RST_CTRL			bit(30)				// RW 0xf hpet0~3模块软件复位
#define CTRL7_HPET2_RST_CTRL			bit(29)
#define CTRL7_HPET1_RST_CTRL			bit(28)
#define CTRL7_HPET0_RST_CTRL			bit(27)
#define CTRL7_I2C3_RST_CTRL				bit(26)				// RW 0xf i2c0~3模块软件复位
#define CTRL7_I2C2_RST_CTRL				bit(25)
#define CTRL7_I2C1_RST_CTRL				bit(24)
#define CTRL7_I2C0_RST_CTRL				bit(23)
#define CTRL7_CAN3_RST_CTRL				bit(22)				// RW 0xf can0~3模块软件复位
#define CTRL7_CAN2_RST_CTRL				bit(21)
#define CTRL7_CAN1_RST_CTRL				bit(20)
#define CTRL7_CAN0_RST_CTRL				bit(19)
#define CTRL7_SPI3_RST_CTRL				bit(18)				// RW 0x3 spi2~3模块软件复位
#define CTRL7_SPI2_RST_CTRL				bit(17)
#define CTRL7_DMA_RST_CTRL				bit(16)				// RW 0x1 dma模块软件复位
#define CTRL7_OTP_RST_CTRL				bit(15)				// RW 0x1 otp模块软件复位
#define CTRL7_RTC_RST_CTRL				bit(14)				// RW 0x1 rtc模块软件复位
#define CTRL7_WDT_RST_CTRL				bit(13)				// RW 0x1 wdt模块软件复位
#define CTRL7_ADC_RST_CTRL				bit(12)				// RW 0x1 adc模块软件复位
#define CTRL7_I2S_RST_CTRL				bit(11)				// RW 0x1 i2s模块软件复位
#define CTRL7_GPIO_RST_CTRL				bit(10)				// RW 0x1 gpio模块软件复位
#define CTRL7_OTG_RST_CTRL				bit(9)				// RW 0x1 usb0(otg)模块软件复位
#define CTRL7_USB_RST_CTRL				bit(8)				// RW 0x1 usb1模块软件复位
#define CTRL7_USBM_RST_CTRL				bit(7)				// RW 0x1 usb全部模块软件复位
#define CTRL7_GMAC1_RST_CTRL			bit(6)				// RW 0x1 gmac1模块软件复位
#define CTRL7_GMAC0_RST_CTRL			bit(5)				// RW 0x1 gmac0模块软件复位
#define CTRL7_DC_RST_CTRL				bit(4)				// RW 0x1 dc模块软件复位
#define CTRL7_LIO_RST_CTRL				bit(3)				// RW 0x1 lio模块软件复位
#define CTRL7_SPI1_RST_CTRL				bit(2)				// RW 0x1 spi1模块软件复位
#define CTRL7_SPI0_RST_CTRL				bit(1)				// RW 0x1 spi0模块软件复位
#define CTRL7_DDR_RST_CTRL				bit(0)				// RW 0x1 ddr模块软件复位

/**
 * Chip Control8
 */
#define CHIP_CTRL8_BASE   				0x16000120
/*
 * 1: 软件复位无效; 0: 软件复位有效.
 */
#define CTRL8_APBM_RST_CTRL				bit(28)				// RW 0x1 apb全部模块软件复位
#define CTRL8_SDIOM_RST_CTRL			bit(27)				// RW 0x1 sdio全部模块软件复位
#define CTRL8_SM4_RST_CTRL				bit(26)				// RW 0x1 sm4模块软件复位
#define CTRL8_SM3_RST_CTRL				bit(25)				// RW 0x1 sm3模块软件复位
#define CTRL8_DES_RST_CTRL				bit(24)				// RW 0x1 des模块软件复位
#define CTRL8_AES_RST_CTRL				bit(23)				// RW 0x1 aes模块软件复位
#define CTRL8_RNG_RST_CTRL				bit(22)				// RW 0x1 rng模块软件复位
#define CTRL8_SM2_RST_CTRL				bit(21)				// RW 0x1 sm2模块软件复位
#define CTRL8_ENCDMA_RST_CTRL			bit(20)				// RW 0x1 encdma模块软件复位
#define CTRL8_CANRAM_RST_CTRL			bit(19)				// RW 0x1 canram模块软件复位
#define CTRL8_CANBUF3_RST_CTRL			bit(18)				// RW 0xf canbuf0~3模块软件复位
#define CTRL8_CANBUF2_RST_CTRL			bit(17)
#define CTRL8_CANBUF1_RST_CTRL			bit(16)
#define CTRL8_CANBUF0_RST_CTRL			bit(15)
#define CTRL8_SDIO1_RST_CTRL			bit(14)				// RW 0x3 sdio0~1模块软件复位
#define CTRL8_SDIO0_RST_CTRL			bit(13)
#define CTRL8_UART9_RST_CTRL			bit(12)				// RW 0x3ff uart0~9模块软件复位
#define CTRL8_UART8_RST_CTRL			bit(11)
#define CTRL8_UART7_RST_CTRL			bit(10)
#define CTRL8_UART6_RST_CTRL			bit(9)
#define CTRL8_UART5_RST_CTRL			bit(8)
#define CTRL8_UART4_RST_CTRL			bit(7)
#define CTRL8_UART3_RST_CTRL			bit(6)
#define CTRL8_UART2_RST_CTRL			bit(5)
#define CTRL8_UART1_RST_CTRL			bit(4)
#define CTRL8_UART0_RST_CTRL			bit(3)
#define CTRL8_PWM_RST_CTRL				bit(2)				// RW 0x1 pwm模块软件复位
#define CTRL8_BTIMER_RST_CTRL			bit(1)				// RW 0x1 btimer模块软件复位
#define CTRL8_GTIMER_RST_CTRL			bit(0)				// RW 0x1 gtimer模块软件复位

/**
 * Chip Control13
 */
#define CHIP_CTRL13_BASE   				0x16000134
/*
 * 00: 通道1、2;  01: 通道3、4;  10: 通道5、6;  11: 通道7、8.
 */
#define CTRL13_UART7_DMA_MAP_MASK		(0x03<<30)			// RW 0x3 bit[31:30] UART7模块DMA通道
#define CTRL13_UART7_DMA_MAP_SHIFT		30
#define CTRL13_UART6_DMA_MAP_MASK		(0x03<<28)			// RW 0x2 bit[29:28] UART6模块DMA通道
#define CTRL13_UART6_DMA_MAP_SHIFT		28
#define CTRL13_UART5_DMA_MAP_MASK		(0x03<<26)			// RW 0x2 bit[27:26] UART5模块DMA通道
#define CTRL13_UART5_DMA_MAP_SHIFT		26
#define CTRL13_UART4_DMA_MAP_MASK		(0x03<<24)			// RW 0x2 bit[25:24] UART4模块DMA通道
#define CTRL13_UART4_DMA_MAP_SHIFT		24
#define CTRL13_UART3_DMA_MAP_MASK		(0x03<<22)			// RW 0x2 bit[23:22] UART3模块DMA通道
#define CTRL13_UART3_DMA_MAP_SHIFT		22
#define CTRL13_UART2_DMA_MAP_MASK		(0x03<<20)			// RW 0x2 bit[21:20] UART2模块DMA通道
#define CTRL13_UART2_DMA_MAP_SHIFT		20
#define CTRL13_UART1_DMA_MAP_MASK		(0x03<<18)			// RW 0x2 bit[19:18] UART1模块DMA通道
#define CTRL13_UART1_DMA_MAP_SHIFT		18
#define CTRL13_UART0_DMA_MAP_MASK		(0x03<<16)			// RW 0x2 bit[17:16] UART0模块DMA通道
#define CTRL13_UART0_DMA_MAP_SHIFT		16
//#define CTRL13_UARTx_DMA_MAP(uart, dma)	((dma)<<((uart)*2+16))	/* uart: 0~7, DMA: 0~3 */
/*
 * 00: 选择加解密DMA通道0; 01: 选择加解密DMA通道1; 其他: 无效
 */
#define CTRL13_SM3_DMA_MAP_MASK			(0x03<<12)			// RW 0x0 bit[13:12] SM3模块数据DMA通道路由
#define CTRL13_SM3_DMA_MAP_SHIFT		12
#define CTRL13_SM4_R_DMA_MAP_MASK		(0x03<<10)			// RW 0x1 bit[11:10] SM4模块读数据DMA通道路由
#define CTRL13_SM4_R_DMA_MAP_SHIFT		10
#define CTRL13_SM4_W_DMA_MAP_MASK		(0x03<<8)			// RW 0x0 bit[9:8] SM4模块写数据DMA通道路由
#define CTRL13_SM4_W_DMA_MAP_SHIFT		8
#define CTRL13_DES_R_DMA_MAP_MASK		(0x03<<6)			// RW 0x1 bit[7:6] DES模块读数据DMA通道路由
#define CTRL13_DES_R_DMA_MAP_SHIFT		6
#define CTRL13_DES_W_DMA_MAP_MASK		(0x03<<4)			// RW 0x0 bit[5:4] DES模块写数据DMA通道路由
#define CTRL13_DES_W_DMA_MAP_SHIFT		4
#define CTRL13_AES_R_DMA_MAP_MASK		(0x03<<2)			// RW 0x1 bit[3:2] AES模块读数据DMA通道路由
#define CTRL13_AES_R_DMA_MAP_SHIFT		2
#define CTRL13_AES_W_DMA_MAP_MASK		(0x03<<0)			// RW 0x0 bit[1:0] AES模块写数据DMA通道路由
#define CTRL13_AES_W_DMA_MAP_SHIFT		0

/**
 * Chip Control14
 */
#define CHIP_CTRL14_BASE   				0x16000138
/*
 * RX 对应的DMA通道为000-111: 通用DMA通道1-8.
 */
#define CTRL14_CAN3_DMA_MAP_MASK		(0x03<<30)			// RW 0x3 bit[31:30] CAN3模块DMA通道映射, XXX 低2位 bit[1:0]
#define CTRL14_CAN3_DMA_MAP_SHIFT		30
#define CTRL14_CAN2_DMA_MAP_MASK		(0x07<<27)			// RW 0x2 bit[29:27] CAN2模块DMA通道映射
#define CTRL14_CAN2_DMA_MAP_SHIFT		27
#define CTRL14_CAN1_DMA_MAP_MASK		(0x07<<24)			// RW 0x1 bit[26:24] CAN1模块DMA通道映射
#define CTRL14_CAN1_DMA_MAP_SHIFT		24
#define CTRL14_CAN0_DMA_MAP_MASK		(0x07<<21)			// RW 0x0 bit[23:21] CAN0模块DMA通道映射
#define CTRL14_CAN0_DMA_MAP_SHIFT		21
#define CTRL14_ADC_DMA_MAP_MASK			(0x07<<18)			// RW 0x0 bit[20:18] ADC模块DMA通道映射
#define CTRL14_ADC_DMA_MAP_SHIFT		18
/*
 * RX、TX对应的通用DMA通道分别为:
 * 00: 通道1、2;  01: 通道3、4; 10: 通道5、6;  11: 通道7、8.
 */
#define CTRL14_I2S_DMA_MAP_MASK			(0x03<<16)			// RW 0x0 bit[17:16] I2S模块DMA通道映射
#define CTRL14_I2S_DMA_MAP_SHIFT		16
#define CTRL14_SPI3_DMA_MAP_MASK		(0x03<<14)			// RW 0x1 bit[15:14] SPI3模块DMA通道映射
#define CTRL14_SPI3_DMA_MAP_SHIFT		14
#define CTRL14_SPI2_DMA_MAP_MASK		(0x03<<12)			// RW 0x0 bit[13:12] SPI2模块DMA通道映射
#define CTRL14_SPI2_DMA_MAP_SHIFT		12
#define CTRL14_I2C3_DMA_MAP_MASK		(0x03<<10)			// RW 0x3 bit[11:10] I2C3模块DMA通道映射
#define CTRL14_I2C3_DMA_MAP_SHIFT		10
#define CTRL14_I2C2_DMA_MAP_MASK		(0x03<<8)			// RW 0x2 bit[9:8] I2C2模块DMA通道映射
#define CTRL14_I2C2_DMA_MAP_SHIFT		8
#define CTRL14_I2C1_DMA_MAP_MASK		(0x03<<6)			// RW 0x1 bit[7:6] I2C1模块DMA通道映射
#define CTRL14_I2C1_DMA_MAP_SHIFT		6
#define CTRL14_I2C0_DMA_MAP_MASK		(0x03<<4)			// RW 0x0 bit[5:4] I2C0模块DMA通道映射
#define CTRL14_I2C0_DMA_MAP_SHIFT		4
#define CTRL14_UART9_DMA_MAP_MASK		(0x03<<2)			// RW 0x3 bit[3:2] UART9模块DMA通道映射
#define CTRL14_UART9_DMA_MAP_SHIFT		2
#define CTRL14_UART8_DMA_MAP_MASK		(0x03<<0)			// RW 0x2 bit[1:0] UART8模块DMA通道映射
#define CTRL14_UART8_DMA_MAP_SHIFT		0

/**
 * Chip Control15
 */
#define CHIP_CTRL15_BASE   				0x1600013c
#define CTRL15_ID_READ_DISABLE			bit(31)				// RW 0x0 ID读使能
#define CTRL15_CAN3_DMA_MAP_HI			bit(0)				// RW 0x0 CAN3模块DMA通道映射, XXX 高1位.

/**
 * Chip Sample0
 */
#define CHIP_SAMP0_BASE					0x16000140
#define SAMP0_EMMC_PADTYPE				bit(6)				// RO 0x0 EMMC0 PAD电平类型. 0:3.3V-IO类型, 1:1.8V-IO类型
#define SAMP0_USB_REFCLKMODE			bit(5)				// RO 0x0 usb参考时钟模式输入.
															//		  0:内部参考时钟输入(时钟频率20MHz)1:外部PAD晶振输入(时钟频率20/24MHz)
#define SAMP0_SDIO1_MODE				bit(4)				// RO 0x0 SDIO1模式配置输入. 0=SDIO模式, 1=EMMC模式
#define SAMP0_SDIO0_MODE				bit(3)				// RO 0x0 SDIO0模式配置输入. 0=SDIO模式, 1=EMMC模式
#define SAMP0_CLK_SEL_MASK				(0x03<<1)			// RO 0x0 bit[2:1] 芯片内部PLL输出时钟上电配置选择
#define SAMP0_CLK_SEL_HW_LO				0					//		  00: 硬件低频时钟配置模式, PLL按照低频配置参数输出时钟;
#define SAMP0_CLK_SEL_HW_HI				(0x01<<1)			//		  01: 硬件高频时钟配置模式, PLL按照高频配置参数输出时钟;
#define SAMP0_CLK_SEL_SW				(0x02<<1)			//		  10: 软件配置模式, PLL按照软件配置选择输出时钟;
#define SAMP0_CLK_SEL_BYPASS			(0x03<<1)			//		  11: 硬件bypass模式, PLL输出时钟全部使用外部输入系统时钟.
#define SAMP0_BOOT_SEL					bit(0)				// RO 0x0 芯片启动选择方式: 0: SPI启动; 1: SDIO启动.

/**
 * Chip Sample2
 */
#define CHIP_SAMP2_BASE					0x16000148
#define SAMP2_DDR4_ECC_ADDR_LO_MASK		0xFFFF0000			// RO DDR bit[31:16] ECC错误地址[15:0]
#define SAMP2_DDR4_ECC_ADDR_LO_SHIFT	16
#define SAMP2_DDR4_ECC_COUNT_MASK		0xFFFF				// RO DDR bit[15:0] ECC错误计数

/**
 * Chip Sample3
 */
#define CHIP_SAMP3_BASE					0x1600014c
#define SAMP3_DDR4_ECC_ADDR_HI_MASK		0xFFFF				// RO DDR bit[15:0] ECC错误地址[31:16]

#define	DDR_ECC_ERR_ADDR				((READ_REG32(CHIP_SAMP2_BASE)>>16) | (READ_REG32(CHIP_SAMP3_BASE)<<16))
#define	DDR_ECC_ERR_COUNT				(READ_REG32(CHIP_SAMP2_BASE) & SAMP2_DDR4_ECC_COUNT_MASK)

/**
 * Chip Counter0
 */
#define CHIP_HPT_LO_BASE				0x16000150			// RW 0x0 64位高精度时钟计数器低32位

/**
 * Chip Counter1
 */
#define CHIP_HPT_HI_BASE				0x16000154			// RW 0x0 64位高精度时钟计数器高32位

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * NODE PLL 时钟配置寄存器0
 */
#define NODE_PLL0_BASE					0x16000400
#define NODE_PLL0_ODIV_MASK				(0x7F<<24)			// RW 0x0 bit[30:24] NODE PLL分频系数配置: 0~127
#define NODE_PLL0_ODIV_SHIFT			24
#define NODE_PLL0_LOOPC_MASK			(0x1FF<<15)			// RW 0x0 bit[23:15] PLL倍频系数: 0~511
#define NODE_PLL0_LOOPC_SHIFT			15
#define NODE_PLL0_REFC_MASK				(0x7F<<8)			// RW 0x0 bit[14:8] PLL参考时钟分频系数: 0~127
#define NODE_PLL0_REFC_SHIFT			8
#define NODE_PLL0_LOCKED				bit(7)				// RO 0x0 PLL锁定标志, 1代表锁定
#define NODE_PLL0_PD					bit(5)				// RW 0x0 PLL关电控制, 1代表关电
#define NODE_PLL0_BYPASS				bit(4)				// RW 0x0 PLL时钟bypass控制, 1代表bypass
#define NODE_PLL0_SOFT_SET				bit(3)				// RW 0x0 允许软件设置PLL, 1代表允许软件配置
#define NODE_PLL0_SEL_I2S				bit(2)				// RW 0x0 I2S选择PLL时钟输出配置, 1代表选择PLL时钟输出
#define NODE_PLL0_SEL_GMAC				bit(1)				// RW 0x0 GMAC选择PLL时钟输出配置, 1代表选择PLL时钟输出
#define NODE_PLL0_SEL_NODE				bit(0)				// RW 0x0 NODE选择PLL时钟输出配置, 1代表选择PLL时钟输出

/**
 * NODE PLL 时钟配置寄存器1
 */
#define NODE_PLL1_BASE					0x16000404
#define NODE_PLL1_LDO_MASK				(0x07<<22)			// RW 0x3 bit[24:22] PLL LDO控制配置, 保持缺省值
#define NODE_PLL1_LDO_SHIFT				22
#define NODE_PLL1_LDO_BYPASS			bit(21)				// RW 0x0 PLL LDO BYPASS配置, 保持缺省值
#define NODE_PLL1_LDO_EN				bit(20)				// RW 0x1 PLL LDO 使能配置, 保持缺省值
#define NODE_PLL1_ODIV_I2S_MASK			(0x7F<<8)			// RW 0x0 bit[14:8] I2S PLL分频系数配置: 0~127
#define NODE_PLL1_ODIV_I2S_SHIFT		8
#define NODE_PLL1_ODIV_GMAC_MASK		(0x7F<<0)			// RW 0x0 bit[6:0] GMAC PLL分频系数配置: 0~127

/**
 * DDR PLL 时钟配置寄存器0
 */
#define DDR_PLL0_BASE					0x16000408
#define DDR_PLL0_ODIV_MASK				(0x7F<<24)			// RW 0x0 bit[30:24] DDR PLL分频系数配置: 0~127
#define DDR_PLL0_ODIV_SHIFT				24
#define DDR_PLL0_LOOPC_MASK				(0x1FF<<15)			// RW 0x0 bit[23:15] PLL倍频系数: 0~511
#define DDR_PLL0_LOOPC_SHIFT			15
#define DDR_PLL0_REFC_MASK				(0x7F<<8)			// RW 0x0 bit[14:8] PLL参考时钟分频系数: 0~127
#define DDR_PLL0_REFC_SHIFT				8
#define DDR_PLL0_LOCKED					bit(7)				// RO 0x0 PLL锁定标志, 1代表锁定
#define DDR_PLL0_PD						bit(5)				// RW 0x0 PLL关电控制, 1代表关电
#define DDR_PLL0_BYPASS					bit(4)				// RW 0x0 PLL时钟bypass控制, 1代表bypass
#define DDR_PLL0_SOFT_SET				bit(3)				// RW 0x0 允许软件设置PLL, 1代表允许软件配置
#define DDR_PLL0_SEL_DEV				bit(2)				// RW 0x0 DEVICE选择PLL时钟输出配置, 1代表选择PLL时钟输出
#define DDR_PLL0_SEL_NETWORK			bit(1)				// RW 0x0 NETWORK选择PLL时钟输出配置, 1代表选择PLL时钟输出
#define DDR_PLL0_SEL_DDR				bit(0)				// RW 0x0 DDR选择PLL时钟输出配置, 1代表选择PLL时钟输出

/**
 * DDR PLL 时钟配置寄存器1
 */
#define DDR_PLL1_BASE					0x1600040c
#define DDR_PLL1_LDO_MASK				(0x07<<22)			// RW 0x3 bit[24:22] PLL LDO控制配置, 保持缺省值
#define DDR_PLL1_LDO_SHIFT				22
#define DDR_PLL1_LDO_BYPASS				bit(21)				// RW 0x0 PLL LDO BYPASS配置, 保持缺省值
#define DDR_PLL1_LDO_EN					bit(20)				// RW 0x1 PLL LDO 使能配置, 保持缺省值
#define DDR_PLL1_MEMDIV_MODE_MASK		(0x03<<18)			// RW 0x1 bit[19:18] DDR 输出时钟分频模式配置
#define DDR_PLL1_MEMDIV_MODE_SHIFT		18
#define DDR_PLL1_SOFT_MC_RSTn			bit(17)				// RW 0x1 DDR控制器软复位控制
#define DDR_PLL1_MEMDIV_RSTn			bit(16)				// RW 0x1 DDR分频复位控制
#define DDR_PLL1_ODIV_DEV_MASK			(0x7F<<8)			// RW 0x0 bit[14:8] I2S PLL分频系数配置: 0~127
#define DDR_PLL1_ODIV_DEV_SHIFT			8
#define DDR_PLL1_ODIV_NETWORK_MASK		(0x7F<<0)			// RW 0x0 bit[6:0] NETWORK PLL分频系数配置: 0~127

/**
 * PIX PLL 时钟配置寄存器0
 */
#define PIX_PLL0_BASE					0x16000410
#define PIX_PLL0_ODIV_MASK				(0x7F<<24)			// RW 0x0 bit[30:24] PIX PLL分频系数配置: 0~127
#define PIX_PLL0_ODIV_SHIFT				24
#define PIX_PLL0_LOOPC_MASK				(0x1FF<<15)			// RW 0x0 bit[23:15] PLL倍频系数: 0~511
#define PIX_PLL0_LOOPC_SHIFT			15
#define PIX_PLL0_REFC_MASK				(0x7F<<8)			// RW 0x0 bit[14:8] PLL参考时钟分频系数: 0~127
#define PIX_PLL0_REFC_SHIFT				8
#define PIX_PLL0_LOCKED					bit(7)				// RO 0x0 PLL锁定标志, 1代表锁定
#define PIX_PLL0_PD						bit(5)				// RW 0x0 PLL关电控制, 1代表关电
#define PIX_PLL0_BYPASS					bit(4)				// RW 0x0 PLL时钟bypass控制, 1代表bypass
#define PIX_PLL0_SOFT_SET				bit(3)				// RW 0x0 允许软件设置PLL, 1代表允许软件配置
#define PIX_PLL0_SEL_GMACBP				bit(1)				// RW 0x0 GMAC-BACKUP选择PLL时钟输出配置, 1代表选择PLL时钟输出
#define PIX_PLL0_SEL_PIX				bit(0)				// RW 0x0 PIX显示选择PLL时钟输出配置, 1代表选择PLL时钟输出

/**
 * PIX PLL 时钟配置寄存器1
 */
#define PIX_PLL1_BASE					0x16000414
#define PIX_PLL1_LDO_MASK				(0x7F<<22)			// RW 0x3 bit[24:22] PLL LDO控制配置, 保持缺省值
#define PIX_PLL1_LDO_SHIFT				22
#define PIX_PLL1_LDO_BYPASS				bit(21)				// RW 0x0 PLL LDO BYPASS配置, 保持缺省值
#define PIX_PLL1_LDO_EN					bit(20)				// RW 0x1 PLL LDO 使能配置, 保持缺省值
#define PIX_PLL1_ODIV_GMACBP_MASK		(0x7F<<0)			// RW 0x0 bit[6:0] GMAC-BACKIP PLL分频系数配置: 0~127

/**
 * 设备时钟分频配置寄存器
 *
 * 设备时钟分频配置寄存器, 分别对应两种分频模式(除SDIO 分频配置外):
 * (1) freq_mode=0, 设备时钟分频计算公式为: fout=fin*(freqscale[2:0]+1)/8;
 * (2) freq_mode=1, 设备时钟分频计算公式为: fout=fin/(freqscale[2:0]+1).
 *
 */
#define FREQSCALE_BASE					0x16000420
#define FREQSCALE_SDIO_MASK				(0x0F<<24)			// RW 0x1 bit[27:24] SDIO时钟分配系数(CLK/freqscale[27:24]):
#define FREQSCALE_SDIO_SHIFT			24					// 		  -对应SDIO时钟输出分频系数: 0~15(0/1均不分频)
#define FREQSCALE_I2S_MODE				bit(23)				// RW     i2s_freqscale[3]:JBIG时钟分配模式freq_mode;
#define FREQSCALE_I2S_MASK				(0x07<<20)			// RW 0x7 bit[22:20] SDIO时钟输出分频系数: 0~7
#define FREQSCALE_I2S_SHIFT				20
#define FREQSCALE_APB_MODE				bit(19)				// RW     apb_freqscale[3]:APB时钟分配模式freq_mode;
#define FREQSCALE_APB_MASK				(0x07<<16)			// RW 0x7 bit[18:16] APB时钟输出分频系数: 0~7
#define FREQSCALE_APB_SHIFT				16
#define FREQSCALE_USB_MODE				bit(15)				// RW     APB时钟分配模式freq_mode;
#define FREQSCALE_USB_MASK				(0x07<<12)			// RW 0x7 bit[14:12] APB时钟输出分频系数: 0~7
#define FREQSCALE_USB_SHIFT				12
#define FREQSCALE_BOOT_MODE				bit(11)				// RW	  BOOT时钟分配模式freq_mode;
#define FREQSCALE_BOOT_MASK				(0x07<<8)			// RW 0x7 bit[10:8] BOOT时钟输出分频系数: 0~7
#define FREQSCALE_BOOT_SHIFT			8
#define FREQSCALE_PIX_MODE				bit(7)				// RW     USB时钟分配模式freq_mode;
#define FREQSCALE_PIX_MASK				(0x07<<4)			// RW 0x7 bit[6:4] USB时钟输出分频系数: 0~7
#define FREQSCALE_PIX_SHIFT				4
#define FREQSCALE_NODE_MODE				bit(3)				// RW     NODE时钟分配模式freq_mode;
#define FREQSCALE_NODE_MASK				0x07				// RW 0x7 bit[3:0] NODE时钟输出分频系数: 0~7

/**
 * 设备时钟输出使能配置寄存器
 */
#define DEV_CLKEN_BASE					0x16000424
#define DEV_CLKEN_GMAC					bit(8)				// RW 0x1 gmac模块时钟源选择配置: 1: 选择SOC-PLL输出时钟; 0: 选择GMACBP-PLL备份输出时钟
#define DEV_CLKEN_PIX					bit(6)				// RW 0x1 pix显示模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_I2S					bit(5)				// RW 0x1 i2s模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_SDIO					bit(4)				// RW 0x1 sdio模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_APB					bit(3)				// RW 0x1 boot模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_USB					bit(2)				// RW 0x1 usb模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_BOOT					bit(1)				// RW 0x1 boot模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭
#define DEV_CLKEN_NODE					bit(0)				// RW 0x1 node模块时钟输出使能配置: 1: 时钟输出使能; 0: 时钟输出关闭

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * GPIO 复用配置寄存器
 *
 * 00: 复用为GPIO; 01: 第一复用; 10: 第二复用;  11: 引脚主功能.
 *
 */
#define GPIO_MUX_BASE                   0x16000490
#define GPIO_MUX_ADDR(n)                (GPIO_MUX_BASE+(n)*4)

#define GPIO_MUX_0_15					0x16000490			// GPIO0~15复用配置寄存器
#define GPIO_MUX_16_31					0x16000494			// GPIO16~31复用配置寄存器
#define GPIO_MUX_32_47					0x16000498			// GPIO32~47复用配置寄存器
#define GPIO_MUX_48_63					0x1600049c			// GPIO48~63复用配置寄存器
#define GPIO_MUX_64_79					0x160004a0			// GPIO64~79复用配置寄存器
#define GPIO_MUX_80_95					0x160004a4			// GPIO80~95复用配置寄存器
#define GPIO_MUX_96_111					0x160004a8			// GPIO96~111复用配置寄存器

/**
 * USB PHY 配置寄存器
 */
#define USB_PHY_CFG0_BASE				0x16000500			// USB PHY 配置寄存器0, 配置USB 接口0的电气特性, 可考USB-PHY 手册寄存器描述.
#define USB_PHY_CFG1_BASE				0x16000504			// USB PHY 配置寄存器0, 配置USB 接口1的电气特性, 可考USB-PHY 手册寄存器描述.

#define USB_PHY_CFG2_BASE				0x16000508
#define USB_CFG2_OTG_SUSPEND_CONFIG		bit(31)				// R/W 0 OTG端口挂起软件配置位, 高有效
#define USB_CFG2_USB_SUSPEND_CONFIG		bit(30)				// R/W 0 USB端口挂起软件配置位, 高有效
#define USB_CFG2_OTG_SUSPEND_SOFTEN		bit(29)				// R/W 1 OTG端口挂起软件配置使能位, 高有效
#define USB_CFG2_USB_SUSPEND_SOFTEN		bit(28)				// R/W 1 USB端口挂起软件配置使能位, 高有效
#define USB_CFG2_PHY_POR				bit(27)				// R/W 0 USB端口复位控制位, 高有效
#define USB_CFG2_FS_DATA_MODE			bit(18)				// R/W 0 FS数据模式
#define USB_CFG2_OTG_PULLDOWN_SOFTEN	bit(17)				// R/W 0 OTG端口下拉模式软件配置使能. 0- 关闭; 1- 使能.
#define USB_CFG2_OTG_OPMODE_SOFTEN		bit(16)				// R/W 0 OTG端口操作模式软件配置使能. 0- 关闭; 1- 使能.
#define USB_CFG2_OTG_OPMODE0_MASK		(0x03<<14)			// R/W 0 bit[15: 14] OTG端口操作模式配置
#define USB_CFG2_OTG_OPMODE0_SHIFT		14
#define USB_CFG2_DP1_PULLDOWN			bit(11)				// R/W 0 USB端口dp端口下拉电阻使能. 1: D+使能; 0: D+关闭
#define USB_CFG2_DM1_PULLDOWN			bit(10)				// R/W 0 USB端口dm端口下拉电阻使能. 1: D-使能; 0: D-关闭
#define USB_CFG2_DP0_PULLDOWN			bit(9)				// R/W 0 OTG端口dp端口下拉电阻使能. 1: D+使能; 0: D+关闭
#define USB_CFG2_DM0_PULLDOWN			bit(8)				// R/W 0 OTG端口dm端口下拉电阻使能. 1: D-使能; 0: D-关闭
#define USB_CFG2_PHY_CLKSEL				bit(3)				// R/W 1 USB端口参考时钟模式选择位: 0-24MHz参考时钟; 1-20MHz参考时钟.
#define USB_CFG2_USB_RESETn				bit(1)				// R/W 0 USB端口复位控制位: 0- 复位有效; 1- 复位撤销.
#define USB_CFG2_OTG_RESETn				bit(0)				// R/W 0 OTG端口复位控制位: 0- 复位有效; 1- 复位撤销.

//-------------------------------------------------------------------------------------------------
// 中断配置及路由
//-------------------------------------------------------------------------------------------------

#if !USE_EXTINT

#define CORE_IPISR 						0x16001000			// RO NA 处理器核的IPI_Status 寄存器
#define CORE_IPIEN 						0x16001004			// RW 0x0 处理器核的IPI_Enalbe 寄存器
#define CORE_IPISET 					0x16001008			// WO NA 处理器核的IPI_Set 寄存器
#define CORE_IPI_CLR 					0x1600100c			// WO NA 处理器核的IPI_Clear 寄存器
#define CORE_INTISR0 					0x16001040			// RO NA 路由给CORE 的低32 位中断状态
#define CORE_INTISR1					0x16001048			// RO NA 路由给CORE 的高32 位中断状态

#define INTC_CORE_ISR0					CORE_INTISR0		// CORE_INTISR0 路由给CORE的低32位中断状态
#define INTC_ISR0						0x16001044			// INTISR0 低32位中断状态寄存器
#define INTC_CORE_ISR1					CORE_INTISR1		// CORE_INTISR1 路由给CORE的高32位中断状态
#define INTC_ISR1						0x1600104c			// INTISR1 高32位中断状态寄存器

#define INTC_ENTRY_0_7					0x16001400			// ENTRY0_0 8位中断路由寄存器[0--7]
#define INTC_ENTRY_8_15					0x16001408			// ENTRY8_0 8位中断路由寄存器[8--15]
#define INTC_ENTRY_16_23				0x16001410			// ENTRY16_0 8位中断路由寄存器[16--23]
#define INTC_ENTRY_24_31				0x16001418			// ENTRY24_0 8位中断路由寄存器[24--31]

#define INTC0_SR_BASE					0x16001420			// INTISR_0 低32位中断状态寄存器
#define INTC0_EN_BASE					0x16001424			// INTIEN_0 低32位中断使能状态寄存器
#define INTC0_SET_BASE					0x16001428			// INTSET_0 低32位设置使能寄存器
#define INTC0_CLR_BASE					0x1600142c			// INTCLR_0 低32位中断清除寄存器, 清除使能寄存器和脉冲触发的中断
#define INTC0_POL_BASE					0x16001430			// INTPOL_0 低32位极性设置寄存器(电平中断)
#define INTC0_EDGE_BASE					0x16001434			// INTEDGE_0 低32位触发方式寄存器(1: 脉冲触发; 0: 电平触发)

#define INTC_ENTRY_32_39				0x16001440			// ENTRY0_1 8位中断路由寄存器[32--39]
#define INTC_ENTRY_40_47				0x16001448			// ENTRY8_1 8位中断路由寄存器[40--47]
#define INTC_ENTRY_48_55				0x16001450			// ENTRY16_1 8位中断路由寄存器[48--55]
#define INTC_ENTRY_56_63				0x16001458			// ENTRY24_1 8位中断路由寄存器[56--63]

#define INTC1_SR_BASE					0x16001460			// INTISR_1 高32位中断状态寄存器
#define INTC1_EN_BASE					0x16001464			// INTIEN_1 高32位中断使能状态寄存器
#define INTC1_SET_BASE					0x16001468			// INTSET_1 高32位设置使能寄存器
#define INTC1_CLR_BASE					0x1600146c			// INTCLR_1 高32位中断清除寄存器, 清除使能寄存器和脉冲触发的中断
#define INTC1_POL_BASE					0x16001470			// INTPOL_1 高32位极性设置寄存器(电平中断)
#define INTC1_EDGE_BASE					0x16001474			// INTEDGE_1 高32位触发方式寄存器(1: 脉冲触发; 0: 电平触发)

/*
 * 中断寄存器索引(n>=0 && n<=1)
 */
#define INTC_SR(n)                      (INTC0_SR_BASE  + n*0x40)
#define INTC_EN(n)                      (INTC0_EN_BASE  + n*0x40)
#define INTC_SET(n)                     (INTC0_SET_BASE + n*0x40)
#define INTC_CLR(n)                     (INTC0_CLR_BASE + n*0x40)
#define INTC_POL(n)                     (INTC0_POL_BASE + n*0x40)
#define INTC_EDGE(n)                    (INTC0_EDGE_BASE+ n*0x40)

/**
 * 传统中断位
 */
#define INTC0_UART0_BIT					bit(0)
#define INTC0_UART1_BIT					bit(1)
#define INTC0_UART_2_5_BIT				bit(2)
#define INTC0_UART_6_9_BIT				bit(3)
#define INTC0_I2C_0_1_BIT				bit(4)
#define INTC0_I2C_2_3_BIT				bit(5)
#define INTC0_SPI2_BIT					bit(6)
#define INTC0_SPI3_BIT					bit(7)
#define INTC0_CAN0_BIT					bit(8)
#define INTC0_CAN1_BIT					bit(9)
#define INTC0_CAN2_BIT					bit(10)
#define INTC0_CAN3_BIT					bit(11)
#define INTC0_I2S_BIT					bit(12)
#define INTC0_ATIMER_BIT				bit(13)
#define INTC0_GTIMER_BIT				bit(14)
#define INTC0_BTIMER_BIT				bit(15)
#define INTC0_PWM_0_1_BIT				bit(16)
#define INTC0_PWM_2_3_BIT				bit(17)
#define INTC0_ADC_BIT					bit(18)
#define INTC0_HPET0_BIT					bit(19)
#define INTC0_HPET1_BIT					bit(20)
#define INTC0_HPET2_BIT					bit(21)
#define INTC0_HPET3_BIT					bit(22)
#define INTC0_DMA0_BIT					bit(23)
#define INTC0_DMA1_BIT					bit(24)
#define INTC0_DMA2_BIT					bit(25)
#define INTC0_DMA3_BIT					bit(26)
#define INTC0_DMA4_BIT					bit(27)
#define INTC0_DMA5_BIT					bit(28)
#define INTC0_DMA6_BIT					bit(29)
#define INTC0_DMA7_BIT					bit(30)
#define INTC0_SDIO0_BIT					bit(31)

#define INTC1_SDIO1_BIT					bit(0)
#define INTC1_SDIO0_DMA_BIT				bit(1)
#define INTC1_SDIO1_DMA_BIT				bit(2)
#define INTC1_ENCRYPT_DMA_BIT			bit(3)
#define INTC1_AES_BIT					bit(4)
#define INTC1_DES_BIT					bit(5)
#define INTC1_SM3_BIT					bit(6)
#define INTC1_SM4_BIT					bit(7)
#define INTC1_RTC_BIT					bit(8)
#define INTC1_TOY_BIT					bit(9)
#define INTC1_RTC_TICK_BIT				bit(10)
#define INTC1_TOY_TICK_BIT				bit(11)
#define INTC1_SPI0_BIT					bit(12)
#define INTC1_SPI1_BIT					bit(13)
#define INTC1_EHCI_BIT					bit(14)
#define INTC1_OHCI_BIT					bit(15)
#define INTC1_OTG_BIT					bit(16)
#define INTC1_GMAC0_BIT					bit(17)
#define INTC1_GMAC1_BIT					bit(18)
#define INTC1_DC_BIT					bit(19)
#define INTC1_THSENS_BIT				bit(20)
#define INTC1_GPIO_0_15_BIT				bit(21)
#define INTC1_GPIO_16_31_BIT			bit(22)
#define INTC1_GPIO_32_47_BIT			bit(23)
#define INTC1_GPIO_48_63_BIT			bit(24)
#define INTC1_GPIO_64_79_BIT			bit(25)
#define INTC1_GPIO_80_95_BIT			bit(26)
#define INTC1_GPIO_96_105_BIT			bit(27)
#define INTC1_DDR_ECC0_BIT				bit(29)
#define INTC1_DDR_ECC1_BIT				bit(30)

/**
 * 传统中断路由, XXX 字节访问
 */
#define I_ENTRY_UART0					0x16001400			// UART00
#define I_ENTRY_UART1					0x16001401			// UART01
#define I_ENTRY_UART_2_5				0x16001402			// UART02~05
#define I_ENTRY_UART_6_9				0x16001403			// UART06~09
#define I_ENTRY_I2C_0_1					0x16001404			// I2C0~1
#define I_ENTRY_I2C_2_3					0x16001405			// I2C2~3
#define I_ENTRY_SPI2					0x16001406			// SPI2
#define I_ENTRY_SPI3					0x16001407			// SPI3
#define I_ENTRY_CAN0					0x16001408			// CAN0
#define I_ENTRY_CAN1					0x16001409			// CAN1
#define I_ENTRY_CAN2					0x1600140a			// CAN2
#define I_ENTRY_CAN3					0x1600140b			// CAN3
#define I_ENTRY_I2S						0x1600140c			// I2S
#define I_ENTRY_ATIMER					0x1600140d			// ATIMER
#define I_ENTRY_GTIMER					0x1600140e			// GTIMER
#define I_ENTRY_BTIMER					0x1600140f			// BTIMER
#define I_ENTRY_PWM_0_1					0x16001410			// PWM0/1
#define I_ENTRY_PWM_2_3					0x16001411			// PWM2/3
#define I_ENTRY_ADC						0x16001412			// ADC
#define I_ENTRY_HPET0					0x16001413			// HPET0
#define I_ENTRY_HPET1					0x16001414			// HPET1
#define I_ENTRY_HPET2					0x16001415			// HPET2
#define I_ENTRY_HPET3					0x16001416			// HPET3
#define I_ENTRY_DMA0					0x16001417			// APB-DMA0
#define I_ENTRY_DMA1					0x16001418			// APB-DMA1
#define I_ENTRY_DMA2					0x16001419			// APB-DMA2
#define I_ENTRY_DMA3					0x1600141a			// APB-DMA3
#define I_ENTRY_DMA4					0x1600141b			// APB-DMA4
#define I_ENTRY_DMA5					0x1600141c			// APB-DMA5
#define I_ENTRY_DMA6					0x1600141d			// APB-DMA6
#define I_ENTRY_DMA7					0x1600141e			// APB-DMA7
#define I_ENTRY_SDIO0_CTRL				0x1600141f			// SDIO0-CTRL

#define I_ENTRY_SDIO1_CTRL				0x16001440			// SDIO1-CTRL
#define I_ENTRY_SDIO0_DMA				0x16001441			// SDIO0-DMA
#define I_ENTRY_SDIO1_DMA				0x16001442			// SDIO1-DMA
#define I_ENTRY_ENCRYPT_DMA				0x16001443			// ENCYPT-DMA
#define I_ENTRY_AES						0x16001444			// AES
#define I_ENTRY_DES						0x16001445			// DES
#define I_ENTRY_SM3						0x16001446			// SM3
#define I_ENTRY_SM4						0x16001447			// SM4
#define I_ENTRY_RTC						0x16001448			// RTC-INT
#define I_ENTRY_TOY						0x16001449			// TOY-INT
#define I_ENTRY_RTC_TICK				0x1600144a			// RTC-TICK
#define I_ENTRY_TOY_TICK				0x1600144b			// TOY-TICK
#define I_ENTRY_SPI0					0x1600144c			// SPI0
#define I_ENTRY_SPI1					0x1600144d			// SPI1
#define I_ENTRY_EHCI					0x1600144e			// ECHI
#define I_ENTRY_OHCI					0x1600144f			// OHCI
#define I_ENTRY_OTG						0x16001450			// OTG
#define I_ENTRY_GMAC0					0x16001451			// GMAC0
#define I_ENTRY_GMAC1					0x16001452			// GMAC1
#define I_ENTRY_DC						0x16001453			// DC
#define I_ENTRY_THSENS					0x16001454			// THSENS
#define I_ENTRY_GPIO_0_15				0x16001455			// GPIO0~15
#define I_ENTRY_GPIO_16_31				0x16001456			// GPIO16~31
#define I_ENTRY_GPIO_32_47				0x16001457			// GPIO32~47
#define I_ENTRY_GPIO_48_63				0x16001458			// GPIO48~63
#define I_ENTRY_GPIO_64_79				0x16001459			// GPIO64~79
#define I_ENTRY_GPIO_80_95				0x1600145a			// GPIO80~95
#define I_ENTRY_GPIO_96_111				0x1600145b			// GPIO96~111
#define I_ENTRY_GPIO_112_127			0x1600145c			// GPIO112~127
#define I_ENTRY_DDR_ECC0				0x1600145d			// DDR-ECC0
#define I_ENTRY_DDR_ECC1				0x1600145e			// DDR-ECC1

/**
 * 7:4 路由的处理器核中断引脚向量号
 */
/*
#define INT_ROUTE_IP0					0x10				// 0001: LA264 处理器核0 中断号
#define INT_ROUTE_IP1					0x20				// 0010: LA264 处理器核1 中断号
#define INT_ROUTE_IP2					0x40				// 0100: LA264 处理器核2 中断号
#define INT_ROUTE_IP3					0x80				// 1000: LA264 处理器核3 中断号
 */

#else // #if USE_EXTINT

/**
 * 扩展中断
 */
#define EXTIOI_ACK_BASE					0x16001148			// EXTIOI_ACK 扩展中断设备反馈寄存器
#define EXTIOI_MAP_BASE					0x160014c0			// EXTIOI_MAP 扩展中断设备路由寄存器
#define EXTIOI_IEN0_BASE				0x16001600			// EXTIOI_IEN0 扩展中断设备使能寄存器0
#define EXTIOI_IEN1_BASE				0x16001604			// EXTIOI_IEN1 扩展中断设备使能寄存器1
#define EXTIOI_IEN2_BASE				0x16001608			// EXTIOI_IEN2 扩展中断设备使能寄存器2
#define EXTIOI_IEN3_BASE				0x1600160c			// EXTIOI_IEN3 扩展中断设备使能寄存器3
#define EXTIOI_POL0_BASE				0x16001640			// EXTIOI_POL0 扩展中断电平配置寄存器0
#define EXTIOI_POL1_BASE				0x16001644			// EXTIOI_POL1 扩展中断电平配置寄存器1
#define EXTIOI_POL2_BASE				0x16001648			// EXTIOI_POL2 扩展中断电平配置寄存器2
#define EXTIOI_POL3_BASE				0x1600164c			// EXTIOI_POL3 扩展中断电平配置寄存器3
#define EXTIOI_ISR0_BASE				0x16001700			// EXTIOI_ISR0 扩展中断状态寄存器0
#define EXTIOI_ISR1_BASE				0x16001704			// EXTIOI_ISR1 扩展中断状态寄存器1
#define EXTIOI_ISR2_BASE				0x16001708			// EXTIOI_ISR2 扩展中断状态寄存器2
#define EXTIOI_ISR3_BASE				0x1600170c			// EXTIOI_ISR3 扩展中断状态寄存器3
#define EXTIOI_CORE_ISR0_BASE			0x16001800			// EXTIOI_CORE_ISR0 路由至CORE扩展中断状态寄存器0
#define EXTIOI_CORE_ISR1_BASE			0x16001804			// EXTIOI_CORE_ISR1 路由至CORE扩展中断状态寄存器1
#define EXTIOI_CORE_ISR2_BASE			0x16001808			// EXTIOI_CORE_ISR2 路由至CORE扩展中断状态寄存器2
#define EXTIOI_CORE_ISR3_BASE			0x1600180c			// EXTIOI_CORE_ISR3 路由至CORE扩展中断状态寄存器3

/*
 * 扩展中断路由寄存器
 */
#define EXTI_MAP_127_96_MASK			(0x0F<<24)			// bit[27:24] EXT_IOI_ISR[127:96]统一路由的处理器核中断引脚向量号
#define EXTI_MAP_127_96_SHIFT			24
#define EXTI_MAP_95_64_MASK				(0x0F<<16)			// bit[19:16] EXT_IOI_ISR[95:64]统一路由的处理器核中断引脚向量号
#define EXTI_MAP_95_64_SHIFT			16
#define EXTI_MAP_63_32_MASK				(0x0F<<8)			// bit[11:8] EXT_IOI_ISR[63:32]统一路由的处理器核中断引脚向量号
#define EXTI_MAP_63_32_SHIFT			8
#define EXTI_MAP_31_0_MASK				(0x0F<<0)			// bit[3:0] EXT_IOI_ISR[31:0]统一路由的处理器核中断引脚向量号
#define EXTI_MAP_31_0_SHIFT				0

#define EXTI_MAP_IP0					0x01				// 0001: LA264 处理器核0 中断号
#define EXTI_MAP_IP1					0x02				// 0010: LA264 处理器核1 中断号
#define EXTI_MAP_IP2					0x04				// 0100: LA264 处理器核2 中断号
#define EXTI_MAP_IP3					0x08				// 1000: LA264 处理器核3 中断号

/**
 * 扩展中断位
 */
#define EXTI0_UART0_BIT					bit(0)
#define EXTI0_UART1_BIT					bit(1)
#define EXTI0_UART2_BIT					bit(2)
#define EXTI0_UART3_BIT					bit(3)
#define EXTI0_UART4_BIT					bit(4)
#define EXTI0_UART5_BIT					bit(5)
#define EXTI0_UART6_BIT					bit(6)
#define EXTI0_UART7_BIT					bit(7)
#define EXTI0_UART8_BIT					bit(8)
#define EXTI0_UART9_BIT					bit(9)
#define EXTI0_I2C0_BIT					bit(10)
#define EXTI0_I2C1_BIT					bit(11)
#define EXTI0_I2C2_BIT					bit(12)
#define EXTI0_I2C3_BIT					bit(13)
#define EXTI0_SPI2_BIT					bit(14)
#define EXTI0_SPI3_BIT					bit(15)
#define EXTI0_CAN0_CORE_BIT				bit(16)
#define EXTI0_CAN0_BUF_BIT				bit(17)
#define EXTI0_CAN1_CORE_BIT				bit(18)
#define EXTI0_CAN1_BUF_BIT				bit(19)
#define EXTI0_CAN2_CORE_BIT				bit(20)
#define EXTI0_CAN2_BUF_BIT				bit(21)
#define EXTI0_CAN3_CORE_BIT				bit(22)
#define EXTI0_CAN3_BUF_BIT				bit(23)
#define EXTI0_I2S_BIT					bit(24)
#define EXTI0_ATIMER_BIT				bit(25)
#define EXTI0_GTIMER_BIT				bit(26)
#define EXTI0_BTIMER_BIT				bit(27)
#define EXTI0_PWM0_BIT					bit(28)
#define EXTI0_PWM1_BIT					bit(29)
#define EXTI0_PWM2_BIT					bit(30)
#define EXTI0_PWM3_BIT					bit(31)

#define EXTI1_ADC_BIT					bit(32-32)
#define EXTI1_HPET0_0_BIT				bit(33-32)
#define EXTI1_HPET0_1_BIT				bit(34-32)
#define EXTI1_HPET0_2_BIT				bit(35-32)
#define EXTI1_HPET1_0_BIT				bit(36-32)
#define EXTI1_HPET1_1_BIT				bit(37-32)
#define EXTI1_HPET1_2_BIT				bit(38-32)
#define EXTI1_HPET2_0_BIT				bit(39-32)
#define EXTI1_HPET2_1_BIT				bit(40-32)
#define EXTI1_HPET2_2_BIT				bit(41-32)
#define EXTI1_HPET3_0_BIT				bit(42-32)
#define EXTI1_HPET3_1_BIT				bit(43-32)
#define EXTI1_HPET3_2_BIT				bit(44-32)
#define EXTI1_DMA0_BIT					bit(45-32)
#define EXTI1_DMA1_BIT					bit(46-32)
#define EXTI1_DMA2_BIT					bit(47-32)
#define EXTI1_DMA3_BIT					bit(48-32)
#define EXTI1_DMA4_BIT					bit(49-32)
#define EXTI1_DMA5_BIT					bit(50-32)
#define EXTI1_DMA6_BIT					bit(51-32)
#define EXTI1_DMA7_BIT					bit(52-32)
#define EXTI1_SDIO0_CTRL_BIT			bit(53-32)
#define EXTI1_SDIO1_CTRL_BIT			bit(54-32)
#define EXTI1_SDIO0_DMA_BIT				bit(55-32)
#define EXTI1_SDIO1_DMA_BIT				bit(56-32)
#define EXTI1_ENCRYPT_DMA_BIT			bit(57-32)
#define EXTI1_AES_BIT					bit(58-32)
#define EXTI1_DES_BIT					bit(59-32)
#define EXTI1_SM3_BIT					bit(60-32)
#define EXTI1_SM4_BIT					bit(61-32)
#define EXTI1_RTC_0_BIT					bit(62-32)
#define EXTI1_RTC_1_BIT					bit(63-32)

#define EXTI2_RTC_2_BIT					bit(64-64)
#define EXTI2_TOY_0_BIT					bit(65-64)
#define EXTI2_TOY_1_BIT					bit(66-64)
#define EXTI2_TOY_2_BIT					bit(67-64)
#define EXTI2_RTC_TICK_BIT				bit(68-64)
#define EXTI2_TOY_TICK_BIT				bit(69-64)
#define EXTI2_SPI0_BIT					bit(70-64)
#define EXTI2_SPI1_BIT					bit(71-64)
#define EXTI2_EHCI_BIT					bit(72-64)
#define EXTI2_OHCI_BIT					bit(73-64)
#define EXTI2_OTG_BIT					bit(74-64)
#define EXTI2_GMAC0_BIT					bit(75-64)
#define EXTI2_GMAC1_BIT					bit(76-64)
#define EXTI2_DC_BIT					bit(77-64)
#define EXTI2_THSENS_BIT				bit(78-64)
#define EXTI2_GPIO_0_3_BIT				bit(79-64)
#define EXTI2_GPIO_4_7_BIT				bit(80-64)
#define EXTI2_GPIO_8_11_BIT				bit(81-64)
#define EXTI2_GPIO_12_15_BIT			bit(82-64)
#define EXTI2_GPIO_16_19_BIT			bit(83-64)
#define EXTI2_GPIO_20_23_BIT			bit(84-64)
#define EXTI2_GPIO_24_27_BIT			bit(85-64)
#define EXTI2_GPIO_28_31_BIT			bit(86-64)
#define EXTI2_GPIO_32_35_BIT			bit(87-64)
#define EXTI2_GPIO_36_39_BIT			bit(88-64)
#define EXTI2_GPIO_40_43_BIT			bit(89-64)
#define EXTI2_GPIO_44_47_BIT			bit(90-64)
#define EXTI2_GPIO_48_51_BIT			bit(91-64)
#define EXTI2_GPIO_52_55_BIT			bit(92-64)
#define EXTI2_GPIO_56_59_BIT			bit(93-64)
#define EXTI2_GPIO_60_63_BIT			bit(94-64)
#define EXTI2_GPIO_64_67_BIT			bit(95-64)

#define EXTI3_GPIO_68_71_BIT			bit(96-96)
#define EXTI3_GPIO_72_75_BIT			bit(97-96)
#define EXTI3_GPIO_76_79_BIT			bit(98-96)
#define EXTI3_GPIO_80_83_BIT			bit(99-96)
#define EXTI3_GPIO_84_87_BIT			bit(100-96)
#define EXTI3_GPIO_88_91_BIT			bit(101-96)
#define EXTI3_GPIO_92_95_BIT			bit(102-96)
#define EXTI3_GPIO_96_99_BIT			bit(103-96)
#define EXTI3_GPIO_100_103_BIT			bit(104-96)
#define EXTI3_GPIO_104_105_BIT			bit(105-96)
#define EXTI3_DDR_ECC0_BIT				bit(111-96)
#define EXTI3_DDR_ECC1_BIT				bit(112-96)

#endif // #if USE_EXTINT

//-------------------------------------------------------------------------------------------------
// GPIO
//-------------------------------------------------------------------------------------------------

/**
 * 芯片共有106 个GPIO 引脚, 全部与其他功能引脚复用.
 */
#define GPIO_COUNT                      106

#if 0

#define GPIO_BASE						0x16104000			// GPIO 基地址

/**
 * 0x00–0x80 按位控制寄存器地址(需按字节形式访问, 按位读写)
 */
#define GPIO_OEN_BASE					0x16104000			// GPIO_OEN 共106 位GPIO 输出使能, 低有效. 每位控制一个GPIO 引脚.
#define GPIO_O_BASE						0x16104010			// GPIO_O 共106 位GPIO 输出值. 每位控制一个GPIO 引脚.
#define GPIO_I_BASE						0x16104020			// GPIO_I 共106 位GPIO 输入值. 每位控制一个GPIO 引脚.
#define GPIO_IEN_BASE					0x16104030			// GPIO_INT_EN 共106 位GPIO 中断使能. 每位控制一个GPIO 引脚.
#define GPIO_IPOL_BASE					0x16104040			// GPIO_INT_POL 共106 位GPIO 中断极性. 每位控制一个GPIO 引脚.
#define GPIO_IEDGE_BASE					0x16104050			// GPIO_INT_EDGE 共106 位GPIO 中断边沿性. 每位控制一个GPIO 引脚.
#define GPIO_ICLR_BASE					0x16104060			// GPIO_INT_CLR 共106 位GPIO 中断清除. 每位控制一个GPIO 引脚.
#define GPIO_ISR_BASE					0x16104070			// GPIO_INT_STS 共106 位GPIO 中断状态. 每位控制一个GPIO 引脚.
#define GPIO_IDUAL_BASE					0x16104080			// GPIO_INT_DUAL 共106 位GPIO 中断双沿模式. 每位控制一个GPIO 引脚.
#endif

/*
 *  触发方式
 *
 * 	| POL	| EDGE	|	 描述			|
 * 	|-------|-------|---------------|
 *	|  0	|  0	| 低电平触发中断	|
 *	|  1	|  0	| 高电平触发中断	|
 * 	|  0	|  1	| 下降沿触发中断	|
 *	|  1	|  1	| 上升沿触发中断	|
 *
 */

/**
 * 0x800-0xFFF 按字节控制寄存器地址(需按字节形式访问, 以字节为单位读写)
 */
#define GPIO_OEN_ADDR					0x16104800			// GPIO_OEN 共106 字节GPIO 输出使能, 低有效. 每个字节控制一个GPIO 引脚.
#define GPIO_O_ADDR						0x16104900			// GPIO_O 共106 字节GPIO 输出值. 每个字节控制一个GPIO 引脚.
#define GPIO_I_ADDR						0x16104A00			// GPIO_I 共106 字节GPIO 输入值. 每个字节控制一个GPIO 引脚.
#define GPIO_IEN_ADDR					0x16104B00			// GPIO_INT_EN 共106 字节GPIO 中断使能. 每个字节控制一个GPIO 引脚.
#define GPIO_IPOL_ADDR					0x16104C00			// GPIO_INT_POL 共106 字节GPIO 中断极性. 每个字节控制一个GPIO 引脚.
#define GPIO_IEDGE_ADDR					0x16104D00			// GPIO_INT_EDGE 共106 字节GPIO 中断边沿性. 每个字节控制一个GPIO 引脚.
#define GPIO_ICLR_ADDR					0x16104E00			// GPIO_INT_CLR 共106 字节GPIO 中断清除. 每个字节控制一个GPIO 引脚.
#define GPIO_ISR_ADDR					0x16104F00			// GPIO_INT_STS 共106 字节GPIO 中断状态. 每个字节控制一个GPIO 引脚.
#define GPIO_IDUAL_ADDR					0x16104F80			// GPIO_INT_DUAL 共106 字节GPIO 中断双沿模式. 每个字节控制一个GPIO 引脚.

//-------------------------------------------------------------------------------------------------
// 其它寄存器
//-------------------------------------------------------------------------------------------------

/**
 * 温度传感器
 */
#define THSENS_INT_HI0					0x16001500			// Thsens_int_ctrl_Hi0 温度传感器高温中断控制寄存器0
#define THSENS_INT_HI1					0x16001504			// Thsens_int_ctrl_Hi1 温度传感器高温中断控制寄存器1
#define THSENS_INT_LO0					0x16001508			// Thsens_int_ctrl_Lo0 温度传感器低温中断控制寄存器0
#define THSENS_INT_LO1					0x1600150c			// Thsens_int_ctrl_Lo1 温度传感器低温中断控制寄存器1
#define THSENS_INT_SR					0x16001510			// Thsens_int_status/clr 温度传感器中断状态/清除寄存器
#define THSENS_VALUE					0x16001514			// Thsens_value 温度传感器测量值(仅低11位有效位), 计算公式为: Tval=Thsens_value[10:0]*0.4-275
#define THSENS_SCALE_HI0				0x16001520			// Thsens_scale_hi0 温度传感器高阈值配置寄存器0
#define THSENS_SCALE_HI1				0x16001524			// Thsens_scale_hi1 温度传感器高阈值配置寄存器1

/**
 * 芯片识别号
 */
#define CHIP_ID4						0x16003fe0			// CHIP_ID4 芯片识别号4
#define CHIP_ID5						0x16003fe4			// CHIP_ID5 芯片识别号5
#define CHIP_ID6						0x16003fe8			// CHIP_ID6 芯片识别号6
#define CHIP_ID7						0x16003fec			// CHIP_ID7 芯片识别号7

#define CHIP_ID0						0x16003ff0			// CHIP_ID0 芯片识别号0
#define CHIP_ID1						0x16003ff4			// CHIP_ID1 芯片识别号1
#define CHIP_ID2						0x16003ff8			// CHIP_ID2 芯片识别号2
#define CHIP_ID3						0x16003ffc			// CHIP_ID3 芯片识别号3

//-------------------------------------------------------------------------------------------------
// 中断函数
//-------------------------------------------------------------------------------------------------

extern void ls2k_install_irq_handler(int vector, irq_handler_t isr, void *arg);
extern void ls2k_remove_irq_handler(int vector);

/*
 * 中断映射
 */
#define INT_ROUTE_IP0                   0x10                /* 中断路由到 IP0 */
#define INT_ROUTE_IP1                   0x20                /* 中断路由到 IP1 */
#define INT_ROUTE_IP2                   0x40                /* 中断路由到 IP2 */
#define INT_ROUTE_IP3                   0x80                /* 中断路由到 IP3 */
extern void ls2k_set_irq_routeip(int vector, int route_ip);

/*
 * 中断触发
 */
#define INT_TRIGGER_LEVEL               0x04                /* 电平触发中断 */
#define INT_TRIGGER_PULSE               0x08                /* 脉冲触发中断 */
extern void ls2k_set_irq_triggermode(int vector, int mode);

extern void ls2k_interrupt_enable(int vector);   			/* 根据中断向量使能中断 */
extern void ls2k_interrupt_disable(int vector);  			/* 根据中断向量禁止中断 */

extern int assert_sw_irq(unsigned int irqnum);      		/* Generate a software interrupt */
extern int negate_sw_irq(unsigned int irqnum);      		/* Clear a software interrupt */

/*
 * cache.S 函数
 */
extern void flush_cache(void);
extern void flush_cache_nowrite(void);
extern void clean_cache(unsigned long kva, unsigned int n);

extern void flush_dcache(void);
extern void clean_dcache(unsigned long kva, unsigned int n);
extern void clean_dcache_indexed(unsigned long kva, unsigned int n);
extern void clean_dcache_nowrite(unsigned long kva, unsigned int n);
extern void clean_dcache_nowrite_indexed(unsigned long kva, unsigned int n);

extern void clean_icache(unsigned long kva, unsigned int n);
extern void clean_icache_indexed(unsigned long kva, unsigned int n);

extern void clean_scache(unsigned long kva, unsigned int n);
extern void clean_scache_indexed(unsigned long kva, unsigned int n);
extern void clean_scache_nowrite(unsigned long kva, unsigned int n);
extern void clean_scache_nowrite_indexed(unsigned long kva, unsigned int n);

extern unsigned int get_memory_size(void);

/*
 * tick.c 函数
 */
extern unsigned long get_clock_ticks(void);

extern void delay_ms(int ms);
extern void delay_us(int us);

#endif // _LS2K301_H

/*
 * @@ END
 */
