/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_uart.c
 *
 * created: 2021/1/5
 *  author: Bian
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UART_BAUDRATE   115200

void rt_ls2k_uart_install(void);

extern const char *ls2k_uart_get_device_name(const void *pUART);

#ifdef __cplusplus
}
#endif

#endif  // __DRV_UART_H__



