/*
 * ls2k_nand.h
 *
 *  Created on: 2023-10-18
 *      Author: Bian
 */

#ifndef _LS2K_NAND_H_
#define _LS2K_NAND_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Driver interface
 */

#define LS2K_NAND_DRV_NAME	"/dev/nand0"

/*
 * XXX 这个驱动程序供YAFFS2读写数据使用. 注意直接读写NAND的操作.
 *
 * How to use:
 *
 * Open:
 *
 *  	int fd;
 *  	fd = open("/dev/nand0");
 *
 * Read:
 *
 * 		NAND_IO_t param; int readed;
 * 		param.pageNum=x;
 * 		...
 * 		readed = read(fd, &param, N);
 * 		if (readed > 0)
 * 		{
 * 			...
 * 		}
 *
 * Write:
 *
 * 		NAND_IO_t param; int writted;
 * 		param.id=x;
 * 		...
 * 		writted = write(fd, &param, param.dataSize);
 * 		if (writted > 0)
 * 		{
 * 			...
 * 		}
 *
 */

rtems_status_code bsp_install_nand_driver(void);

/*
 * NAND_Flash_t *flash
 */
rtems_status_code bsp_install_nand_flash_driver(void *flash);

/******************************************************************************/

/*
 * NAND operate defination
 */
enum
{
	NAND_OP_MAIN  = 0x0001,
	NAND_OP_SPARE = 0x0002,
};

typedef struct
{
	unsigned int   pageNum;			// physcal page number
	unsigned int   colAddr;			// address in page
	unsigned char *dataBuf;
	unsigned int   dataSize;
	unsigned int   opFlags;
} NAND_IO_t;

//-----------------------------------------------------------------------------
// NAND chip
//-----------------------------------------------------------------------------

/*
 * NAND 片选, NAND 控制器使用
 */
#define NAND_CS             0

/**
 * Flash 参数
 *
 * 初始化时使用的是默认参数: FORESEE-FS33ND04GS108TFI0
 *
 * 在 open 后使用 ioctl(fd, IOCTL_NAND_SET_FLASH_TYPE, NAND_Flash_t* flash) 设置.
 *
 */
typedef struct
{
    unsigned int bytes_per_page;
    unsigned int oobytes_of_page;
    unsigned int pages_per_block;
    unsigned int blocks_of_chip;
    int          flag_nop;              // if == 1 rw with full page
} NAND_Flash_t;

//-----------------------------------------------------------------------------
// NAND io control command                      param type
//-----------------------------------------------------------------------------

#define IOCTL_NAND_RESET        	0x1000 	// none
#define IOCTL_NAND_SET_FLASH_TYPE   0x2000 	// NAND_Flash_t  * - FLASH chip parameters
#define IOCTL_NAND_GET_ID       	0x4000 	// unsigned long *
#define IOCTL_NAND_GET_STATS    	0x8000  // TODO

#define IOCTL_NAND_ERASE_BLOCK      0x0100  // unsigned long   - block number
#define IOCTL_NAND_ERASE_CHIP       0x0200  // unsigned long   - ~0: check is bad block
#define IOCTL_NAND_PAGE_BLANK       0x0400  // unsigned long   - page number
#define IOCTL_NAND_PAGE_VERIFY      0x0800  // TODO

#define IOCTL_NAND_MARK_BAD_BLOCK   0x0010  // unsigned long   - block number
#define IOCTL_NAND_IS_BAD_BLOCK     0x0020  // unsigned long   - block number


#ifdef __cplusplus
}
#endif

#endif /* _LS2K_NAND_H_ */


