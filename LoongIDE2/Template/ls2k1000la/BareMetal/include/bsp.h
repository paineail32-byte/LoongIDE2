/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp.h
 *
 * created: 2022-03-01
 *  author: Bian
 */

#ifndef _BSP_H
#define _BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_PESUDO
#define BSP_USE_OS          1
#else
#define BSP_USE_OS          0
#endif

//-----------------------------------------------------------------------------
// debug
//-----------------------------------------------------------------------------

#if 1
#define DBG_OUT(...)        printk(__VA_ARGS__)
#else
#define DBG_OUT(...)
#endif

//-----------------------------------------------------------------------------
// bus frequency
//-----------------------------------------------------------------------------

extern unsigned apb_frequency;

//-----------------------------------------------------------------------------
// device used
//-----------------------------------------------------------------------------

/**
 * UART
 */
#define BSP_USE_UART0   1
#define BSP_USE_UART1   1
#define BSP_USE_UART2   1
#define BSP_USE_UART3   1
#define BSP_USE_UART4   0
#define BSP_USE_UART5   0
#define BSP_USE_UART6   0
#define BSP_USE_UART7   0
#define BSP_USE_UART8   0
#define BSP_USE_UART9   0
#define BSP_USE_UART10  0
#define BSP_USE_UART11  0

//---------
// Mdbus
//---------

#define	USE_MODBUS		0

/**
 * SPI
 */
#define BSP_USE_SPI0    1

#define BSP_USE_SPI     BSP_USE_SPI0

#if BSP_USE_SPI0
#define GD25Q80_DRV     1           // NOR Flash 1M
#endif

/**
 * I2C
 */
#define BSP_USE_I2C0    1
#define BSP_USE_I2C1    1

#define BSP_USE_I2C     (BSP_USE_I2C0 || BSP_USE_I2C1)

#if BSP_USE_I2C0
#define ADS1015_DRV     1
#define MCP4725_DRV     1
#define IS24C16_DRV     1           // I2C flash 16K
#endif

/**
 * NAND
 */
#define BSP_USE_NAND    0
#if BSP_USE_NAND
#define USE_YAFFS2      0
#endif

/**
 * RTC
 */
#define BSP_USE_RTC     1

/**
 * HPET
 */
#define BSP_USE_HPET0   1

#define BSP_USE_HPET    BSP_USE_HPET0

/**
 * CAN
 */
#define BSP_USE_CAN0    1
#define BSP_USE_CAN1    1

#define BSP_USE_CAN     (BSP_USE_CAN0 || BSP_USE_CAN1)

/**
 * GMAC
 */
#define BSP_USE_GMAC0   1
#define BSP_USE_GMAC1   0

#define BSP_USE_GMAC    (BSP_USE_GMAC0 || BSP_USE_GMAC1)

#if BSP_USE_GMAC
#define USE_LWIP        1
#endif

/**
 * PWM
 */
#define BSP_USE_PWM0    1
#define BSP_USE_PWM1    1
#define BSP_USE_PWM2    1
#define BSP_USE_PWM3    1

#define BSP_USE_PWM     (BSP_USE_PWM0 || BSP_USE_PWM1 || BSP_USE_PWM2 || BSP_USE_PWM3)

/**
 * Display Control
 */
#define BSP_USE_DC      1

//-----------------------------------------------------------------------------
// filesystem
//-----------------------------------------------------------------------------

#define BSP_USE_FS      0

#if BSP_USE_FS

#define BSP_USE_USB     0

#define BSP_USE_SATA    0

#endif // BSP_USE_FS

//-----------------------------------------------------------------------------
// Shell
//-----------------------------------------------------------------------------

#define BSP_USE_SHELL   1

//*****************************************************************************
//-----------------------------------------------------------------------------
// This function print to console directly
//-----------------------------------------------------------------------------

extern void printk(const char *fmt, ...);

extern void print_hex(char *p, int size);

extern void delay_us(int us);
extern void delay_ms(int ms);

extern unsigned long get_clock_ticks(void);

//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // _BSP_H

/*
 * @@ END
 */

