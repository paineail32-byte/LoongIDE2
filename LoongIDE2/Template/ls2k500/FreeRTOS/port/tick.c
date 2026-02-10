/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * tick.c
 *
 * created: 2022-02-18
 *  author: Bian
 */

#include <stdio.h>
#include <stdint.h>
#include <larchintrin.h>

#include "cpu.h"
#include "regdef.h"

#include "ls2k500.h"
#include "ls2k500_irq.h"

#include "FreeRTOSConfig.h"

extern void printk(const char *fmt, ...);

//-----------------------------------------------------------------------------

#define TICKS_PER_SECOND    configTICK_RATE_HZ  // 1000

/*
 * LA_CSR_MCSR2
 */
static unsigned int cc_frequency;
static unsigned int cc_1us_count = 25;         	/* 延时1us的计数值 */

static volatile uint64_t Clock_driver_ticks;    /* Clock ticks since initialization */

//-----------------------------------------------------------------------------
// FreeRTOS glue
//-----------------------------------------------------------------------------

extern void vPortIncrementTick(int vector, void *arg);

void vApplicationTickHook(void)
{
    ++Clock_driver_ticks;           /* 计数加 1 */
}

//-----------------------------------------------------------------------------

uint64_t get_clock_ticks(void)
{
    return Clock_driver_ticks;
}

/*
 * Clock_initialize
 */
void Clock_initialize(void)
{
    uint64_t tcfg, cc;
    unsigned int mul, div;

    cc = __csrrd_d(LA_CSR_MCSR2);
    cc_frequency = (unsigned int)(cc & MCSR2_CCFREQ_MASK);
    mul = (cc >> MCSR2_CCMUL_SHIFT) & 0xFFFF;
    div = (cc >> MCSR2_CCDIV_SHIFT) & 0xFFFF;
    if (div && mul)
    {
        cc_frequency = cc_frequency * mul / div;
    }
    cc_frequency /= 4;          /* XXX 必须 4 分频, 手册没有介绍? */

    Clock_driver_ticks = 0;

    /* install then Clock isr
     */
    ls2k_install_irq_handler(LS2K500_IRQ_TIMER, vPortIncrementTick, 0);
    
    __csrwr_d(0, LA_CSR_TVAL);
    __csrwr_d(0, LA_CSR_CNTC);

    tcfg = cc_frequency / TICKS_PER_SECOND;
    tcfg <<= CSR_TCFG_VAL_SHIFT;
    tcfg |= CSR_TCFG_PERIOD | CSR_TCFG_EN;

    __csrwr_d(tcfg, LA_CSR_TCFG);

    printk("\nClock: %i per second\n", TICKS_PER_SECOND);

    cc_1us_count = cc_frequency / 1000000;
}

/*
 * TODO 限制 us
 */
void delay_us(int us)
{
    volatile uint64_t startVal, endVal, curVal;

    asm volatile( "rdtime.d %0, $r0 ; " : "=r"(startVal) );
    
    endVal = startVal + cc_1us_count * us;
    
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

//-------------------------------------------------------------------------------------------------

void delay_ms(int ms)
{
    volatile uint64_t startTicks, endTicks, curTicks;

    if (ms <= 0)
    {
        return;
    }

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
