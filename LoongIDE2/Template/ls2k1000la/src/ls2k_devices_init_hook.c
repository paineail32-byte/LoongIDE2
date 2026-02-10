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
#if BSP_USE_UART10
    // if (dev == devUART10)
#endif
#if BSP_USE_UART11
    // if (dev == devUART11)
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

