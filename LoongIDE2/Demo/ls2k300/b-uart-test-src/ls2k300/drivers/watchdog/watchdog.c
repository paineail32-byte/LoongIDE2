/*
 * watchdog.c
 *
 * created: 2024-10-04
 *  author: 
 */

/**
 * 警告: 看门狗不能进行断点调试.
 */

#include <larchintrin.h>

#include "cpu.h"
#include "ls2k300.h"

extern unsigned int apb_frequency;

//-----------------------------------------------------------------------------
// Watch Dog 基地址
//-----------------------------------------------------------------------------

#define WDT_BASE        0x16124000

//-----------------------------------------------------------------------------
// Watch Dog 函数
//-----------------------------------------------------------------------------

int watchdog_start(int interval_ms)
{
    if (interval_ms > 0)
    {
        unsigned int ticks = apb_frequency / 1000 * interval_ms;

        WRITE_REG32(WDT_BASE + 0x08, ticks);    // timer

        WRITE_REG32(WDT_BASE, 0x02);            // en
    }

    return -1;
}

void watchdog_stop(void)
{
    WRITE_REG32(WDT_BASE, 0);
}

void watchdog_feed(void)
{
    WRITE_REG32(WDT_BASE + 0x04, 1);
}

//-----------------------------------------------------------------------------

void ls2k300_reset_immediately(void)
{
    /*
     * WDT_BASE bit[0] 写 1 立即系统软复位
     */

    loongarch_interrupt_disable();
    
    delay_us(10000);
    
    WRITE_REG32(WDT_BASE, 0x01);

    while (1) ;

}

//-----------------------------------------------------------------------------
/*
 * @@END
 */

