/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K1000_GPIO_H
#define _LS2K1000_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO方向
 */
#define GPIO_IN         1           /* GPIO 输入 */
#define GPIO_OUT        0           /* GPIO 输出 */

#define GPIO_COUNT      64

//-------------------------------------------------------------------------------------------------
// GPIO 基本操作函数
//-------------------------------------------------------------------------------------------------

/*
 * 使能GPIO端口
 * 参数:    gpionum   gpio端口序号
 *          dir_in    gpio方向. ~0: 输入, 0: 输出
 * 返回:    0=成功
 */
int gpio_enable(int gpionum, int dir_in);

/*
 * 读GPIO端口, 该GPIO被设置为输入模式
 * 参数:    gpionum   gpio端口序号
 * 返回:    0或者1
 */
int gpio_read(int gpionum);

/*
 * 写GPIO端口, 该GPIO被设置为输出模式
 * 参数:    gpionum   gpio端口序号
 *          val       0或者1
 */
void gpio_write(int gpionum, int val);

/*
 * 使能GPIO中断
 */
int gpio_int_enable(int gpionum, int en);

//-------------------------------------------------------------------------------------------------
// LS2K1000 的 PAD 复用, 注意是成组复用
//-------------------------------------------------------------------------------------------------

typedef enum
{
    gmac1_as_gpio        = 0x01,        /* GPIO04~13 */
    sata_as_gpio         = 0x02,        /* GPIO14 */
    i2c0_as_gpio         = 0x03,        /* GPIO16~17 */
    i2c1_as_gpio         = 0x04,        /* GPIO18~19 */
    pwm0_as_gpio         = 0x05,        /* GPIO20 */
    pwm1_as_gpio         = 0x06,        /* GPIO21 */
    pwm2_as_gpio         = 0x07,        /* GPIO22 */
    pwm3_as_gpio         = 0x08,        /* GPIO23 */
    hda_as_gpio          = 0x09,        /* GPIO24~30 */
    hda_as_i2s           = 0x0A,
    can0_as_gpio         = 0x0B,        /* GPIO32~33 */
    can1_as_gpio         = 0x0C,        /* GPIO34~35 */
    sdio_as_gpio         = 0x0D,        /* GPIO36~41 */
    nand_as_gpio         = 0x0E,        /* GPIO44~63 */

    gmac1_as_pad         = 0x11,
    sata_as_pad          = 0x12,
    i2c0_as_pad          = 0x13,
    i2c1_as_pad          = 0x14,
    pwm0_as_pad          = 0x15,
    pwm1_as_pad          = 0x16,
    pwm2_as_pad          = 0x17,
    pwm3_as_pad          = 0x18,
    hda_as_pad           = 0x19,
    can0_as_pad          = 0x1B,
    can1_as_pad          = 0x1C,
    sdio_as_pad          = 0x1D,
    nand_as_pad          = 0x1E,

    dvo1_as_camera       = 0x21,
    uart0_as_4line       = 0x22,
    uart0_as_2line       = 0x23,
    dvo0_as_lio          = 0x24,
    dvo0_as_uart12_8line = 0x25,
    dvo0_as_uart12_4line = 0x26,
    dvo0_as_uart12_2line = 0x27,

    dvo1_as_pad          = 0x31,
    uart0_as_pad         = 0x32,
    dvo0_as_pad          = 0x34,
} dev_mux_t;

/*
 * PAD 复用
 * 参数:    mux     dev_mux_t类型
 */
int ls2k1000_pad_mux(dev_mux_t mux);


#ifdef __cplusplus
}
#endif

#endif // _LS2K1000_GPIO_H

