/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#ifndef _NS16550_H
#define _NS16550_H

#include <stdbool.h>
#include "ls1c103.h"

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

#define UART_BUF_SIZE       64

typedef struct
{
    char  Buf[UART_BUF_SIZE];
    int   Count;
    char *pHead;
    char *pTail;
} NS16550_buf_t;

typedef struct
{
    HW_NS16550_t  *hwUART;
    bool           bInterrupt;
    unsigned int   irqNum;
    int            baudRate;  
    NS16550_buf_t  rx_buf;
    NS16550_buf_t  tx_buf;
} UART_t;

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

#if (BSP_USE_UART0)
extern UART_t *devUART0;
#endif
#if (BSP_USE_UART1)
extern UART_t *devUART1;
#endif

//-----------------------------------------------------------------------------

/*
 * NS16550 io control command                  param type
 */
#define IOC_NS16550_SET_MODE    0x1000      // struct termios *

//-----------------------------------------------------------------------------
// UART function
//-----------------------------------------------------------------------------

int NS16550_init(void *dev, void *arg);
int NS16550_open(void *dev, void *arg);
int NS16550_close(void *dev, void *arg);
int NS16550_read(void *dev, unsigned char *buf, int size, void *arg);
int NS16550_write(void *dev, unsigned char *buf, int size, void *arg);
int NS16550_ioctl(void *dev, unsigned cmd, void *arg);

//-----------------------------------------------------------------------------
// UART as Console
//-----------------------------------------------------------------------------

extern UART_t *ConsolePort;

char Console_get_char(UART_t *pUART);
void Console_output_char(UART_t *pUART, char ch);

#endif // _NS16550_H

