/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "bsp.h"

#include "ls2k_uart.h"

#include "console.h"
#include "termios.h"

//-----------------------------------------------------------------------------

#if BSP_USE_FS

int fd_stdin  = -1;
int fd_stdout = -1;
int fd_stderr = -1;

/*
 * 使用标准输入输出
 */
#define CONSOLE_STDIO       0

#endif

//-----------------------------------------------------------------------------

void console_init(unsigned int baudrate)
{
    struct termios t;

    t.c_cflag = BAUDRATE_TO_CFLAG(baudrate) | CS8;  // 8N1

#if BSP_USE_FS

    char devnm[16];

    snprintf(devnm, 16, "/dev/%s", ls2k_uart_get_device_name(ConsolePort));

    fd_stdin  = open(devnm, O_RDWR, &t);    // STDIN_FILENO
    fd_stdout = open(devnm, O_RDWR, &t);    // STDOUT_FILENO
    fd_stderr = open(devnm, O_RDWR, &t);    // STDERR_FILENO

#else

    ls2k_uart_init(ConsolePort, NULL);
    ls2k_uart_open(ConsolePort, &t);

#endif
}

//-----------------------------------------------------------------------------

char console_getch(void)
{
    char ch = -1;

#if CONSOLE_STDIO
    if (fd_stdin == 0)
    {
        if (read(fd_stdin, &ch, 1) == 1)
        {
            return ch;
        }

        return -1;
    }
    else
#endif

    ch = Console_get_char(ConsolePort);

    return ch;
}

ssize_t console_gets(void *buf, size_t nbytes)
{
    int count = 0;

    if (!buf || (nbytes <= 0))
    {
        return 0;
    }

#if CONSOLE_STDIO
    if (fd_stdin == 0)
    {
        count = read(fd_stdin, buf, nbytes);
    }
    else
#endif

    /*
     * LS2K300 是否加上超时等待? 1ms
     */
    count = ls2k_uart_read(ConsolePort, buf, (int)nbytes, 0);

    return count;
}

//-----------------------------------------------------------------------------

void console_putch(char ch)
{
#if CONSOLE_STDIO
    if (fd_stdout == 1)
    {
        write(fd_stdout, &ch, 1);
    }
    else
#endif

    Console_output_char(ConsolePort, ch);
}

ssize_t console_puts(char *buf, size_t nbytes)
{
    if (!buf || (nbytes <= 0))
    {
        return 0;
    }

#if CONSOLE_STDIO
    if (fd_stdout == 1)
    {
        return write(fd_stdout, buf, nbytes);
    }
    else
#endif

    {
        int i = 0;
        char last_ch = 0;

        /*
         * 否则查询模式发送
         */

        while (i < nbytes)
        {
            if ((buf[i] == '\n') && (last_ch != '\r'))
                Console_output_char(ConsolePort, '\r');

            Console_output_char(ConsolePort, buf[i]);
            last_ch = buf[i];

            i++;
        }
    }

    return nbytes;
}

/******************************************************************************
 * RT-Thread
 */
#ifdef OS_RTTHREAD

#include "rtconfig.h"

#if defined(RT_USING_CONSOLE)

void rt_hw_console_output(const char *str)
{
    console_puts((char *)str, strlen(str));
}

char rt_hw_console_getchar(void)
{
    return console_getch();
}

#endif // #if defined(RT_USING_CONSOLE)

#endif // #ifdef OS_RTTHREAD

//-----------------------------------------------------------------------------
/*
 * @@ End
 */


