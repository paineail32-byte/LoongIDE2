/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * w25q32.h
 *
 * created: 2022-03-11
 *  author: Bian
 */

#ifndef _W25Q32_H
#define _W25Q32_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// io control command
//-----------------------------------------------------------------------------

#define IOCTL_W25Q32_READ_ID            0x0001
#define IOCTL_W25Q32_READ_JDECID        0x0002
#define IOCTL_W25Q32_READ_UNIQUEID      0x0004
#define IOCTL_W25Q32_ERASE_4K           0x0008      /* sector erase, 4KB */
#define IOCTL_W25Q32_ERASE_32K          0x0010      /* block erase, 32KB */
#define IOCTL_W25Q32_ERASE_64K          0x0020      /* block erase, 64KB */
#define IOCTL_W25Q32_SECTOR_ERASE       0x0040      /* sector erase */
#define IOCTL_W25Q32_BULK_ERASE         0x0080      /* chip erase */
#define IOCTL_W25Q32_WRITE_PROTECT      0x0100      /* write protect */
#define IOCTL_W25Q32_IS_BLANK           0x0200      /* sector empty check */

//-----------------------------------------------------------------------------
// SPI0-W25Q32 driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *w25q32_drv_ops;

#define ls2k_w25q32_init(spi, arg)             w25q32_drv_ops->init_entry(spi, arg)
#define ls2k_w25q32_open(spi, arg)             w25q32_drv_ops->open_entry(spi, arg)
#define ls2k_w25q32_close(spi, arg)            w25q32_drv_ops->close_entry(spi, arg)
#define ls2k_w25q32_read(spi, buf, size, arg)  w25q32_drv_ops->read_entry(spi, buf, size, arg)
#define ls2k_w25q32_write(spi, buf, size, arg) w25q32_drv_ops->write_entry(spi, buf, size, arg)
#define ls2k_w25q32_ioctl(spi, cmd, arg)       w25q32_drv_ops->ioctl_entry(spi, cmd, arg)

#else

/*
 * 初始化W25Q32芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int W25Q32_init(const void *bus, void *arg);

/*
 * 打开W25Q32芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int W25Q32_open(const void *bus, void *arg);

/*
 * 关闭W25Q32芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int W25Q32_close(const void *bus, void *arg);

/*
 * 从W25Q32芯片读数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 读flash的起始地址(W25Q32内部地址从0开始进行线性编址)
 *
 * 返回:    读取的字节数
 */
int W25Q32_read(const void *bus, void *buf, int size, void *arg);

/*
 * 向W25Q32芯片写数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 写flash的起始地址(W25Q32内部地址从0开始进行线性编址)
 *
 * 返回:    写入的字节数
 *
 * 说明:    待写入的W25Q32块已经格式化
 */
int W25Q32_write(const void *bus, void *buf, int size, void *arg);

/*
 * 向总线/W25Q32芯片发送控制命令
 * 参数:    dev     busSPI0
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_ENABLE    |   NULL. 开启SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_DISABLE   |   NULL. 停止SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_READ_ID            |   类型: unsigned int *
 *                                          |   用途: 读取W25Q32芯片的ID
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_ERASE_4K           |   类型: unsigned long
 *          IOCTL_W25Q32_SECTOR_ERASE       |   用途: 擦除该地址所在的4K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_ERASE_32K          |   类型: unsigned long
 *                                          |   用途: 擦除该地址所在的32K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_ERASE_64K          |   类型: ulong nsigned int
 *                                          |   用途: 擦除该地址所在的64K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_BULK_ERASE         |   NULL, 擦除整块flash芯片
 *      ---------------------------------------------------------------------------------
 *          IOCTL_W25Q32_IS_BLANK           |   NULL, 检查是否为空
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int W25Q32_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_w25q32_init(spi, arg)             W25Q32_init(spi, arg)
#define ls2k_w25q32_open(spi, arg)             W25Q32_open(spi, arg)
#define ls2k_w25q32_close(spi, arg)            W25Q32_close(spi, arg)
#define ls2k_w25q32_read(spi, buf, size, arg)  W25Q32_read(spi, buf, size, arg)
#define ls2k_w25q32_write(spi, buf, size, arg) W25Q32_write(spi, buf, size, arg)
#define ls2k_w25q32_ioctl(spi, cmd, arg)       W25Q32_ioctl(spi, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _W25Q32_H

