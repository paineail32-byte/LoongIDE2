/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k500_gpio.c
 *
 * created: 2022-03-03
 *  author: Bian
 */

#include <stdint.h>

#include "ls2k500.h"
#include "ls2k500_irq.h"
#include "ls2k500_gpio.h"

//-------------------------------------------------------------------------------------------------
// GPIO functions
//-------------------------------------------------------------------------------------------------

void gpio_mux(int gpionum, int mux)
{
    if ((gpionum >= 0) && (gpionum < GPIO_ALL_COUNT))
    {
        unsigned int register regval;
        int register regindex = gpionum / 8;
        int register field = (gpionum % 8) << 2;    /* multi 4 */

        regval = READ_REG32(GPIO_MUX_ADDR(regindex));
        regval &= ~(0x07 << field);
        regval |= (mux & 0x07) << field;
        WRITE_REG32(GPIO_MUX_ADDR(regindex), regval);
    }
}

void gpio_enable(int gpionum, int dir_in)
{
    if ((gpionum >= 0) && (gpionum < GPIO_ALL_COUNT))
    {
        int register regindex = gpionum / 32;
        int register bitshift = gpionum % 32;

        /* mux as GPIO first */
        gpio_mux(gpionum, GPIO_AS_GPIO);

        /* set direction second */
        if (dir_in)
        {
            OR_REG32(GPIO_OEN_ADDR(regindex), (1 << bitshift));     // 1: in; 0: out
        }
        else
        {
            AND_REG32(GPIO_OEN_ADDR(regindex), ~(1 << bitshift));
        }
    }
}

int gpio_read(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_ALL_COUNT))
    {
        unsigned int register regval;
        int register regindex = gpionum / 32;
        int register bitshift = gpionum % 32;

        regval = READ_REG32(GPIO_IN_ADDR(regindex));
        return ((int)(regval >> bitshift)) & 0x1;
    }

    return -1;
}

void gpio_write(int gpionum, int val)
{
    if ((gpionum >= 0) && (gpionum < GPIO_ALL_COUNT))
    {
        unsigned int register regval;
        int register regindex = gpionum / 32;
        int register bitshift = gpionum % 32;

        regval = READ_REG32(GPIO_OUT_ADDR(regindex));
        if (val)
        {
            regval |= 1 << bitshift;
        }
        else
        {
            regval &= ~(1 << bitshift);
        }
        WRITE_REG32(GPIO_OUT_ADDR(regindex), regval);
    }
}

void gpio_disable(int gpionum)
{
    gpio_mux(gpionum, GPIO_AS_PAD);
}



