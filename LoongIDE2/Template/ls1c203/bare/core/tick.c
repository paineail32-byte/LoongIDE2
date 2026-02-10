/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * tick.c
 *
 * created: 2026-06-5
 *  author: Bian
 */

#include <stdio.h>
#include <stdint.h>
#include <larchintrin.h>

#include "cpu.h"
#include "regdef.h"

#include "ls1c203.h"
#include "ls1c203_irq.h"

extern void printk(const char *fmt, ...);

//-----------------------------------------------------------------------------

#define TICKS_PER_SECOND    1000

extern unsigned int cpu_frequency;

static volatile unsigned int delay_1us_count;       /* hda 延时1us的计数值 */

volatile unsigned int Clock_driver_ticks;    /* Clock ticks since initialization */

//-----------------------------------------------------------------------------

__WEAK unsigned int get_clock_ticks(void)
{
    return Clock_driver_ticks;
}

/*
 *  Clock_isr
 *
 *  This is the clock tick interrupt handler.
 */
__WEAK void ls1c203_ticker_isr(int vector, void *arg)
{
    ++Clock_driver_ticks;           /* 计数加 1 */
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
    ls1c203_install_isr(LS1C203_IRQ_TICKER,
                        ls1c203_ticker_isr,
                        NULL);

    tcfg = cpu_frequency / TICKS_PER_SECOND;
    // tcfg <<= CSR_TCFG_VAL_SHIFT;
    tcfg |= CSR_TCFG_PERIOD | CSR_TCFG_EN;

    __csrwr_w(tcfg, LA_CSR_TCFG);

    printk("\nClock: %i per second\n", TICKS_PER_SECOND);

    delay_1us_count = cpu_frequency / 1000000;
}

/*
 * TODO 限制 us
 */
void delay_us(int us)
{
    register unsigned int startVal, endVal, curVal;

    asm volatile( "rdtimel.w %0, $r0 ; " : "=r"(startVal) );
    
    endVal = startVal + delay_1us_count * us;
    
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
}

void delay_ms(int ms)
{
    register unsigned int startTicks, endTicks, curTicks;

    /*
     * 如果关中断, 调用 delay_us 延时
     */
    if ((__csrrd_w(LA_CSR_CRMD) & CSR_CRMD_IE))
    {
        startTicks = Clock_driver_ticks;
        endTicks   = startTicks + ms * TICKS_PER_SECOND / 1000;

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
    else
    {
        delay_us(ms * 1000);
    }
}


