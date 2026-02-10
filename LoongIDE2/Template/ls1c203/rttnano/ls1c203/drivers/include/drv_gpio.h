/*
 * drv_gpio.h
 *
 * created: 2025-06-05
 *  author: 
 */

#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// 端口配置
//-----------------------------------------------------------------------------

#define IO_IN_ANA			0x0			// 模拟输入模式
#define IO_IN_FLOAT			0x1			// 浮空输入模式
#define IO_IN_PULL			0x2			// 上拉/下拉输入模式

#define IO_OUT_PULL			0x4			// 推挽输出模式
#define IO_OUT_OC			0x8			// 开漏输出模式

/*
 * GPIO位访问配置端口 - GPIOBit_CFG
 */
int ls1c203_io_cfg(int gpioNum, int cfg_mode);

//-----------------------------------------------------------------------------
// io 端口功能选择 - 复用
//-----------------------------------------------------------------------------

#define IOSEL_GPIO			0
#define IOSEL_MAIN			1
#define IOSEL_MUX1			2
#define IOSEL_MUX2			3

int ls1c203_io_sel(int gpioNum, unsigned mux);

//-----------------------------------------------------------------------------
// gpio 输入输出
//-----------------------------------------------------------------------------

/*
 * GPIO位访问输入端口 - GPIOBit_IDR
 */
int ls1c203_gpio_in(int gpioNum);

/*
 * GPIO位访问输出端口 - GPIOBit_ODR
 */
int ls1c203_gpio_out(int gpioNum, int outVal);


#ifdef __cplusplus
}
#endif

#endif // _DRV_GPIO_H
