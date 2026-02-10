/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dma.h
 *
 * created: 2024-06-19
 *  author: 
 */

#ifndef _LS2K_DMA_H
#define _LS2K_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * DMA 通道编号
 */
#define DMA_Channel_0   0x00
#define DMA_Channel_1   0x01
#define DMA_Channel_2   0x02
#define DMA_Channel_3   0x03
#define DMA_Channel_4   0x04
#define DMA_Channel_5   0x05
#define DMA_Channel_6   0x06
#define DMA_Channel_7   0x07

/**
 * DMA 外设
 */
#define DMA_UART0       0x01        // RX & TX
#define DMA_UART1       0x02        // RX & TX
#define DMA_UART2       0x03        // RX & TX
#define DMA_UART3       0x04        // RX & TX
#define DMA_UART4       0x05        // RX & TX
#define DMA_UART5       0x06        // RX & TX
#define DMA_UART6       0x07        // RX & TX
#define DMA_UART7       0x08        // RX & TX
#define DMA_UART8       0x09        // RX & TX
#define DMA_UART9       0x0A        // RX & TX

#define DMA_I2C0        0x10        // RX & TX
#define DMA_I2C1        0x11        // RX & TX
#define DMA_I2C2        0x12        // RX & TX
#define DMA_I2C3        0x13        // RX & TX

#define DMA_SPI2        0x20        // RX & TX
#define DMA_SPI3        0x21        // RX & TX

#define DMA_I2S         0x30        // RX & TX

#define DMA_ADC         0x40        // RX

#define DMA_CAN0        0x50        // RX
#define DMA_CAN1        0x51        // RX
#define DMA_CAN2        0x52        // RX
#define DMA_CAN3        0x53        // RX

#define DMA_MEM         0x54        // any channel for mem2mem

#define DMA_ATIM        0x60        // fixed: CH1 CH2 CH3 CH4 COM UP TRG
#define DMA_GTIM        0x61        // fixed: CH1 CH2 CH3 CH4  -  UP TRG

//-----------------------------------------------------------------------------
// DMA中断回调函数
//-----------------------------------------------------------------------------

#define DMA_SR_ERROR    (1<<3)      // DMA 出错
#define DMA_SR_DONE 	(1<<1)      // 传输完成

struct dma_chnl_cfg;

/*
 * 参数:    chnl    DMA通道号
 *          status  DMA_SR_ERR | DMA_SR_DONE
 */
typedef void (*dma_callback_t)(struct dma_chnl_cfg *cfg, int bytes, unsigned int status);

//-----------------------------------------------------------------------------

/**
 * DMA 通道控制寄存器
 */
struct dma_ccr
{
    unsigned int en       : 1;      // bit[0] DMA_CCR_EN   通道开启
    unsigned int tcie     : 1;      // bit[1] DMA_CCR_TCIE 传输完成中断使能
    unsigned int htie     : 1;      // bit[2] DMA_CCR_HTIE 传输过半中断使能
    unsigned int teie     : 1;      // bit[3] DMA_CCR_TEIE 传输错误中断使能
    unsigned int dir      : 1;      // bit[4] DMA_CCR_DIR  数据传输方向: 0: 从外设读; 1: 从存储器读.
    unsigned int circ     : 1;      // bit[5] DMA_CCR_CIRC 循环模式
    unsigned int pinc     : 1;      // bit[6] DMA_CCR_PINC 外设地址增量模式
    unsigned int minc     : 1;      // bit[7] DMA_CCR_MINC 存储器地址增量模式
    unsigned int psize    : 2;      // bit[9:8]     外设数据宽度
    unsigned int msize    : 2;      // bit[11:10]   存储器数据宽度
    unsigned int priority : 2;      // bit[13:12]   通道优先级
    unsigned int mem2mem  : 1;      // bit[14] DMA_CCR_MEM2MEM 储器到存储器模式
};

/**
 * DMA 通道配置
 */
struct dma_chnl_cfg
{
    int      chNum;                 // 使用的DMA通道号: DMA_CHNL0~DMA_CHNL7; -1时自动查找空闲通道

    unsigned devNum;                // 外部设备: DMA_UART0~DMA_GTIM. mem2mem=1: as memory source address
    void    *device;                // 外部设备.
    unsigned memAddr;               // 内存地址, 低32位
    int      transbytes;            // 传输数据字节数

    dma_callback_t cb;              // 中断回调函数

