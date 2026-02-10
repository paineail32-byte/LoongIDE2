/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_spi_hw.h
 *
 * created: 2022-02-24
 *  author: Bian
 */

#ifndef _LS2K_SPI_HW_H
#define _LS2K_SPI_HW_H

//-------------------------------------------------------------------------------------------------
// SPI 设备
//-------------------------------------------------------------------------------------------------

/*
 * SPI 控制器
 */
typedef struct
{
    volatile unsigned char ctrl;            // 0x00 控制寄存器
    volatile unsigned char sr;              // 0x01 状态寄存器
    volatile unsigned char data;            // 0x02 数据寄存器
    volatile unsigned char er;              // 0x03 外部寄存器
    volatile unsigned char param;           // 0x04 参数控制寄存器
    volatile unsigned char softcs;          // 0x05 片选寄存器
    volatile unsigned char timing;          // 0x06 时序控制寄存器
} HW_SPI_t;

#define SPI_CTRL_IEN            bit(7)      // 中断输出使能信号, 高有效
#define SPI_CTRL_EN             bit(6)      // 系统工作使能信号, 高有效
#define SPI_CTRL_MASTER         bit(4)      // master模式选择位, 此位一直保持1
#define SPI_CTRL_CPOL           bit(3)      // 时钟极性位
#define SPI_CTRL_CPHA           bit(2)      // 时钟相位位, 1则相位相反, 为0则相同
#define SPI_CTRL_SPR_MASK       0x03        // bit[1:0], sclk_o分频设定, 需要与sper的spre一起使用

#define SPI_SR_IFLAG            bit(7)      // 中断标志位, 1表示有中断申请, 写1则清零
#define SPI_SR_WOVERFLOW        bit(6)      // 写寄存器溢出标志位, 1表示已经溢出, 写1则清零
#define SPI_SR_BUSY             bit(4)      // 总线忙状态, 0: 总线空闲, 1: 总线忙状态
#define SPI_SR_WFFULL           bit(3)      // 写寄存器满标志, 1表示已经满
#define SPI_SR_WFEMPTY          bit(2)      // 写寄存器空标志, 1表示空
#define SPI_SR_RFFULL           bit(1)      // 读寄存器满标志, 1表示已经满
#define SPI_SR_RFEMPTY          bit(0)      // 读寄存器空标志, 1表示空

#define SPI_ER_ICNT_MASK        0xF0        // bit[7:4], 传输完多少个字节后发中断
#define SPI_ER_ICNT_SHIFT       4
#define SPI_ER_MODE             bit(2)      // spi接口模式控制, 0: 采样与发送时机同时, 1: 采样与发送时机错开半周期
#define SPI_ER_SPRE_MASK        0x03        // 与spr一起设定分频的比率

#define SPI_PARAM_CLKDIV_MASK   0xF0        // bit[7:4], 时钟分频数选择, 分频系数与{spre, spr}组合相同
#define SPI_PARAM_CLKDIV_SHIFT  4
#define SPI_PARAM_DUAL_IO       bit(3)      // 双I/O模式, 优先级高于快速读
#define SPI_PARAM_FAST_READ     bit(2)      // 快速读模式
#define SPI_PARAM_BURST_EN      bit(1)      // SPI flash支持连续地址读模式
#define SPI_PARAM_MEMORY_EN     bit(0)      // SPI flash读使能, 无效时csn[0]可由软件控制

#define SPI_SOFT_CSn_MASK       0xF0        // bit[7:4] csn引脚输出值
#define SPI_SOFT_CSn_SHIFT      4
#define SPI_SOFT_CSn_3          bit(7)
#define SPI_SOFT_CSn_2          bit(6)
#define SPI_SOFT_CSn_1          bit(5)
#define SPI_SOFT_CSn_0          bit(4)
#define SPI_SOFT_CSEN_MASK      0x0F        // bit[3:0] 为1时对应位的csn线由7:4位控制
#define SPI_SOFT_CSEN_3         bit(3)
#define SPI_SOFT_CSEN_2         bit(2)
#define SPI_SOFT_CSEN_1         bit(1)
#define SPI_SOFT_CSEN_0         bit(0)

#define SPI_TIMING_4B_ADDR_EN   bit(3)      // 4字节地址访问使能位(32MB空间大小扩展访问配置)
                                            // 1: 使能4字节地址访问, 此时SPI可支持最大32MB地址空间寻址;
                                            // 0: 关闭4字节地址访问, 此时SPI可支持最大16MB地址空间.
#define SPI_TIMING_tFAST        bit(2)      // SPI flash读采样模式. 0: 上沿采样, 间隔半个SPI周期; 1: 上沿采样, 间隔一个SPI周期
#define SPI_TIMING_tCSH_MASK    0x03        // bit[1:0] SPI Flash的片选信号最短无效时间，以分频后时钟周期T计算
#define SPI_TIMING_tCSH_1T      0
#define SPI_TIMING_tCSH_2T      1
#define SPI_TIMING_tCSH_4T      2
#define SPI_TIMING_tCSH_8T      3

#define SPI_CSn_EN              bit(1)      // SPI片选引脚输出使能, 低电平有效
#define SPI_CSn                 bit(0)      // SPI片选软件配置位, 低电平有效

#endif // _LS2K_SPI_HW_H

