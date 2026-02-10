/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K500_GPIO_H
#define _LS2K500_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO方向
 */
#define GPIO_IN         1           /* GPIO 输入 */
#define GPIO_OUT        0           /* GPIO 输出 */

/*
 * 引脚复用
 */
#define GPIO_AS_GPIO    0           /* 引脚复用为GPIO */
#define GPIO_AS_MUX1    1           /* 引脚复用1  */
#define GPIO_AS_MUX2    2           /* 引脚复用2  */
#define GPIO_AS_MUX3    3           /* 引脚复用3  */
#define GPIO_AS_MUX4    4           /* 引脚复用4  */
#define GPIO_AS_PAD     5           /* 引脚主功能 */

//-------------------------------------------------------------------------------------------------
// GPIO 基本操作函数
//-------------------------------------------------------------------------------------------------

/*
 * GPIO 端口复用
 * 参数: 	gpionum   gpio端口序号
 *        	mux       复用编号
 *
 */
void gpio_mux(int gpionum, int mux);

/*
 * 使能GPIO端口
 * 参数: 	gpionum   gpio端口序号
 *        	dir_in    gpio方向. ~0: 输入, 0: 输出
 */
void gpio_enable(int gpionum, int dir_in);

/*
 * 读GPIO端口, 该GPIO被设置为输入模式
 * 参数: 	gpionum   gpio端口序号
 * 返回: 	0或者1
 */
int gpio_read(int gpionum);

/*
 * 写GPIO端口, 该GPIO被设置为输出模式
 * 参数:  	gpionum   gpio端口序号
 *        	val       0或者1
 */
void gpio_write(int gpionum, int val);

/*
 * 关闭GPIO功能, 端口恢复默认设置
 * 参数:  	gpionum   gpio端口序号
 */
void gpio_disable(int gpionum);

//-------------------------------------------------------------------------------------------------
// GPIO 中断相关
//-------------------------------------------------------------------------------------------------

#define GPIO_INT_TRIG_EDGE_UP       0x01        /* 上升沿触发 gpio 中断 */
#define GPIO_INT_TRIG_EDGE_DOWN     0x02        /* 下降沿触发 gpio 中断 */
#define GPIO_INT_TRIG_LEVEL_HIGH    0x04        /* 高电平触发 gpio 中断 */
#define GPIO_INT_TRIG_LEVEL_LOW     0x08        /* 低电平触发 gpio 中断 */

/*
 * 使能GPIO中断
 * 参数:		gpio            gpio端口序号
 *         	trigger_mode    中断触发模式, 见上定义
 */
extern int ls2k500_gpio_interrupt_enable(int gpionum, int trigger_mode);

/*
 * 禁止GPIO中断
 * 参数: 	gpio    gpio端口序号
 */
extern int ls2k500_gpio_interrupt_disable(int gpionum);

#ifdef __cplusplus
}
#endif

#endif // _LS2K500_GPIO_H

