/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * gd25q80.h
 *
 * created: 2022-10-12
 *  author: Bian
 */

#ifndef _GD25Q80_H
#define _GD25Q80_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// io control command
//-----------------------------------------------------------------------------

#define IOCTL_GD25Q80_READ_ID           0x0001
#define IOCTL_GD25Q80_READ_JDECID       0x0002
#define IOCTL_GD25Q80_READ_UNIQUEID     0x0004
#define IOCTL_GD25Q80_ERASE_4K          0x0008      /* sector erase, 4KB */
#define IOCTL_GD25Q80_ERASE_32K         0x0010      /* block erase, 32KB */
#define IOCTL_GD25Q80_ERASE_64K         0x0020      /* block erase, 64KB */
#define IOCTL_GD25Q80_SECTOR_ERASE      0x0040      /* sector erase */
#define IOCTL_GD25Q80_BULK_ERASE        0x0080      /* chip erase */
#define IOCTL_GD25Q80_WRITE_PROTECT     0x0100      /* write protect */
#define IOCTL_GD25Q80_IS_BLANK          0x0200      /* sector empty check */

//-----------------------------------------------------------------------------
// SPI0-GD25Q80 driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *gd25q80_drv_ops;

#define ls2k_gd25q80_init(spi, arg)             gd25q80_drv_ops->init_entry(spi, arg)
#define ls2k_gd25q80_open(spi, arg)             gd25q80_drv_ops->open_entry(spi, arg)
#define ls2k_gd25q80_close(spi, arg)            gd25q80_drv_ops->close_entry(spi, arg)
#define ls2k_gd25q80_read(spi, buf, size, arg)  gd25q80_drv_ops->read_entry(spi, buf, size, arg)
#define ls2k_gd25q80_write(spi, buf, size, arg) gd25q80_drv_ops->write_entry(spi, buf, size, arg)
#define ls2k_gd25q80_ioctl(spi, cmd, arg)       gd25q80_drv_ops->ioctl_entry(spi, cmd, arg)

#else

/*
 * 初始化GD25Q80芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int GD25Q80_init(const void *bus, void *arg);

/*
 * 打开GD25Q80芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int GD25Q80_open(const void *bus, void *arg);

/*
 * 关闭GD25Q80芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int GD25Q80_close(const void *bus, void *arg);

/*
 * 从GD25Q80芯片读数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 读flash的起始地址(GD25Q80内部地址从0开始进行线性编址)
 *
 * 返回:    读取的字节数
 */
int GD25Q80_read(const void *bus, void *buf, int size, void *arg);

/*
 * 向GD25Q80芯片写数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 写flash的起始地址(GD25Q80内部地址从0开始进行线性编址)
 *
 * 返回:    写入的字节数
 *
 * 说明:    待写入的GD25Q80块已经格式化
 */
int GD25Q80_write(const void *bus, void *buf, int size, void *arg);

/*
 * 向总线/GD25Q80芯片发送控制命令
 * 参数:    dev     busSPI0
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_ENABLE    |   NULL. 开启SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_DISABLE   |   NULL. 停止SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_READ_ID           |   类型: unsigned int *
 *                                          |   用途: 读取GD25Q80芯片的ID
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_ERASE_4K          |   类型: unsigned long
 *          IOCTL_GD25Q80_SECTOR_ERASE      |   用途: 擦除该地址所在的4K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_ERASE_32K         |   类型: unsigned long
 *                                          |   用途: 擦除该地址所在的32K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_ERASE_64K         |   类型: ulong nsigned int
 *                                          |   用途: 擦除该地址所在的64K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_BULK_ERASE        |   NULL, 擦除整块flash芯片
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GD25Q80_IS_BLANK          |   NULL, 检查是否为空
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int GD25Q80_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_gd25q80_init(spi, arg)             GD25Q80_init(spi, arg)
#define ls2k_gd25q80_open(spi, arg)             GD25Q80_open(spi, arg)
#define ls2k_gd25q80_close(spi, arg)            GD25Q80_close(spi, arg)
#define ls2k_gd25q80_read(spi, buf, size, arg)  GD25Q80_read(spi, buf, size, arg)
#define ls2k_gd25q80_write(spi, buf, size, arg) GD25Q80_write(spi, buf, size, arg)
#define ls2k_gd25q80_ioctl(spi, cmd, arg)       GD25Q80_ioctl(spi, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _GD25Q80_H

