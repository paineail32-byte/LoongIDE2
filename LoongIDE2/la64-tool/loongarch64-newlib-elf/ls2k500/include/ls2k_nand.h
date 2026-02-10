/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_NAND_H
#define _LS2K_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NAND operate defination
 */
enum
{
    NAND_OP_MAIN  = 0x0001,
    NAND_OP_SPARE = 0x0002,
};

/*
 * NAND 读写参数
 */
typedef struct
{
    unsigned int pageNum;           // physcal page number
    unsigned int colAddr;           // address in page / oob
    unsigned int opFlags;           // NAND_OP_MAIN / NAND_OP_SPARE
} NAND_PARAM_t;

/*
 * NAND 片选, NAND 控制器使用
 *
 */
#define NAND_CS                 0

/*
 * NAND flash 容量大小的参数, NAND 控制器使用
 *
 * 用于 ls2k_nand_init()/ls2k_nand_open() 的 arg 参数
 *
 */
#define NAND_FLASH_1Gb			1		// 0: 1Gb(2K页)
#define NAND_FLASH_2Gb			2		// 1: 2Gb(2K页)
#define NAND_FLASH_4Gb			4		// 2: 4Gb(2K页)
#define NAND_FLASH_8Gb			8		// 3: 8Gb(2K页)

/*
 * NAND FORESEE-FS33ND04GS108TFI0 chip info
 *
 * NAND flash 芯片的详细参数
 *
 */
#define BLOCKS_OF_CHIP          0x1000              // 4096 blocks of chip
#define PAGES_OF_BLOCK          0x40                // 64   pages per block
#define BYTES_OF_PAGE           0x800               // 2048 bytes per page
#define OOBBYTES_OF_PAGE        0x40                // 64   oobbytes of page

#define PAGES_OF_CHIP           (PAGES_OF_BLOCK*BLOCKS_OF_CHIP) // 0x40000 pages of chip
#define BYTES_OF_CHIP           (BYTES_OF_PAGE*PAGES_OF_CHIP)   // 512M    bytes of chip

#define BLOCK_TO_PAGE2K(x)		((x)<<6)            // first page in block
#define PAGE2K_TO_BLOCK(x)		((x)>>6)            // block of page

//-----------------------------------------------------------------------------
// NAND device
//-----------------------------------------------------------------------------

#if BSP_USE_NAND
extern const void *devNAND;
#endif

//-----------------------------------------------------------------------------
// NAND io control command                      param type
//-----------------------------------------------------------------------------

#define IOCTL_NAND_RESET            0x1000      // none
#define IOCTL_NAND_SET_FLASH        0x2000      // flash_param_t * - TODO
#define IOCTL_NAND_GET_ID           0x4000      // unsigned long *
#define IOCTL_NAND_GET_STATS        0x8000      // TODO

#define IOCTL_NAND_ERASE_BLOCK      0x0100      // unsigned long   - block number
#define IOCTL_NAND_ERASE_CHIP       0x0200      // unsigned long   - ~0: check is bad block
#define IOCTL_NAND_PAGE_BLANK       0x0400      // unsigned long   - page number
#define IOCTL_NAND_PAGE_VERIFY      0x0800      // TODO

#define IOCTL_NAND_MARK_BAD_BLOCK   0x0010      // unsigned long   - block number
#define IOCTL_NAND_IS_BAD_BLOCK     0x0020      // unsigned long   - block number

//-----------------------------------------------------------------------------
// NAND driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *nand_drv_ops;

#define ls2k_nand_init(nand, arg)             nand_drv_ops->init_entry(nand, arg)
#define ls2k_nand_open(nand, arg)             nand_drv_ops->open_entry(nand, arg)
#define ls2k_nand_close(nand, arg)            nand_drv_ops->close_entry(nand, arg)
#define ls2k_nand_read(nand, buf, size, arg)  nand_drv_ops->read_entry(nand, buf, size, arg)
#define ls2k_nand_write(nand, buf, size, arg) nand_drv_ops->write_entry(nand, buf, size, arg)
#define ls2k_nand_ioctl(nand, cmd, arg)       nand_drv_ops->ioctl_entry(nand, cmd, arg)

#else

/*
 * 初始化NAND设备
 * 参数:    dev     devNAND
 *          arg     unsigned, NAND_FLASH_1GB ~ NAND_FLASH_4GB
 *
 * 返回:    0=成功
 */
int NAND_initialize(const void *dev, void *arg);

/*
 * 打开NAND设备
 * 参数:    dev     devNAND
 *          arg     unsigned, NAND_FLASH_1GB ~ NAND_FLASH_4GB
 *
 * 返回:    0=成功
 */
int NAND_open(const void *dev, void *arg);

/*
 * 关闭NAND设备
 * 参数:    dev     devNAND
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int NAND_close(const void *dev, void *arg);

/*
 * 从NAND Flash芯片读数据
 * 参数:    dev     devNAND
 *          buf     类型: char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型: NAND_PARAM_t *.
 *
 * 返回:    读取的字节数
 *
 * 说明:    读取NAND FLash芯片的字节数取 16 的倍数.
 */
int NAND_read(const void *dev, void *buf, int size, void *arg);

/*
 * 向NAND Flash芯片写数据
 * 参数:    dev     devNAND
 *          buf     类型: char *, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量
 *          arg     类型: NAND_PARAM_t *.
 *
 * 返回:    写入的字节数
 *
 * 说明:    1. 写入前的NAND Flash块已经格式化;
 *          2. 建议对 NAND Flash芯片的写操作按照整页写入.
 */
int NAND_write(const void *dev, void *buf, int size, void *arg);

/*
 * 向NAND Flash芯片发送控制命令
 * 参数:    dev     devNAND
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_RESET            |   NULL, 复位NAND控制器
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_SET_FLASH        |   类型: flash_param_t *
 *                                      |   用途: 设置 flash 芯片参数
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_GET_ID           |   类型: unsigned int *
 *                                      |   用途: 读取Flash芯片 ID
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_ERASE_BLOCK      |   类型: unsigned int
 *                                      |   用途: 删除/格式化Flash芯片的一个块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_ERASE_CHIP       |   NULL, 删除/格式化整个Flash芯片
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_PAGE_BLANK       |   类型: unsigned int
 *                                      |   用途: 检查是否Flash芯片的一个块是不是空的
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_MARK_BAD_BLOCK   |   类型: unsigned int
 *                                      |   用途: 标记Flash芯片的一个块为坏块
 *      ---------------------------------------------------------------------------------
 *          IOCTL_NAND_IS_BAD_BLOCK     |   类型: unsigned int
 *                                      |   用途: 检查Flash芯片的一个块是否是坏块
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int NAND_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_nand_init(nand, arg)             NAND_initialize(nand, arg)
#define ls2k_nand_open(nand, arg)             NAND_open(nand, arg)
#define ls2k_nand_close(nand, arg)            NAND_close(nand, arg)
#define ls2k_nand_read(nand, buf, size, arg)  NAND_read(nand, buf, size, arg)
#define ls2k_nand_write(nand, buf, size, arg) NAND_write(nand, buf, size, arg)
#define ls2k_nand_ioctl(nand, cmd, arg)       NAND_ioctl(nand, cmd, arg)

#endif

/**
 * device name
 */
#define LS2K_NAND_DEVICE_NAME    	"nand0"

#ifdef __cplusplus
}
#endif

#endif // _LS2K_NAND_H

