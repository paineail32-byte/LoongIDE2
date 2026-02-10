/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp_start_hook.c
 *
 * created: 2024-11-13
 *  author: 
 */

#include <stddef.h>
#include <malloc.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bsp.h"
#include "ls2k_uart.h"

#include "ls2k_nand.h"

//-----------------------------------------------------------------------------
// 系统使用的变量
//-----------------------------------------------------------------------------

/**
 * DC 显示模式
 */
#if BSP_USE_DC
char LCD_display_mode[] = "800x480-16@60";
#endif

/**
 * 串口控制台端口
 */
void *ConsolePort = NULL;

//-----------------------------------------------------------------------------
// 在 heap 初始化完成、控制台初始化前执行
//-----------------------------------------------------------------------------

int bsp_start_hook1(void)
{
    ConsolePort = (void *)devUART2;

    /**
     * 文件系统
     */
	#if BSP_USE_FS
	{
		extern void filesystem_initialize(void);
		extern void register_all_devices(void);

		filesystem_initialize();		/* should be initialized first */

		register_all_devices();			/* register all devices as fs */
	}
	#endif

    /*
     * do more...
     */

	return 0;
}

//-----------------------------------------------------------------------------
// 在跳转 main() 之前执行
//-----------------------------------------------------------------------------

int bsp_start_hook2(void)
{
	/******************************************************
	 * avoid linker error!!!
	 */
	#ifdef LIB_BSP
	{
		(void)fstat(-1, NULL);
		(void)isatty(-1);
		(void)realloc(NULL, 0);
	}
	#endif

    /**
     * NAND 设备
     */
    #if USE_YAFFS2
    {
        #ifndef RYFS_MOUNTED_FS_NAME
        #define RYFS_MOUNTED_FS_NAME	"/ndd"
        #endif

        extern int yaffs2_startup_and_mount(const char *mount_name,
                                            int start_block, int end_block);
        extern int register_yaffs2(char *dev_name,
                                   char *mount_point,
                                   char *mount_point_me,
                                   mode_t mode,
                                   void *data);

        /**
         * NOTE: size of nand
         */
        ls2k_nand_init(devNAND, (void *)NAND_FLASH_4Gb);
        ls2k_nand_open(devNAND, NULL);

        /*
         * 检查是否有 NAND-Flash 芯片存在
         */
        unsigned long nand_flash_id = 0;

        if (ls2k_nand_ioctl(devNAND, IOCTL_NAND_GET_ID, (void *)&nand_flash_id) != 0)
        {
            ls2k_nand_close(devNAND, NULL);
            printk("  Read nand-flash chip ID error.\r\n");
        }

        /**
         * 如果没有 NAND-Flash 芯片, 不执行挂载
         */
        if (nand_flash_id == 0)
        {
            ls2k_nand_close(devNAND, NULL);
            printk("  No nand-flash chip exists.\r\n");
        }
        else if (yaffs2_startup_and_mount(RYFS_MOUNTED_FS_NAME, 400, 4095) == 0)
        {
            if (register_yaffs2(LS2K_NAND_DEVICE_NAME,
                                RYFS_MOUNTED_FS_NAME,
                                RYFS_MOUNTED_FS_NAME,
                                0, NULL) == 0 )
            {
                printk("yaffs2 register as filesystem successful.\r\n");
            }
            else
            {
                printk("yaffs2 register as filesystem initialized fail.\r\n");
            }
        }
    }
    #endif

    /**
     * SATA 设备
     */
    #if BSP_USE_SATA
    {
        extern int dwc_ahsata_probe(void);

        dwc_ahsata_probe();
    }
    #endif

    /**
     * USB 设备
     */
    #if BSP_USE_USB
    {
        extern int usbh_init(void);

        if (usbh_init() == 0)
        {
            printk("USB stack initialized successful.\r\n");
        }
        else
        {
            printk("USB stack initialized fail.\r\n");
        }
    }
    #endif

    /**
     * SHELL
     */
    #if BSP_USE_SHELL
    {
        extern void shell_task_start(const void *pUART);

        shell_task_start(NULL);
    }
    #endif

    /*
     * do more...
     */

    return 0;
}

/**
 * rt_components_board_init() 执行
 */
#ifdef OS_RTTHREAD
#include "rtdef.h"
INIT_EXPORT(bsp_start_hook2, "6");
#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

