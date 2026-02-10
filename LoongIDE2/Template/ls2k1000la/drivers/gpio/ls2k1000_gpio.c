/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k1000_gpio.c
 *
 * created: 2022-10-12
 *  author: Bian
 */

#include "ls2k1000.h"

#include "ls2k1000_gpio.h"

/**
 * GPIO44~51 只能用作输出
 */
int gpio_enable(int gpionum, int dir_in)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        if (dir_in)
        {
            if ((gpionum >= 44) && (gpionum <= 51))
                return -1;
                
            OR_REG64(GPIO_OEN_BASE, 1ul<<gpionum);        // set 1 as in
        }
        else
            AND_REG64(GPIO_OEN_BASE, ~(1ul<<gpionum));    // set 0 as out

        return 0;
    }
    
    return -1;
}

int gpio_read(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        unsigned long r_in = READ_REG64(GPIO_IN_BASE);
        return (int)((r_in >> gpionum) & 0x1);
    }

    return -1;
}

void gpio_write(int gpionum, int val)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        if (val)
            OR_REG64(GPIO_OUT_BASE, 1ul<<gpionum);
        else
            AND_REG64(GPIO_OUT_BASE, ~(1ul<<gpionum));
    }
}

int gpio_int_enable(int gpionum, int en)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        if (en)
            OR_REG64(GPIO_INTEN_BASE, 1ul<<gpionum);
        else
            AND_REG64(GPIO_INTEN_BASE, ~(1ul<<gpionum));
            
        return 0;
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------
//
// PAD 复用
//
// 参数:    mux     dev_mux_t类型
//
//-------------------------------------------------------------------------------------------------
 
int ls2k1000_pad_mux(dev_mux_t mux)
{
    switch (mux)
    {
        case gmac1_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_GMAC1_SEL);
            break;
        case gmac1_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_GMAC1_SEL);
            break;
        
        case sata_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_SATA_SEL);
            break;
        case sata_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_SATA_SEL);
            break;

        case i2c0_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_I2C0_SEL);
            break;
        case i2c0_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_I2C0_SEL);
            break;

        case i2c1_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_I2C1_SEL);
            break;
        case i2c1_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_I2C1_SEL);
            break;

        case pwm0_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_PWM0_SEL);
            break;
        case pwm0_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_PWM0_SEL);
            break;

        case pwm1_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_PWM1_SEL);
            break;
        case pwm1_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_PWM1_SEL);
            break;

        case pwm2_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_PWM2_SEL);
            break;
        case pwm2_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_PWM2_SEL);
            break;

        case pwm3_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_PWM3_SEL);
            break;
        case pwm3_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_PWM3_SEL);
            break;

        case hda_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_HDA_SEL);
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_I2S_SEL);
            break;

        case hda_as_i2s:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_HDA_SEL);
            OR_REG64(CHIP_CFG0_BASE, CFG0_I2S_SEL);
            break;
            
        case hda_as_pad:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_I2S_SEL);
            OR_REG64(CHIP_CFG0_BASE, CFG0_HDA_SEL);
            break;

        case can0_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_CAN0_SEL);
            break;
        case can0_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_CAN0_SEL);
            break;

        case can1_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_CAN1_SEL);
            break;
        case can1_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_CAN1_SEL);
            break;

        case sdio_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_SDIO_SEL);
            break;
        case sdio_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_SDIO_SEL);
            break;

        case nand_as_gpio:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_NAND_SEL);
            break;
        case nand_as_pad:
            OR_REG64(CHIP_CFG0_BASE, CFG0_NAND_SEL);
            break;

        /*
         * 以下非GPIO的复用
         */
        case dvo1_as_camera:
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_DVO1_SEL);
            OR_REG64(CHIP_CFG2_BASE, CFG2_CAM_SEL);
            break;
        case dvo1_as_pad:
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_CAM_SEL);
            OR_REG64(CHIP_CFG2_BASE, CFG2_DVO1_SEL);
            break;
            
        case uart0_as_4line:
            AND_REG64(CHIP_CFG1_BASE, ~CFG1_UART0_MODE_MASK);
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART0_4);
            break;
        case uart0_as_2line:
            AND_REG64(CHIP_CFG1_BASE, ~CFG1_UART0_MODE_MASK);
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART0_2);
            break;
        case uart0_as_pad:
            AND_REG64(CHIP_CFG1_BASE, ~CFG1_UART0_MODE_MASK);
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART0_8);
            break;

        case dvo0_as_lio:
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_DVO0_SEL);
            AND_REG64(CHIP_CFG1_BASE, ~(CFG1_UART1_SEL | CFG1_UART2_SEL));
            OR_REG64(CHIP_CFG0_BASE, CFG0_LIO_SEL);
            break;
        case dvo0_as_uart12_8line:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_LIO_SEL);
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_DVO0_SEL);
            AND_REG64(CHIP_CFG1_BASE, ~(CFG1_UART1_MODE_MASK | CFG1_UART2_MODE_MASK));
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART1_SEL | CFG1_UART2_SEL | CFG1_UART1_8 | CFG1_UART2_8);
            break;
        case dvo0_as_uart12_4line:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_LIO_SEL);
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_DVO0_SEL);
            AND_REG64(CHIP_CFG1_BASE, ~(CFG1_UART1_MODE_MASK | CFG1_UART2_MODE_MASK));
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART1_SEL | CFG1_UART2_SEL | CFG1_UART1_4 | CFG1_UART2_4);
            break;
        case dvo0_as_uart12_2line:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_LIO_SEL);
            AND_REG64(CHIP_CFG2_BASE, ~CFG2_DVO0_SEL);
            AND_REG64(CHIP_CFG1_BASE, ~(CFG1_UART1_MODE_MASK | CFG1_UART2_MODE_MASK));
            OR_REG64(CHIP_CFG1_BASE, CFG1_UART1_SEL | CFG1_UART2_SEL | CFG1_UART1_2 | CFG1_UART2_2);
            break;
        case dvo0_as_pad:
            AND_REG64(CHIP_CFG0_BASE, ~CFG0_LIO_SEL);
            AND_REG64(CHIP_CFG1_BASE, ~(CFG1_UART1_SEL | CFG1_UART2_SEL));
            OR_REG64(CHIP_CFG2_BASE, CFG2_DVO0_SEL);
            break;
    }

    return 0;
}


