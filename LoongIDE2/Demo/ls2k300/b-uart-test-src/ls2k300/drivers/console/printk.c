/*
 * printk.c
 *
 * created: 2024-10-30
 *  author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "console.h"

//-----------------------------------------------------------------------------
// printk() function
//
// This function used to print to console directly
//-----------------------------------------------------------------------------

#define PRINTK_BUF_SIZE     511

void printk(const char *fmt, ...)
{
    int slen;
    char printk_buf[PRINTK_BUF_SIZE+1];

    if (!fmt)
    {
        return;
    }

    if (NULL != strchr(fmt, '%'))
    {
        va_list ap;

        va_start(ap, fmt);
        slen = vsnprintf(printk_buf, PRINTK_BUF_SIZE, fmt, ap);
        va_end(ap);

        if ((slen > 0) && (slen <= PRINTK_BUF_SIZE))
        {
            console_puts(printk_buf, (size_t)slen);
        }
    }
    else
    {
        slen = strlen(fmt);

        if ((slen > 0) && (slen <= PRINTK_BUF_SIZE))
        {
            console_puts((char *)fmt, (size_t)slen);
        }
    }

    return;
}

//-----------------------------------------------------------------------------
/*
 * @@ End
 */

