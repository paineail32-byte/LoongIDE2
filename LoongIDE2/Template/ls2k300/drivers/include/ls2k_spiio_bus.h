/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_spiio_bus.h
 *
 * created: 2024-09-07
 *  author: Bian
 */

/*
 * 问题: 当 DI/DO 的字节数不是 sizeof(int) 的整数倍时, 可能存在问题...
 */

#ifndef _LS2K_SPIIO_BUS_H
#define _LS2K_SPIIO_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ls2k_drv_io.h"

#include "ls2k_spi_mode.h"

//-----------------------------------------------------------------------------
// SPI devices
//-----------------------------------------------------------------------------

#if BSP_USE_SPI2
extern const void *busSPI2;
#endif
#if BSP_USE_SPI3
extern const void *busSPI3;
#endif

//-----------------------------------------------------------------------------
// SPI-IO io control command                       param type
//-----------------------------------------------------------------------------

#define IOCTL_SPI_SET_WORKMODE      0x2001      // int: SPI work mode, see below

#define IOCTL_SPI_GET_WORKMODE      0x2002      // return: SPI work mode, see below

#define IOCTL_SPI_PRINT_WORKMODE    0x2003

/*
 * 工作模式: DMA or INT else POLL
 */
#define SPIIO_WORK_DMA      0x01
#define SPIIO_WORK_INT      0x02
#define SPIIO_WORK_POLL     0x04     /* default work mode */

#define SPIIO_RX_TX_SWAP    0x10

/******************************************************************************
 * bus operators
 ******************************************************************************/

#if (PACK_DRV_OPS)

extern const libspi_ops_t *spiio_drv_ops;

#define ls2k_spiio_initialize(bus)            spiio_drv_ops->init(bus)
#define ls2k_spiio_send_start(bus, addr)      spiio_drv_ops->send_start(bus, addr)
#define ls2k_spiio_send_stop(bus, addr)       spiio_drv_ops->send_stop(bus, addr)
#define ls2k_spiio_send_addr(bus, addr, rw)   spiio_drv_ops->send_addr(bus, addr, rw)
#define ls2k_spiio_read_bytes(bus, buf, len)  spiio_drv_ops->read_bytes(bus, buf, len)
#define ls2k_spiio_write_bytes(bus, buf, len) spiio_drv_ops->write_bytes(bus, buf, len)
#if SUPPORT_SPI_DUAL_IO
#define ls2k_spiio_rw_bytes(bus, \
                txbuf, txlen, rxbuf, rxlen)   spiio_drv_ops->rw_bytes(bus, txbuf, txlen, rxbuf, rxlen)
#endif
#define ls2k_spiio_ioctl(bus, cmd, arg)       spiio_drv_ops->ioctl(bus, cmd, arg)

#else

/*
 * 初始化SPI总线
 * 参数:    I2C device
 *
 * 返回:    0=成功
 *
 * 说明:    SPI总线在使用前, 必须要先调用该初始化函数
 */
int SPIIO_initialize(const void *bus);

/*
 * 开始SPI总线操作. 本函数获取SPI总线的控制权
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int SPIIO_send_start(const void *bus, unsigned int Addr);

/*
 * 结束SPI总线操作. 本函数释放SPI总线的控制权
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int SPIIO_send_stop(const void *bus, unsigned int Addr);

/*
 * 读写SPI总线前发送片选信号
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *          rw      未使用
 *
 * 返回:    0=成功
 */
int SPIIO_send_addr(const void *bus, unsigned int Addr, int rw);

/*
 * 从SPI从设备读取数据
 * 参数:    buf     类型 unsigned char *, 用于存放读取数据的缓冲区
 *          len     类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次读操作的字节数
 */
int SPIIO_read_bytes(const void *bus, unsigned char *buf, int len);

/*
 * 向SPI从设备写入数据
 * 参数:    buf     类型 unsigned char *, 用于存放待写数据的缓冲区
 *          len     类型 int, 待写的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次写操作的字节数
 */
int SPIIO_write_bytes(const void *bus, unsigned char *buf, int len);

/**
 * 全双工读写
 */
#if SUPPORT_SPI_DUAL_IO
int SPIIO_read_write_bytes(const void *bus, unsigned char *txbuf, int txlen,
                                      unsigned char *rxbuf, int rxlen);
#endif

/*
 * 向SPI总线发送控制命令
 * 参数:
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SPI_I2C_SET_TFRMODE       |   类型: SPI_mode_t *
 *                                          |   用途: 设置SPI总线的通信模式
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 *
 * 说明:    该函数调用的时机是: SPI设备已经初始化且空闲, 或者已经获取总线控制权
 */
int SPIIO_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_spiio_initialize(bus)            SPIIO_initialize(bus)
#define ls2k_spiio_send_start(bus, addr)      SPIIO_send_start(bus, addr)
#define ls2k_spiio_send_stop(bus, addr)       SPIIO_send_stop(bus, addr)
#define ls2k_spiio_send_addr(bus, addr, rw)   SPIIO_send_addr(bus, addr, rw)
#define ls2k_spiio_read_bytes(bus, buf, len)  SPIIO_read_bytes(bus, buf, len)
#define ls2k_spiio_write_bytes(bus, buf, len) SPIIO_write_bytes(bus, buf, len)
#if SUPPORT_SPI_DUAL_IO
#define ls2k_spiio_rw_bytes(bus, \
                txbuf, txlen, rxbuf, rxlen)   SPIIO_read_write_bytes(bus, txbuf, txlen, rxbuf, rxlen)
#endif
#define ls2k_spiio_ioctl(bus, cmd, arg)       SPIIO_ioctl(bus, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _LS2K_SPIIO_BUS_H

