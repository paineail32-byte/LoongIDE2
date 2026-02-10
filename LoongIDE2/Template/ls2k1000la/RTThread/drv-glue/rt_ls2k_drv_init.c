/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#include "bsp.h"

#include "drv_uart.h"

#if BSP_USE_CAN
#include "drv_can.h"
#endif

#if BSP_USE_NAND
#include "drv_nand.h"
#endif

#if BSP_USE_DC
#include "drv_dc.h"
#endif

#if BSP_USE_PWM
#include "drv_pwm.h"
#endif

/*
 * I2C 从设备
 */
#if BSP_USE_I2C0
#if IS24C16_DRV
#include "i2c/drv_is24c16.h"
#endif
#endif

/*
 * SPI 从设备
 */
#if BSP_USE_SPI
#if GD25Q80_DRV
#include "spi/drv_gd25q80.h"        // SPI Flash
#endif
#endif

//-----------------------------------------------------------------------------
// Initialize drivers according BSP's Configuration
//-----------------------------------------------------------------------------

int rt_ls2k_drv_init(void)
{
    rt_ls2k_uart_install();

    #if BSP_USE_CAN
        rt_ls2k_can_install();
    #endif

    #if BSP_USE_NAND
        rt_ls2k_nand_install();
    #endif

    #if BSP_USE_DC
        rt_ls2k_dc_install();
    #endif

    #if BSP_USE_PWM
        rt_ls2k_pwm_install();
    #endif

    /*
     * I2C 从设备
     */
    #if IS24C16_DRV
        rt_ls2k_is24c16_install();      // EEPROM
    #endif

    /*
     * SPI 从设备
     */
    #if GD25Q80_DRV
        rt_ls2k_gd25q80_install();      // SPI Flash
    #endif

    return 0;
}

