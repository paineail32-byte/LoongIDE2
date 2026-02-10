/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * tick.c
 *
 * created: 2024-07-16
 *  author: Bian
 */

#include <stdio.h>
#include <stdint.h>
#include <larchintrin.h>

#include "bsp.h"

#include "rtthread.h"
#include "rthw.h"

#include "cpu.h"
#include "regdef.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

extern void printk(const char *fmt, ...);

//-------------------------------------------------------------------------------------------------

#define TICKS_PER_SECOND    RT_TICK_PER_SECOND  // 1000

extern unsigned int cpu_frequency;              /* 用于tick */

static unsigned int hda_frequency;              /* 用于delay */

static unsigned int hda_1us_count = 30;         /* hda 延时1us的计数值, 默认值的作用: delay_us */

static volatile uint64_t Clock_driver_ticks;    /* Clock ticks since initialization */

//-------------------------------------------------------------------------------------------------

uint64_t get_clock_ticks(void)
{
    return Clock_driver_ticks;
}

/*
 * Clock_isr
 *
 * This is the clock tick interrupt handler.
 */
static void Clock_isr(int vector, void *arg)
{
    ++Clock_driver_ticks;           /* 计数加 1 */

    if (rt_thread_os_running)
    {
        rt_tick_increase();
    }
}

/*
 * Clock_initialize
 */
void Clock_initialize(void)
{
    uint64_t tcfg;
    
    Clock_driver_ticks = 0;

    rt_tick_sethook(RT_NULL);       /* AVOID interrupt brfore RTThread initialized */

    /* install then Clock isr
     */
    ls2k_install_irq_handler(LS2K300_IRQ_TIMER, Clock_isr, 0);

    __csrwr_d(0, LA_CSR_TVAL);       /* 计数器清零 */
    __csrwr_d(0, LA_CSR_CNTC);

    /*
     * 计时频率.
     */
    #if 1
    {
        unsigned long mcsr2;
        unsigned int mul, div;

        mcsr2 = __csrrd_d(LA_CSR_MCSR2);
        hda_frequency = (unsigned int)(mcsr2 & MCSR2_CCFREQ_MASK);
        mul = (mcsr2 >> MCSR2_CCMUL_SHIFT) & 0xFFFF;
        div = (mcsr2 >> MCSR2_CCDIV_SHIFT) & 0xFFFF;
        if (div && mul)
        {
            hda_frequency = hda_frequency * mul / div;
        }

        hda_frequency >>= 2;        /* 4 分频: 120M / 4 == 30M */
    }
    #else
    {
        hda_frequency = 30000000;
    }
    #endif

    tcfg = hda_frequency / TICKS_PER_SECOND;
    tcfg <<= CSR_TCFG_VAL_SHIFT;
    tcfg |= CSR_TCFG_PERIOD | CSR_TCFG_EN;

    __csrwr_d(tcfg, LA_CSR_TCFG);

    printk("\r\nClock: %i ticks per second\r\n", TICKS_PER_SECOND);

    hda_1us_count = hda_frequency / 1000000;
}

//-------------------------------------------------------------------------------------------------

/*
 * TODO 限制 us
 */
void delay_us(int us)
{
    volatile uint64_t startVal, endVal, curVal;

    asm volatile( "rdtime.d %0, $r0 ; " : "=r"(startVal) );
    
    endVal = startVal + hda_1us_count * us;
    
    while (1)
    {
        asm volatile( "rdtime.d %0, $r0 ; " : "=r"(curVal) );
        
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
    volatile uint64_t startTicks, endTicks, curTicks;

    /*
     * 如果关中断, 调用 delay_us 延时
     */
    if ((__csrrd_d(LA_CSR_CRMD) & CSR_CRMD_IE) == 0)
    {
        delay_us(ms * 1000);
        return;
    }

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

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */

