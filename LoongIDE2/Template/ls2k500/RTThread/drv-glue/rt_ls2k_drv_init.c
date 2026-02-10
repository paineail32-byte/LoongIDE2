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
#if BSP_USE_I2C

#endif

/*
 * SPI 从设备
 */
#if BSP_USE_SPI
#ifdef W25Q32_DRV
#include "spi/drv_w25q32.h"         // SPI Flash
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
    #if MFRC522_DRV
        //
    #endif

    /*
     * SPI 从设备
     */
    #if W25Q32_DRV
        rt_ls2k_w25q32_install();       // SPI Flash
    #endif

    return 0;
}

