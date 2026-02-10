/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_devices_init_hook.c
 *
 * created: 2024-10-30
 *  author: 
 */

/*
 * 片上设备初始化时调用, 用于实现引脚复用等的设置
 */

#include <stdint.h>
#include <stddef.h>

#include "bsp.h"

#include "ls2k_can.h"
#include "ls2k_hpet.h"
#include "ls2k_pwm.h"
#include "ls2k_rtc.h"
#include "ls2k_uart.h"
#include "ls2k_nand.h"
#include "ls2k_dc.h"

#include "ls2k_spi_bus.h"
#include "ls2k_i2c_bus.h"

//-----------------------------------------------------------------------------

/******************************************************************************
 * UART is always use
 */
int ls2k_uart_init_hook(const void *dev)
{
#if BSP_USE_UART0
    // if (dev == devUART0)
#endif
#if BSP_USE_UART1
    // if (dev == devUART1)
#endif
#if BSP_USE_UART2
    // if (dev == devUART2)
#endif
#if BSP_USE_UART3
    // if (dev == devUART3)
#endif
#if BSP_USE_UART4
    // if (dev == devUART4)
#endif
#if BSP_USE_UART5
    // if (dev == devUART5)
#endif
#if BSP_USE_UART6
    // if (dev == devUART6)
#endif
#if BSP_USE_UART7
    // if (dev == devUART7)
#endif
#if BSP_USE_UART8
    // if (dev == devUART8)
#endif
#if BSP_USE_UART9
    // if (dev == devUART9)
#endif

    return 0;
}

/******************************************************************************
 * SPI bus master
 */
#if BSP_USE_SPI
int ls2k_spi_init_hook(const void *bus)
{
#if BSP_USE_SPI0
    // if (bus == busSPI0)
#endif
#if BSP_USE_SPI1
    // if (bus == busSPI1)
#endif
#if BSP_USE_SPI2
    // if (bus == busSPI2)
#endif
#if BSP_USE_SPI3
    // if (bus == busSPI3)
#endif
#if BSP_USE_SPI4
    // if (bus == busSPI4)
#endif
#if BSP_USE_SPI5
    // if (bus == busSPI5)
#endif

    return 0;
}
#endif // #if BSP_USE_SPI

/******************************************************************************
 * I2C bus master
 */
#if BSP_USE_I2C
int ls2k_i2c_init_hook(const void *bus)
{
#if BSP_USE_I2C0
    // if (bus == busI2C0)
#endif
#if BSP_USE_I2C1
    // if (bus == busI2C1)
#endif
#if BSP_USE_I2C2
    // if (bus == busI2C2)
#endif
#if BSP_USE_I2C3
    // if (bus == busI2C3)
#endif
#if BSP_USE_I2C4
    // if (bus == busI2C4)
#endif
#if BSP_USE_I2C5
    // if (bus == busI2C5)
#endif

    return 0;
}
#endif // #if BSP_USE_I2C

/******************************************************************************
 * CAN
 */
#if BSP_USE_CAN
int ls2k_can_init_hook(const void *dev)
{
#if BSP_USE_CAN0
    // if (dev == devCAN0)
#endif
#if BSP_USE_CAN1
    // if (dev == devCAN1)
#endif
#if BSP_USE_CAN2
    // if (dev == devCAN2)
#endif
#if BSP_USE_CAN3
    // if (dev == devCAN3)
#endif

    return 0;
}
#endif // #if BSP_USE_CAN

/******************************************************************************
 * PWM
 */
#if BSP_USE_PWM
int ls2k_pwm_init_hook(const void *dev)
{
#if BSP_USE_PWM0
    // if (dev == devPWM0)
#endif
#if BSP_USE_PWM1
    // if (dev == devPWM1)
#endif
#if BSP_USE_PWM2
    // if (dev == devPWM2)
#endif
#if BSP_USE_PWM3
    // if (dev == devPWM3)
#endif
#if BSP_USE_PWM4
    // if (dev == devPWM4)
#endif
#if BSP_USE_PWM5
    // if (dev == devPWM5)
#endif
#if BSP_USE_PWM6
    // if (dev == devPWM6)
#endif
#if BSP_USE_PWM7
    // if (dev == devPWM7)
#endif
#if BSP_USE_PWM8
    // if (dev == devPWM8)
#endif
#if BSP_USE_PWM9
    // if (dev == devPWM9)
#endif
#if BSP_USE_PWM10
    // if (dev == devPWM10)
#endif
#if BSP_USE_PWM11
    // if (dev == devPWM11)
#endif
#if BSP_USE_PWM12
    // if (dev == devPWM12)
#endif
#if BSP_USE_PWM13
    // if (dev == devPWM13)
#endif
#if BSP_USE_PWM14
    // if (dev == devPWM14)
#endif
#if BSP_USE_PWM15
    // if (dev == devPWM15)
#endif

    return 0;
}
#endif // #if BSP_USE_PWM

/******************************************************************************
 * Display Control
 */
#if BSP_USE_DC
int ls2k_dc_init_hook(const void *dev)
{
    return 0;
}
#endif

/******************************************************************************
 * GMAC
 */
#if BSP_USE_GMAC
int ls2k_gmac_init_hook(const void *dev)
{
#if BSP_USE_GMAC0
    // extern void *devGMAC0;
    // if (dev == devGMAC0)
#endif
#if BSP_USE_GMAC1
    // extern void *devGMAC1;
    // if (dev == devGMAC1)
#endif

    return 0;
}
#endif // #if BSP_USE_GMAC

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

