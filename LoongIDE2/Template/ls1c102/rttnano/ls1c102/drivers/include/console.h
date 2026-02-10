/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

void console_init(unsigned int baudrate);
char console_getch(void);
void console_putch(char ch);
void console_putstr(char *s);

#ifdef __cplusplus
}
#endif

#endif /*__CONSOLE_H__*/

