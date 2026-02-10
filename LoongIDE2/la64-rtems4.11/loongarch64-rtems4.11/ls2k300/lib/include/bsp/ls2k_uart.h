/*
 * ls2k300_uart.h
 *
 *  Created on: 2024-12-18
 *      Author: Bian
 */

#ifndef _LS2K_UART_H
#define _LS2K_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * UART device base
 */
#define LS2K_UART0_BASE         0x16100000
#define LS2K_UART1_BASE         0x16100400
#define LS2K_UART2_BASE         0x16100800
#define LS2K_UART3_BASE         0x16100c00
#define LS2K_UART4_BASE         0x16101000
#define LS2K_UART5_BASE         0x16101400
#define LS2K_UART6_BASE         0x16101800
#define LS2K_UART7_BASE         0x16101c00
#define LS2K_UART8_BASE         0x16102000
#define LS2K_UART9_BASE         0x16102400

/******************************************************************************
 * UART Names Table
 */
#define LS2K_DEVNM_UART0 		"/dev/tty0"
#define LS2K_DEVNM_UART1		"/dev/tty1"
#define LS2K_DEVNM_UART2		"/dev/tty2"
#define LS2K_DEVNM_UART3		"/dev/tty3"
#define LS2K_DEVNM_UART4		"/dev/tty4"
#define LS2K_DEVNM_UART5		"/dev/tty5"
#define LS2K_DEVNM_UART6		"/dev/tty6"
#define LS2K_DEVNM_UART7		"/dev/tty7"
#define LS2K_DEVNM_UART8		"/dev/tty8"
#define LS2K_DEVNM_UART9		"/dev/tty9"

#ifdef __cplusplus
}
#endif

#endif	/* _LS2K_UART_H */
