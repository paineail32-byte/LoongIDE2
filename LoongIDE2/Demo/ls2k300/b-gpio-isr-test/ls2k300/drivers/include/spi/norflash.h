/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * norflash.h
 *
 * created: 2024-06-23
 *  author: Bian
 */

#ifndef _NORFLASH_H
#define _NORFLASH_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Device name
//-----------------------------------------------------------------------------

#define NORFLASH_DEV_NAME       "spi0.norflash"

//-----------------------------------------------------------------------------
// io control command
//-----------------------------------------------------------------------------

#define IOCTL_NORFLASH_READ_ID          0x0001
#define IOCTL_NORFLASH_READ_JDECID      0x0002
#define IOCTL_NORFLASH_READ_UNIQUEID    0x0004
#define IOCTL_NORFLASH_ERASE_4K         0x0008      /* sector erase, 4KB */
#define IOCTL_NORFLASH_ERASE_32K        0x0010      /* block erase, 32KB */
#define IOCTL_NORFLASH_ERASE_64K        0x0020      /* block erase, 64KB */
#define IOCTL_NORFLASH_SECTOR_ERASE     0x0040      /* sector erase */
#define IOCTL_NORFLASH_BULK_ERASE       0x0080      /* chip erase */
#define IOCTL_NORFLASH_WRITE_PROTECT    0x0100      /* write protect */
#define IOCTL_NORFLASH_IS_BLANK         0x0200      /* sector empty check */

//-----------------------------------------------------------------------------
// SPI0-NORFLASH driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *norflash_drv_ops;

#define ls2k_norflash_init(bus, arg)             norflash_drv_ops->init_entry(bus, arg)
#define ls2k_norflash_open(bus, arg)             norflash_drv_ops->open_entry(bus, arg)
#define ls2k_norflash_close(bus, arg)            norflash_drv_ops->close_entry(bus, arg)
#define ls2k_norflash_read(bus, buf, size, arg)  norflash_drv_ops->read_entry(bus, buf, size, arg)
#define ls2k_norflash_write(bus, buf, size, arg) norflash_drv_ops->write_entry(bus, buf, size, arg)
#define ls2k_norflash_ioctl(bus, cmd, arg)       norflash_drv_ops->ioctl_entry(bus, cmd, arg)

#else

/*
 * 初始化NORFLASH芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int NORFLASH_initialize(const void *bus, void *arg);

/*
 * 打开NORFLASH芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int NORFLASH_open(const void *bus, void *arg);

/*
 * 关闭NORFLASH芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int NORFLASH_close(const void *bus, void *arg);

/*
 * 从NORFLASH芯片读数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 读flash的起始地址(NORFLASH内部地址从0开始进行线性编址)
 *
 * 返回:    读取的字节数
 */
int NORFLASH_read(const void *bus, void *buf, int size, void *arg);

/*
 * 向NORFLASH芯片写数据
 * 参数:    dev     busSPI0
 *          buf     类型: unsigned char *, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int *, 写flash的起始地址(NORFLASH内部地址从0开始进行线性编址)
 *
 * 返回:    写入的字节数
 *
 * 说明:    待写入的NORFLASH块已经格式化
 */
int NORFLASH_write(const void *bus, void *buf, int size, void *arg);

/*
 * 向总线/NORFLASH芯片发送控制命令
 * 参数:    dev     busSPI0
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_ENABLE    |   NULL. 开启SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_DISABLE   |   NULL. 停止SPI总线的FLASH快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_READ_ID          |   类型: unsigned int *
 *                                          |   用途: 读取NORFLASH芯片的ID
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_ERASE_4K         |   类型: unsigned long
 *          IOCTL_NORFLASH_SECTOR_ERASE     |   用途: 擦除该地址所在的4K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_ERASE_32K        |   类型: unsigned long
 *                                          |   用途: 擦除该地址所在的32K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_ERASE_64K        |   类型: ulong nsigned int
 *                                          |   用途: 擦除该地址所在的64K块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_BULK_ERASE       |   NULL, 擦除整块flash芯片
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NORFLASH_IS_BLANK         |   NULL, 检查是否为空
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int NORFLASH_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_norflash_init(bus, arg)             NORFLASH_initialize(bus, arg)
#define ls2k_norflash_open(bus, arg)             NORFLASH_open(bus, arg)
#define ls2k_norflash_close(bus, arg)            NORFLASH_close(bus, arg)
#define ls2k_norflash_read(bus, buf, size, arg)  NORFLASH_read(bus, buf, size, arg)
#define ls2k_norflash_write(bus, buf, size, arg) NORFLASH_write(bus, buf, size, arg)
#define ls2k_norflash_ioctl(bus, cmd, arg)       NORFLASH_ioctl(bus, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _NORFLASH_H

