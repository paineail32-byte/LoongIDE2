/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#include <stdbool.h>

#include "rtthread.h"
#include "rthw.h"

#include "termios.h"
#include "ls1c203.h"

#include "bsp.h"

#include "drv_uart.h"
#include "console.h"

void console_init(unsigned int baudrate)
{
    struct termios t;

    UART_init(ConsolePort, NULL);
    
    t.c_cflag = BAUDRATE_TO_CFLAG(baudrate) | CS8;	// 8N1
    UART_open(ConsolePort, (void *)&t);
}

char console_getch(void)
{
   return Console_get_char(ConsolePort);
}

void console_putch(char ch)
{
    Console_output_char(ConsolePort, ch);
}

void console_putstr(char *s)
{
    while (*s)
    {
    	console_putch(*s++);
    }
}

//-----------------------------------------------------------------------------
// RTThread-nano support
//-----------------------------------------------------------------------------

#ifdef RT_USING_CONSOLE

void rt_hw_console_output(const char *str)
{
    while (*str)
    {
        if (str[0] == '\n') 
        {
            if (str[-1] != '\r')	/* rtthread use '\r' rarely */
            {
                Console_output_char(ConsolePort, '\r');
            }
        }

        Console_output_char(ConsolePort, *str++);
    }
}

#endif

