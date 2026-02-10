/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_w25q32.h
 *
 * created: 2021/1/6
 *  author: Bian
 */

#ifndef _DRV_W25Q32_H
#define _DRV_W25Q32_H

#ifdef __cplusplus
extern "C" {
#endif

#define W25Q32_DEVICE_NAME      "spi0.w25q32"

void rt_ls2k_w25q32_install(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_W25Q32_H

