/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

void console_init(unsigned int baudrate);

char console_getch(void);
ssize_t console_gets(void *buf, size_t nbytes);

void console_putch(char ch);
ssize_t console_puts(char *buf, size_t nbytes);

#ifdef __cplusplus
}
#endif

#endif /*__CONSOLE_H__*/

