/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#ifndef _DRV_UART_H
#define _DRV_UART_H

#include "ls1c203.h"

//-----------------------------------------------------------------------------

#define CFLAG_TO_BAUDRATE(flag)      \
        ((flag == B1200)   ? 1200 :  \
         (flag == B2400)   ? 2400 :  \
         (flag == B4800)   ? 4800 :  \
         (flag == B9600)   ? 9600 :  \
         (flag == B19200)  ? 19200 : \
         (flag == B38400)  ? 38400 : \
         (flag == B57600)  ? 57600 : \
         (flag == B115200) ? 115200 : 9600)

#define BAUDRATE_TO_CFLAG(baud)      \
        ((baud == 1200)   ? B1200 :  \
         (baud == 2400)   ? B2400 :  \
         (baud == 4800)   ? B4800 :  \
         (baud == 9600)   ? B9600 :  \
         (baud == 19200)  ? B19200 : \
         (baud == 38400)  ? B38400 : \
         (baud == 57600)  ? B57600 : \
         (baud == 115200) ? B115200 : B9600)

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

#if (BSP_USE_UART0)
extern const void *devUART0;
#endif
#if (BSP_USE_UART1)
extern const void *devUART1;
#endif

//-----------------------------------------------------------------------------

/*
 * UART io control command                     param type
 */
#define IOCTL_UART_SET_MODE     0x0001      // struct termios *

//-----------------------------------------------------------------------------
// UART function
//-----------------------------------------------------------------------------

int UART_init(void *dev, void *arg);
int UART_open(void *dev, void *arg);
int UART_close(void *dev, void *arg);
int UART_read(void *dev, unsigned char *buf, int size, void *arg);
int UART_write(void *dev, unsigned char *buf, int size, void *arg);
int UART_ioctl(void *dev, unsigned cmd, void *arg);

//-----------------------------------------------------------------------------
// UART as Console
//-----------------------------------------------------------------------------

extern void *ConsolePort;

char Console_get_char(void *pUART);
void Console_output_char(void *pUART, char ch);

#endif // _DRV_UART_H

