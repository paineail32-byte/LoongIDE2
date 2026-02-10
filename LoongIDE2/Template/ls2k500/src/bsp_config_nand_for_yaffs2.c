/*
 * bsp_config_nand_for_yaffs2.c
 *
 * created: 2025-02-16
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_NAND && USE_YAFFS2

#include <stddef.h>
#include <sys/types.h>

#include "ls2k_nand.h"
#include "ls2k_yaffs.h"

//-----------------------------------------------------------------------------
// 初始化并挂载文件系统   /ndd
//-----------------------------------------------------------------------------

extern int yaffsfs_init_mutex(void);
extern int yaffs_mount(const char *path);

int yaffs2_startup_and_mount(const char *mount_name, int start_block, int end_block)
{
    int ret;
    yaffs2_nand_cfg_t nand_cfg = { 0 };

    if (!mount_name || (start_block < 0) || ((end_block - start_block) < 100))
    {
        printk("yaffs2_startup_and_mount() argument error.\r\n");
        return -1;
    }

    if (yaffsfs_init_mutex() != 0)
    {
        printk("create yaffs2 mutex fail\r\n");
        return -1;
    }

#if 0
    /*
     * PMON config for linux
     */
	add_mtd_device(ls2k_mtd, 0, 14*1024*1024, "kernel");
	add_mtd_device(ls2k_mtd, 14*1024*1024, 100*1024*1024, "rootfs");
	add_mtd_device(ls2k_mtd, (100+14)*1024*1024, 14*1024*1024, "data");
#endif

	nand_cfg.bytes_of_page    = BYTES_OF_PAGE;      // 2K bytes
	nand_cfg.pages_of_block   = PAGES_OF_BLOCK;     // 64 pages
	nand_cfg.oobbytes_of_page = OOBBYTES_OF_PAGE;
	nand_cfg.start_block      = start_block;        // 400; // 50*1024*1024 == 400*64*2K
	nand_cfg.end_block        = end_block;          // 4096-1;
	nand_cfg.reserved_blocks  = 16; // (end_block - start_block)*2/100; // 2%
	nand_cfg.cache_blocks     = 32; // (end_block - start_block)*2/100; // 3%
    nand_cfg.nand_flash_chip  = XT27G04A;           // 512M
    nand_cfg.nand_rw_fullpage = 0;

    ls2k_initialize_yaffs2(&nand_cfg);

    ret = yaffs_mount(mount_name);

    printk("YAFFS: NAND flash mounted as %s %s.\r\n",
            mount_name, (ret == 0) ? "successful" : "fail");

    return ret;
}

#endif // #if BSP_USE_NAND && USE_YAFFS2

