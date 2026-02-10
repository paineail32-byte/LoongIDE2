/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_gpio.c
 *
 * created: 2024-07-03
 *  author: Bian
 */

#include <stdint.h>

#include "ls2k300.h"
#include "ls2k300_irq.h"
#include "ls2k_gpio.h"

//-----------------------------------------------------------------------------

/**
 * GPIO 复用配置寄存器
 *
 * 00: 复用为GPIO; 01: 第一复用; 10: 第二复用;  11: 引脚主功能.
 *
 */
void gpio_mux(int gpionum, int mux)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        unsigned int register regval;
        int register regindex = gpionum / 16;
        int register field = (gpionum % 16) << 1;    /* multi 2 */

        regval = READ_REG32(GPIO_MUX_ADDR(regindex));
        regval &= ~(0x03 << field);
        regval |= (mux & 0x03) << field;
        WRITE_REG32(GPIO_MUX_ADDR(regindex), regval);
    }
}

//-----------------------------------------------------------------------------

/**
 * 0x800-0xFFF 按字节控制寄存器地址(需按字节形式访问, 以字节为单位读写)
 */
 
void gpio_enable(int gpionum, int dir_in)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        /* mux as GPIO first */
        gpio_mux(gpionum, PAD_AS_GPIO);

        /* set direction second */
        if (DIR_IN == dir_in)
        {
            WRITE_REG8(GPIO_OEN_ADDR + gpionum, DIR_IN);    // 1: in
        }
        else
        {
            WRITE_REG8(GPIO_OEN_ADDR + gpionum, DIR_OUT);   // 0: out
        }
    }
}

int gpio_read(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        /* confirm the gpio is in */
        // if (DIR_IN == READ_REG8(GPIO_OEN_ADDR + gpionum))
        {
            return (int)READ_REG8(GPIO_I_ADDR + gpionum);
        }
    }

    return -1;
}

void gpio_write(int gpionum, int val)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        /* confirm the gpio is out */
        // if (DIR_OUT == READ_REG8(GPIO_OEN_ADDR + gpionum))
        {
            WRITE_REG8(GPIO_O_ADDR + gpionum, (val ? 1 : 0));
        }
    }
}

void gpio_disable(int gpionum)
{
    gpio_mux(gpionum, PAD_AS_MASTER);
}

//-----------------------------------------------------------------------------
// GPIO 中断相关, 实现在 irq.c
//-----------------------------------------------------------------------------

/*
 * @@ END
 */

