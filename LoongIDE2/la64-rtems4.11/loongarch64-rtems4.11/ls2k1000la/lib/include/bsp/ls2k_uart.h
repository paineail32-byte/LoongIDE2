/*
 * ls2k1000_uart.h
 *
 *  Created on: 2022-12-22
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
#define UART_APB_BASE			0x1fe20000ul

#define LS2K_UART0_BASE			UART_APB_BASE+0x0000
#define LS2K_UART1_BASE			UART_APB_BASE+0x0100
#define LS2K_UART2_BASE			UART_APB_BASE+0x0200
#define LS2K_UART3_BASE			UART_APB_BASE+0x0300
#define LS2K_UART4_BASE			UART_APB_BASE+0x0400
#define LS2K_UART5_BASE			UART_APB_BASE+0x0500
#define LS2K_UART6_BASE			UART_APB_BASE+0x0600
#define LS2K_UART7_BASE			UART_APB_BASE+0x0700
#define LS2K_UART8_BASE			UART_APB_BASE+0x0800
#define LS2K_UART9_BASE			UART_APB_BASE+0x0900
#define LS2K_UART10_BASE		UART_APB_BASE+0x0A00
#define LS2K_UART11_BASE		UART_APB_BASE+0x0B00

/******************************************************************************
 * UART Names Table
 */
#define LS2K_DEVNM_UART0 		"/dev/tty0"
#define LS2K_DEVNM_UART1		"/dev/tty1"
#define LS2K_DEVNM_UART2		"/dev/tty2"
#define LS2K_DEVNM_UART3		"/dev/tty3"

/* Split UART0 As 4 */
#define LS2K_DEVNM_UART4		"/dev/tty4"
#define LS2K_DEVNM_UART5		"/dev/tty5"
#define LS2K_DEVNM_UART6		"/dev/tty6"

#define LS2K_DEVNM_UART7		"/dev/tty7"
#define LS2K_DEVNM_UART8		"/dev/tty8"

/* Split UART8 As 4 */
#define LS2K_DEVNM_UART9		"/dev/tty9"
#define LS2K_DEVNM_UART10		"/dev/tty10"
#define LS2K_DEVNM_UART11		"/dev/tty11"

#ifdef __cplusplus
}
#endif

#endif	/* _LS2K_UART_H */
