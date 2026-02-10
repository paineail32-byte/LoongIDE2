/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * tick.c
 *
 * created: 2023-06-10
 *  author: Bian
 */

#include <stdio.h>
#include <stdint.h>
#include <larchintrin.h>

#include "rtthread.h"
#include "rthw.h"

#include "cpu.h"
#include "regdef.h"

#include "ls1c102.h"
#include "ls1c102_irq.h"

//-----------------------------------------------------------------------------

#define MICROSECONDS_PER_TICKS    2000              /* us per tick */

extern unsigned int cpu_frequency;

static volatile unsigned int delay_1us_insns;       /* 延时1us的指令数 */

static volatile unsigned int Clock_driver_ticks;    /* Clock ticks since initialization */

//-----------------------------------------------------------------------------

/*
 *  Clock_isr
 *
 *  This is the clock tick interrupt handler.
 */
 
extern rt_thread_t rt_current_thread;

static void ls1c102_ticker_isr(int vector, void *arg)
{
    ++Clock_driver_ticks;           /* 计数加 1 */

    if (RT_NULL != rt_current_thread)
    {
        rt_tick_increase();
    }
}

//-----------------------------------------------------------------------------

unsigned int get_clock_ticks(void)
{
    return Clock_driver_ticks;
}

/*
 * Clock_initialize
 */
void Clock_initialize(void)
{
    unsigned int tcfg;
    
    Clock_driver_ticks = 0;

    __csrwr_w(0, LA_CSR_TVAL);
    __csrwr_w(0, LA_CSR_CNTC);

    /**
     * 安装中断向量
     */
    ls1c102_install_isr(LS1C102_IRQ_TICKER,
                        ls1c102_ticker_isr,
                        NULL);

    tcfg = (cpu_frequency / 1000) * MICROSECONDS_PER_TICKS / 1000 / 4;
    tcfg <<= CSR_TCFG_VAL_SHIFT;
    tcfg |= CSR_TCFG_PERIOD | CSR_TCFG_EN;

    __csrwr_w(tcfg, LA_CSR_TCFG);

    rt_kprintf("\nClock: %i us per tick\n", MICROSECONDS_PER_TICKS);

    delay_1us_insns = cpu_frequency / 1000000;
}

//-----------------------------------------------------------------------------

/*
 * us 级延时
 */
void delay_us(int us)
{
#if 1
    /*
     * 用循环来实现 us 级延时
     */
    register unsigned int i, count;

  #if 1
    count = us * delay_1us_insns / 4;
  #else
    count = us * (cpu_frequency / 1000) / 4000;
  #endif

    for (i=0; i<count; i++)
    {
        asm volatile( "nop ; ");
    }

#else
    /*
     * XXX rdtimel.w 读出总是为 1?
     */
    register unsigned int startVal, endVal, curVal;

    asm volatile( "rdtimel.w %0, $r0 ; " : "=r"(startVal) );
    
    endVal = startVal + delay_1us_insns * us;
    
    while (1)
    {
        asm volatile( "rdtimel.w %0, $r0 ; " : "=r"(curVal) );
        
        /*
         * 防止数值溢出
         */
        if (((endVal > startVal) && (curVal >= endVal)) ||
            ((endVal < startVal) && (curVal < startVal) && (curVal >= endVal)))
            break;
    }

#endif
}

/*
 * ms 级延时
 */

extern rt_err_t rt_thread_sleep(rt_tick_t tick);

void delay_ms(int ms)
{
    register unsigned int startTicks, endTicks, curTicks;

    endTicks = ms * 1000 / MICROSECONDS_PER_TICKS;

    /*
     * 如果关中断, 调用 delay_us 延时
     */
    if (!endTicks || !(__csrrd_w(LA_CSR_CRMD) & CSR_CRMD_IE))
    {
        delay_us(ms * 1000);
        return;
    }

    if (RT_NULL != rt_current_thread)   // Running
    {
        rt_thread_sleep(endTicks);
        return;
    }

    startTicks = Clock_driver_ticks;
    endTicks  += startTicks;

    while (1)
    {
        curTicks = Clock_driver_ticks;

        /*
         * 防止数值溢出
         */
        if (((endTicks > startTicks) && (curTicks >= endTicks)) ||
            ((endTicks < startTicks) && (curTicks < startTicks) && (curTicks >= endTicks)))
            break;
    }
}