    union
    {
        unsigned int ccr32;         // CCR 配置值
        struct dma_ccr ccr;         // DMA 方向: ccr.dir: 1=mem->peripheral
    };
};

//-----------------------------------------------------------------------------
// IOCTL 参数                               arg 参数
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DMA function
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *dma_drv_ops;

#define ls2k_dma_init(dma, arg)             dma_drv_ops->init_entry(dma, arg)
#define ls2k_dma_open(dma, arg)             dma_drv_ops->open_entry(dma, arg)
#define ls2k_dma_close(dma, arg)            dma_drv_ops->close_entry(dma, arg)

#else

/*
 * 初始化DMA设备
 * 参数:    dev     NULL
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int DMA_initialize(const void *dev, void *arg);

/*
 * 开启DMA通道
 * 参数:    dev     NULL
 *          arg     类型 struct dma_chnl_cfg *, 把DMA通道配置为指定参数模式. 该参数不可NULL.
 *
 * 返回:    0=成功
 */
int DMA_open(const void *dev, void *arg);

/*
 * 关闭DMA通道
 * 参数:    dev     NULL
 *          arg     类型 int, DMA通道号.
 *
 * 返回:    0=成功
 */
int DMA_close(const void *dev, void *arg);

#define ls2k_dma_init(dma, arg)             DMA_initialize(dma, arg)
#define ls2k_dma_open(dma, arg)             DMA_open(dma, arg)
#define ls2k_dma_close(dma, arg)            DMA_close(dma, arg)

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// DMA User API
//-----------------------------------------------------------------------------

/**
 * 获取空闲的DMA通道
 *
 * 参数:    devNum      DMA_UART0 ~ DMA_GTIM
 *          rx_chnl     接收通道 DMA_CHNL0 ~ DMA_CHNL7
 *          tx_chnl     发送通道 DMA_CHNL0 ~ DMA_CHNL7. 双通道传输使用
 *
 * 返回:    0=成功
 *          
 */
int dma_get_idle_channel(int devNum, int *rx_chnl, int *tx_chnl);

/**
 * 通道 channel 是否空闲
 *
 * 参数:    channel     DMA_CHNL0 ~ DMA_CHNL7
 *
 * 返回:    1: idle
 *
 */
int dma_channel_is_idle(int channel);

/**
 * 通道 channel 是否就绪
 *
 * 参数:    channel     DMA_CHNL0 ~ DMA_CHNL7
 *
 * 返回:    1: ready
 *
 */
int dma_channel_is_ready(int channel);

/**
 * 启动DMA传输
 *
 * 参数:    cfg         DMA 传输的配置参数
 *          priority    DMA 优先级, <=0 不改变
 *
 * 返回:    0=成功, -1=失败
 *
 */
#define DMA_PRIORITY_LOW		0x01
#define DMA_PRIORITY_MID		0x02
#define DMA_PRIORITY_HIGH		0x04
#define DMA_PRIORITY_HIGHEST	0x08

int dma_start(struct dma_chnl_cfg *cfg, int priority);

/**
 * 暂停启动的 DMA 通道
 */
int dma_pause(int channel);

/**
 * 重新启动已经配置好的DMA通道
 *
 * 参数:    channel     DMA_CHNL0 ~ DMA_CHNL7
 *          buf         数据地址
 *          size        字节数
 *          xferbits    传输宽度, 8/16/32 之外不修改初始设置
 *
 * 返回:    -1=失败, 否则返回 size
 *
 */
#define DMA_XFER_8b         8
#define DMA_XFER_16b        16
#define DMA_XFER_32b        32

int dma_restart(int channel, char *buf, int size, int xferbits);

/**
 * 停止DMA通道
 *
 * 参数:    channel      DMA_CHNL0 ~ DMA_CHNL7
 *
 */
void dma_stop(int channel);

/**
 * 获取DMA通道状态寄存器
 *
 * 参数:    channel      DMA_CHNL0 ~ DMA_CHNL7
 *
 * 返回:    DMA通道的状态寄存器(已右移相应通道). -1=没有启用
 *
 */
int dma_get_status(int channel);

/**
 * 获取DMA通道状态寄存器
 *
 * 参数:    channel     DMA_CHNL0 ~ DMA_CHNL7
 *          timeout     超时等待, 仅是一个数字
 *
 */
int dma_wait_done(int channel, int timeout);


#ifdef __cplusplus
}
#endif

#endif // _LS2K_DMA_H

