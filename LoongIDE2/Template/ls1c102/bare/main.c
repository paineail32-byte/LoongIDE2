/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

/*
 * Loongson 1c102 Bare Program, Sample main file
 */

#include <stdio.h>
#include <stdbool.h>

#include "bsp.h"

#include "console.h"

#include "ls1c102.h"

extern HW_PMU_t *g_pmu;

int main(void)
{
    printk("\nHello world!\n");
    printk("Welcome to loongarch-ls1c102!\n\n");

    for (;;)
    {
        register unsigned int ticks;
        ticks = get_clock_ticks();
        
        printk("ticks = %u\r\n", ticks);

        g_pmu->WdtFeed = WDTFEED_FOOD;      // 不喂狗的话会复位, 5秒

        delay_ms(500);
    }

    /*
     * Never goto here!
     */
    return 0;
}

/*
 * @@ End
 */
