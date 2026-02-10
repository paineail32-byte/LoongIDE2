/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_is24c16.h
 *
 * created: 2022-10-12
 *  author: Bian
 */

#ifndef _DRV_IS24C16_H
#define _DRV_IS24C16_H

#ifdef __cplusplus
extern "C" {
#endif

#define IS24C16_DEVICE_NAME     "i2c0.is24c16"

void rt_ls2k_is24c16_install(void);

#ifdef __cplusplus
}
#endif

#endif // _DRV_IS24C16_H

