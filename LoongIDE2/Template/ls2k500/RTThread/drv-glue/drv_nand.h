/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_nand.h
 *
 * created: 2022/6/7
 *  author: Bian
 */

#ifndef _DRV_NAND_H
#define _DRV_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPECIAL for LS2K NAND:
 *
 *   rt_device_read() / rt_device_write() parameter "pos" is as "NAND_PARAM_t *"
 *
 */

void rt_ls2k_nand_install(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_NAND_H

