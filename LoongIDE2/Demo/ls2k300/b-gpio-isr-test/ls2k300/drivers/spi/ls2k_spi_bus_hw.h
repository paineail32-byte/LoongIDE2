/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_spi_bus_hw.h
 *
 * created: 2024-06-08
 *  author: Bian
 */

#ifndef _LS2K_SPI_HW_H
#define _LS2K_SPI_HW_H

//-------------------------------------------------------------------------------------------------
// SPI 设备
//-------------------------------------------------------------------------------------------------

#define SPI0_BASE               0x16010000
#define SPI1_BASE               0x16018000

#define SPI0_MEM_ADDR			0x10000000		// – 0x11ff,ffff SPI0 MEM 32MB
#define SPI1_MEM_ADDR			0x12000000		// – 0x13ff,ffff SPI1 MEM 32MB

/*
 * SPI-Flash 控制器
 */
typedef struct
{
    volatile unsigned char ctrl;            // 0x00 控制寄存器
    volatile unsigned char sr;              // 0x01 状态寄存器
    volatile unsigned char data;            // 0x02 数据寄存器
    volatile unsigned char er;              // 0x03	外部寄存器
    volatile unsigned char param;       	// 0x04	参数控制寄存器
    volatile unsigned char softcs;          // 0x05 片选寄存器
    volatile unsigned char timing;          // 0x06 时序控制寄存器
} HW_SPI_t;

#define SPI_CTRL_IEN            bit(7)      // RW 0 中断输出使能信号高有效
#define SPI_CTRL_EN             bit(6)      // RW 0 系统工作使能信号高有效
#define SPI_CTRL_MASTER         bit(4)      // RO 1 master模式选择位, 此位一直保持1
#define SPI_CTRL_CPOL           bit(3)      // RW 0 时钟极性位
#define SPI_CTRL_CPHA           bit(2)      // RW 0 时钟相位位1 则相位相反，为0 则相同
#define SPI_CTRL_SPR_MASK       0x03        // RW 0 bit[1:0] sclk_o 分频设定，需要与sper 的spre 一起使用

#define SPI_SR_IFLAG            bit(7)      // RW 0 中断标志位1 表示有中断申请，写1 则清零
#define SPI_SR_WOVERFLOW        bit(6)      // RW 0 写寄存器溢出标志位为1 表示已经溢出,写1 则清零
#define SPI_SR_BUSY             bit(4)      // RO 0 总线忙状态, 0: 总线空闲, 1: 总线忙状态
#define SPI_SR_WFFULL           bit(3)      // RO 0 写寄存器满标志1 表示已经满
#define SPI_SR_WFEMPTY          bit(2)      // RO 1 写寄存器空标志1 表示空
#define SPI_SR_RFFULL           bit(1)      // RO 0 读寄存器满标志1 表示已经满
#define SPI_SR_RFEMPTY          bit(0)      // RO 1 读寄存器空标志1 表示空

#define SPI_ER_ICNT_MASK        0xC0        // RW 0 bit[7:6] 传输完多少个字节后发中断
#define SPI_ER_ICNT_SHIFT       6
#define SPI_ER_ICNT_1B			(0<<6)
#define SPI_ER_ICNT_2B			(1<<6)
#define SPI_ER_ICNT_3B			(2<<6)
#define SPI_ER_ICNT_4B			(3<<6)
#define SPI_ER_MODE             bit(2)      // RW 0 spi 接口模式控制. 0: 采样与发送时机同时, 1: 采样与发送时机错开半周期
#define SPI_ER_SPRE_MASK        0x03        // RW 0 bit[1:0] 与spr 一起设定分频的比率

/*
 * SPI 分频系数
 *
 *  ---------------------------------------------------------------------------------
 * | 	spre 	| 00 | 00 | 00 | 00 | 01 | 01 | 01  | 01  | 10  |  10  |  10  |  10  |
 * |	spr  	| 00 | 01 | 10 | 11 | 00 | 01 | 10  | 11  | 00  |  01  |  10  |  11  |
 * |------------|----|----|----|----|----|----|-----|-----|-----|------|------|------|
 * |  分频系数	| 2  | 4  | 16 | 32 | 8  | 64 | 128 | 256 | 512 | 1024 | 2048 | 4096 |
 *  ---------------------------------------------------------------------------------
 *
 */
#define SPI_PARAM_CLKDIV_MASK   0xF0        // RW 2 bit[7:4] 时钟分频数选择分频系数与{spre, spr}组合相同
#define SPI_PARAM_CLKDIV_SHIFT  4
#define SPI_PARAM_DUAL_IO       bit(3)      // RW 0 双I/O 模式，优先级高于快速读
#define SPI_PARAM_FAST_READ     bit(2)      // RW 0 快速读模式
#define SPI_PARAM_BURST_EN      bit(1)      // RW 0 SPI flash 支持连续地址读模式
#define SPI_PARAM_MEMORY_EN     bit(0)      // RW 1 SPI flash 读使能，无效时csn[0]可由软件控制

#define SPI_SOFT_CSn_MASK       0xF0        // RW bit[7:4] csn引脚输出值
#define SPI_SOFT_CSn_SHIFT      4
#define SPI_SOFT_CSn_3          bit(7)
#define SPI_SOFT_CSn_2          bit(6)
#define SPI_SOFT_CSn_1          bit(5)
#define SPI_SOFT_CSn_0          bit(4)
#define SPI_SOFT_CSEN_MASK      0x0F        // RW bit[3:0] 为1时对应位的csn线由7:4位控制
#define SPI_SOFT_CSEN_3         bit(3)
#define SPI_SOFT_CSEN_2         bit(2)
#define SPI_SOFT_CSEN_1         bit(1)
#define SPI_SOFT_CSEN_0         bit(0)

#define SPI_TIMING_tFAST        bit(2)      // RW 0 SPI flash 读采样模式. 0: 上沿采样，间隔半个SPI 周期, 1: 上沿采样，间隔一个SPI 周期
#define SPI_TIMING_tCSH_MASK    0x03        // R/W 3 bit[1:0] SPI Flash 的片选信号最短无效时间，以分频后时钟周期T计算
#define SPI_TIMING_tCSH_1T      0
#define SPI_TIMING_tCSH_2T      1
#define SPI_TIMING_tCSH_4T      2
#define SPI_TIMING_tCSH_8T      3

#endif // _LS2K_SPI_HW_H

