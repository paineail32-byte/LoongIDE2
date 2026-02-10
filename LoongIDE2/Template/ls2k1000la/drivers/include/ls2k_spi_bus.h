/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_SPI_BUS_H
#define _LS2K_SPI_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ls2k_drv_io.h"

//-----------------------------------------------------------------------------
// SPI device communication mode
//-----------------------------------------------------------------------------

typedef struct
{
    unsigned int  baudrate;         /* maximum bits per second */
    unsigned char bits_per_char;    /* how many bits per byte/word/longword? */
    bool          lsb_first;        /* true: send LSB first */
    bool          clock_pha;        /* clock phase    - spi mode */
    bool          clock_pol;        /* clock polarity - spi mode */
    bool          clock_inv;        /* true: inverted clock (low active) - cs high or low */
    bool          clock_phs;        /* true: clock starts toggling at start of data tfr - interface mode */
} SPI_mode_t;

//-----------------------------------------------------------------------------
// SPI devices
//-----------------------------------------------------------------------------

extern const void *busSPI0;

/******************************************************************************
 * bus operators
 ******************************************************************************/

#if (PACK_DRV_OPS)

extern const libspi_ops_t *spi_drv_ops;

#define ls2k_spi_initialize(spi)            spi_drv_ops->init(spi)
#define ls2k_spi_send_start(spi, addr)      spi_drv_ops->send_start(spi, addr)
#define ls2k_spi_send_stop(spi, addr)       spi_drv_ops->send_stop(spi, addr)
#define ls2k_spi_send_addr(spi, addr, rw)   spi_drv_ops->send_addr(spi, addr, rw)
#define ls2k_spi_read_bytes(spi, buf, len)  spi_drv_ops->read_bytes(spi, buf, len)
#define ls2k_spi_write_bytes(spi, buf, len) spi_drv_ops->write_bytes(spi, buf, len)
#define ls2k_spi_ioctl(spi, cmd, arg)       spi_drv_ops->ioctl(spi, cmd, arg)

#else

/*
 * 初始化SPI总线
 * 参数:    none
 *
 * 返回:    0=成功
 *
 * 说明:    SPI总线在使用前, 必须要先调用该初始化函数
 */
int SPI_initialize(const void *bus);

/*
 * 开始SPI总线操作. 本函数获取SPI总线的控制权
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int SPI_send_start(const void *bus, unsigned int Addr);

/*
 * 结束SPI总线操作. 本函数释放SPI总线的控制权
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int SPI_send_stop(const void *bus, unsigned int Addr);

/*
 * 读写SPI总线前发送片选信号
 * 参数:    Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *          rw      未使用
 *
 * 返回:    0=成功
 */
int SPI_send_addr(const void *bus, unsigned int Addr, int rw);

/*
 * 从SPI从设备读取数据
 * 参数:    buf     类型 unsigned char *, 用于存放读取数据的缓冲区
 *          len     类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次读操作的字节数
 */
int SPI_read_bytes(const void *bus, unsigned char *buf, int len);

/*
 * 向SPI从设备写入数据
 * 参数:    buf     类型 unsigned char *, 用于存放待写数据的缓冲区
 *          len     类型 int, 待写的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次写操作的字节数
 */
int SPI_write_bytes(const void *bus, unsigned char *buf, int len);

/*
 * 向SPI总线发送控制命令
 * 参数:
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SPI_I2C_SET_TFRMODE       |   类型: SPI_mode_t *
 *                                          |   用途: 设置SPI总线的通信模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_ENABLE    |   NULL, 设置SPI控制器为 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_DISABLE   |   NULL, 取消SPI控制器的 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_GET_FAST_READ_MODE  |   类型: unsigned int *
 *                                          |   用途: 读取SPI控制器是否处于 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 *
 * 说明:    该函数调用的时机是: SPI设备已经初始化且空闲, 或者已经获取总线控制权
 */
int SPI_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_spi_initialize(spi)            SPI_initialize(spi)
#define ls2k_spi_send_start(spi, addr)      SPI_send_start(spi, addr)
#define ls2k_spi_send_stop(spi, addr)       SPI_send_stop(spi, addr)
#define ls2k_spi_send_addr(spi, addr, rw)   SPI_send_addr(spi, addr, rw)
#define ls2k_spi_read_bytes(spi, buf, len)  SPI_read_bytes(spi, buf, len)
#define ls2k_spi_write_bytes(spi, buf, len) SPI_write_bytes(spi, buf, len)
#define ls2k_spi_ioctl(spi, cmd, arg)       SPI_ioctl(spi, cmd, arg)

#endif

/******************************************************************************
 * bus api
 ******************************************************************************/

/*
 * 设置SPI控制器为 Flash快速读模式
 * 参数:    bus
 */
int ls2k_spiflash_fastread_enable(const void *bus);

/*
 * 取消SPI控制器的 Flash快速读模式
 * 参数:    bus
 */
int ls2k_spiflash_fastread_disable(const void *bus);

#ifdef __cplusplus
}
#endif

#endif // _LS2K_SPI_BUS_H

