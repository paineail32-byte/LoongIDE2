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
#include "ls2k500.h"

extern unsigned int apb_frequency;

//-----------------------------------------------------------------------------
// Watch Dog 基地址
//-----------------------------------------------------------------------------

#define WDT_BASE        0x1FF6C000

//-----------------------------------------------------------------------------
// Watch Dog 函数
//-----------------------------------------------------------------------------

int watchdog_start(int interval_ms)
{
    if (interval_ms > 0)
    {
        unsigned int ticks = apb_frequency / 1000 * interval_ms;

        WRITE_REG32(WDT_BASE + 0x30, 0x02);     // 0x800000001ff6c030 bit[1], 使能看门狗

        WRITE_REG32(WDT_BASE + 0x38, ticks);    // 0x800000001ff6c038 看门狗计数值

        WRITE_REG32(WDT_BASE + 0x34, 0x1);      // 0x800000001ff6c034 bit[0], 启动看门狗
    }

    return -1;
}

void watchdog_stop(void)
{
    WRITE_REG32(WDT_BASE + 0x30, 0);
}

void watchdog_feed(void)
{
    WRITE_REG32(WDT_BASE + 0x34, 0x1);
}

//-----------------------------------------------------------------------------
/*
 * @@END
 */

