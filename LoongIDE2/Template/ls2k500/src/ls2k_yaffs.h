/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls2k_yaffs.h
 *
 *  Created on: 2014-3-18
 *      Author: Bian
 */

#ifndef LS2K_YAFFS2_H_
#define LS2K_YAFFS2_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RYFS_MOUNTED_FS_NAME	"/ndd"

/******************************************************************************
 * NAND flash chip type - 128M
 */
#define K9F1G08U0C      0x0C    // NOP=4
#define K9F1G08U0D      0x0D    // NOP=4
#define K9F1G08U0E      0x0E    // NOP=1

/******************************************************************************
 * NAND flash chip type - 512M
 */
#define FM29G04C        0x51    // NOP=1
#define XT27G04A        0x52    // NOP=?

/******************************************************************************
 * struct for initialize
 */
typedef struct yaffs2_nand_cfg
{
	/* likely "struct yaffs_param" */
	unsigned bytes_of_page;             // total_bytes_per_chunk = BYTES_OF_PAGE;
	int      pages_of_block;            // chunks_per_block      = PAGES_OF_BLOCK;
	int      oobbytes_of_page;          // spare_bytes_per_chunk = OOBBYTES_OF_PAGE;
	int      start_block;               // start_block           = 112;
	int      end_block;                 // end_block             = 935;
	int      reserved_blocks;           // n_reserved_blocks     = 5;
	int      cache_blocks;              // n_caches              = 8;
	/* NAND flash NOP */
	int      nand_flash_chip;           // K9F1G08U0C/D/E etc
	int      nand_rw_fullpage;          // 1: yes, by nand_flash_chip
} yaffs2_nand_cfg_t;

//*****************************************************************************
//
// 初始化并挂载文件系统   /ndd
//
// 注意: yaffs2_nand_cfg_t 参数与NAND芯片相关
//
//*****************************************************************************

int ls2k_initialize_yaffs2(yaffs2_nand_cfg_t *cfg);

int yaffs2_startup_and_mount(const char *mount_name, int start_block, int end_block);

int yaffs2_is_mounted(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LS2K_YAFFS2_H_ */


