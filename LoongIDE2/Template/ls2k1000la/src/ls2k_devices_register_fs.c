/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_devices_register_fs.c
 *
 * created: 2024-11-12
 *  author: 
 */

#include <stdio.h>
#include <time.h>

#include "bsp.h"

#include "ls2k_drv_io.h"

#include "ls2k_can.h"
#include "ls2k_hpet.h"
#include "ls2k_pwm.h"
#include "ls2k_rtc.h"
#include "ls2k_uart.h"
#include "ls2k_nand.h"
#include "ls2k_dc.h"

#include "ls2k_spi_bus.h"
#include "spi/gd25q80.h"

#include "ls2k_i2c_bus.h"
#include "i2c/ads1015.h"
#include "i2c/mcp4725.h"

//-----------------------------------------------------------------------------

extern int register_devicefs(const char *dev_name, const void *device,
                             const driver_ops_t *drv_ops, void *arg);

//-----------------------------------------------------------------------------
// …Ë±∏±Ì
//-----------------------------------------------------------------------------

#define DEVICE_ID_UART          0x0001
#define DEVICE_ID_SPISLAVE      0x0002
#define DEVICE_ID_I2CSLAVE      0x0004
#define DEVICE_ID_RTC           0x0008
#define DEVICE_ID_HPET          0x0100
#define DEVICE_ID_CAN           0x0200
#define DEVICE_ID_PWM           0x0400
#define DEVICE_ID_ADC           0x0800

//-----------------------------------------------------------------------------
// register all devices to filesystem
//-----------------------------------------------------------------------------

void register_all_devices(void)
{
#if BSP_USE_FS

    const char *dev_name;

    /**
     * UART
     */
#if BSP_USE_UART0
    dev_name = ls2k_uart_get_device_name(devUART0);
    register_devicefs(dev_name, devUART0, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART1
    dev_name = ls2k_uart_get_device_name(devUART1);
    register_devicefs(dev_name, devUART1, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART2
    dev_name = ls2k_uart_get_device_name(devUART2);
    register_devicefs(dev_name, devUART2, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART3
    dev_name = ls2k_uart_get_device_name(devUART3);
    register_devicefs(dev_name, devUART3, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART4
    dev_name = ls2k_uart_get_device_name(devUART4);
    register_devicefs(dev_name, devUART4, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART5
    dev_name = ls2k_uart_get_device_name(devUART5);
    register_devicefs(dev_name, devUART5, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART6
    dev_name = ls2k_uart_get_device_name(devUART6);
    register_devicefs(dev_name, devUART6, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART7
    dev_name = ls2k_uart_get_device_name(devUART7);
    register_devicefs(dev_name, devUART7, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART8
    dev_name = ls2k_uart_get_device_name(devUART8);
    register_devicefs(dev_name, devUART8, uart_drv_ops, NULL);
#endif
#if BSP_USE_UART9
    dev_name = ls2k_uart_get_device_name(devUART9);
    register_devicefs(dev_name, devUART9, uart_drv_ops, NULL);
#endif

    /**
     * SPI bus master
     */
#if BSP_USE_SPI0
    ls2k_spi_initialize(busSPI0);
#endif

    /**
     * SPI slave
     */
#if GD25Q80_DRV
    register_devicefs("spi0.norflash", busSPI0, gd25q80_drv_ops, NULL);
#endif

    /**
     * I2C bus master
     */
#if BSP_USE_I2C0
    ls2k_i2c_initialize(busI2C0);
#endif
#if BSP_USE_I2C1
    ls2k_i2c_initialize(busI2C1);
#endif

    /**
     * I2C slave
     */
#if ADS1015_DRV
    register_devicefs("i2c0.ads1015", busI2C0, ads1015_drv_ops, NULL);
#endif
#if MCP4725_DRV
    register_devicefs("i2c0.mcp4725", busI2C0, mcp4725_drv_ops, NULL);
#endif

    /**
     * RTC
     */
#if BSP_USE_RTC
    register_devicefs("rtc", devRTC, rtc_drv_ops, NULL);
#endif

    /**
     * HPET
     */
#if BSP_USE_HPET0
    register_devicefs("hpet0", devHPET0, hpet_drv_ops, NULL);
#endif

    /**
     * CAN
     */
#if BSP_USE_CAN0
    dev_name = ls2k_can_get_device_name(devCAN0);
    register_devicefs(dev_name, devCAN0, can_drv_ops, NULL);
#endif
#if BSP_USE_CAN1
    dev_name = ls2k_can_get_device_name(devCAN1);
    register_devicefs(dev_name, devCAN1, can_drv_ops, NULL);
#endif

    /**
     * PWM
     */
#if BSP_USE_PWM0
    dev_name = ls2k_pwm_get_device_name(devPWM0);
    register_devicefs(dev_name, devPWM0, pwm_drv_ops, NULL);
#endif
#if BSP_USE_PWM1
    dev_name = ls2k_pwm_get_device_name(devPWM1);
    register_devicefs(dev_name, devPWM1, pwm_drv_ops, NULL);
#endif
#if BSP_USE_PWM2
    dev_name = ls2k_pwm_get_device_name(devPWM2);
    register_devicefs(dev_name, devPWM2, pwm_drv_ops, NULL);
#endif
#if BSP_USE_PWM3
    dev_name = ls2k_pwm_get_device_name(devPWM3);
    register_devicefs(dev_name, devPWM3, pwm_drv_ops, NULL);
#endif

    /**
     * NAND
     */
#if BSP_USE_NAND
    // register with yaffs filesystem, not device filesystem
#endif

    /**
     * Display Control
     */
#if BSP_USE_DC
    register_devicefs("fb0", devDC0, dc_drv_ops, NULL);
#endif

#endif // #if BSP_USE_FS
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

