/*
 * drv_gpio.c
 *
 * created: 2025-06-05
 *  author: 
 */

#include "ls1c203.h"
#include "ls1c203_io_hw.h"

#include "drv_gpio.h"

//-----------------------------------------------------------------------------

/**
 * 将 GPIOn 设置为复用
 */
int ls1c203_io_sel(int gpioNum, unsigned mux)
{
    if ((gpioNum >= 0) && (gpioNum <= 28))
    {
        register unsigned int regVal, regAddr = AFIO_SEL0_ADDR;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 4;   // AFIO_SEL1_ADDR;
        }

        gpioNum *= 2;
        regVal  = READ_REG32(regAddr);
        regVal &= ~(0x3 << gpioNum);
        regVal |= (mux & 0x3) << gpioNum;
        WRITE_REG32(regAddr, regVal);

        return 0;
    }

    return -1;
}

//-----------------------------------------------------------------------------

/**
 * GPIO位访问配置端口 - GPIOBit_CFG
 */
int ls1c203_io_cfg(int gpioNum, int cfg_mode)
{
    unsigned char cfg, mode;
    
    switch (cfg_mode)
    {
        case IO_IN_ANA:     cfg = 0; mode = 0; break;
        case IO_IN_FLOAT:   cfg = 1; mode = 0; break;
        case IO_IN_PULL:    cfg = 2; mode = 0; break;
        case IO_OUT_PULL:   cfg = 0; mode = 1; break;
        case IO_OUT_OC:     cfg = 1; mode = 1; break;
        default:            return -1;
    }

    if ((gpioNum >= 0) && (gpioNum <= 28))
    {
        register unsigned int regAddr = GPIOA_BASE;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;	// GPIOB_BASE
        }

        regAddr += 0x80 + gpioNum;
        WRITE_REG8(regAddr, ((cfg & 0x3) << 2) + (mode & 0x3));

        return 0;
    }

    return -1;
}

/**
 * GPIO位访问输入端口 - GPIOBit_IDR
 */
int ls1c203_gpio_in(int gpioNum)
{
    if ((gpioNum >= 0) && (gpioNum <= 28))
    {
        register unsigned int regAddr = GPIOA_BASE;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;	// GPIOB_BASE
        }

        regAddr += 0x90 + gpioNum;

        return READ_REG8(regAddr) & 0x1;
    }

    return -1;
}

/**
 * GPIO位访问输出端口 - GPIOBit_ODR
 */
int ls1c203_gpio_out(int gpioNum, int outVal)
{
    if ((gpioNum >= 0) && (gpioNum <= 28))
    {
        register unsigned int regAddr = GPIOA_BASE;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;	// GPIOB_BASE
        }

        regAddr += 0xA0 + gpioNum;
        WRITE_REG8(regAddr, outVal & 0x1);

        return 0;
    }

    return -1;
}

