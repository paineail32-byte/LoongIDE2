/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_norflash.h
 *
 * created: 2021/1/6
 *  author: Bian
 */

#ifndef _DRV_NORFLASH_H
#define _DRV_NORFLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#define NORFLASH_DEVICE_NAME      "spi0.norflash"

void rt_ls2k_norflash_install(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_NORFLASH_H

