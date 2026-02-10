/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_gd25q80.h
 *
 * created: 2022/10/12
 *  author: Bian
 */

#ifndef _DRV_GD25Q80_H
#define _DRV_GD25Q80_H

#ifdef __cplusplus
extern "C" {
#endif

#define GD25Q80_DEVICE_NAME      "spi0.gd25q80"

void rt_ls2k_gd25q80_install(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_GD25Q80_H

