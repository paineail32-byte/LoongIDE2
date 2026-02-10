/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef _BSP_H
#define _BSP_H

//-----------------------------------------------------------------------------
// 全局变量
//-----------------------------------------------------------------------------

extern unsigned int bus_frequency;              // 总线工作频率

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

#define BSP_USE_UART0       1
#define BSP_USE_UART1       0

//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------

extern void delay_us(unsigned int us);
extern void delay_ms(unsigned int ms);

extern unsigned int get_clock_ticks(void);

extern int ls1c203_install_isr(int vector, void (*isr)(int, void *), void *arg);
extern int ls1c203_remove_isr(int vector);

extern int ls1c203_interrupt_enable(int vector);
extern int ls1c203_interrupt_disable(int vector);


#endif // _BSP_H

