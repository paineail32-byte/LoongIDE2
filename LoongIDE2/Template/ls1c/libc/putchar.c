/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "console.h"

void putchar(int ch)
{
    console_putch(ch);
}

void _putchar(int ch)
{
    console_putch(ch);
}

int puts(const char *s)
{
    int count = 0;
    while (*s)
    {
        putchar(*s++);
        count++;
    }
    return count;
}


