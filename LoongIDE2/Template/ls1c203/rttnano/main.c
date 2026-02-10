/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

/*
 * Loongson 1c103 with RTThread NANO demo.
 */

#include <stdio.h>
#include <stdbool.h>

#include "rtthread.h"
#include "rthw.h"

#include "bsp.h"
#include "console.h"
#include "ls1c203.h"

#define DEMO1       1
#define DEMO2       1

extern HW_PMU_t *g_pmu;

//-------------------------------------------------------------------------------------------------
// Simple demo of task
//-------------------------------------------------------------------------------------------------

#if DEMO1

static rt_uint8_t demo1_stack[512];

struct rt_thread  demo1_thread;

static void demo1_thread_entry(void *arg)
{
    unsigned int tickcount;

    for ( ; ; )
    {
        tickcount = rt_tick_get();

        rt_kprintf("demo1 tick count = %i\r\n", tickcount);

        rt_thread_delay(1000);
    }
}

#endif

//-------------------------------------------------------------------------------------------------

#if DEMO2

static rt_uint8_t demo2_stack[512];

struct rt_thread  demo2_thread;

static void demo2_thread_entry(void *arg)
{
	unsigned int tickcount;

    for ( ; ; )
    {
        tickcount = rt_tick_get();

        rt_kprintf("DEMO2 tick count = %i\r\n", tickcount);

        rt_thread_delay(500);
    }
}

#endif

//-------------------------------------------------------------------------------------------------

int main(void)
{
    unsigned int ticks;
    rt_thread_t tid1, tid2;
    
    rt_kprintf("Hello world!\r\n");
    rt_kprintf("Welcome to Loongson 2K500!\r\n\r\n");

#if DEMO1
	/*
     * 1st Task1 initializing...
     */
    tid1 = &demo1_thread;
    if (rt_thread_init(tid1, "demo1_thread", demo1_thread_entry, RT_NULL,
                       demo1_stack, sizeof(demo1_stack), 5, 10) == RT_EOK)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        rt_kprintf("create demo1 thread fail!\r\n");
    }
#endif

#if DEMO2
	/*
     * 2nd Task2 initializing...
     */
    tid2 = &demo2_thread;
    if (rt_thread_init(tid2, "demo2_thread", demo2_thread_entry, RT_NULL,
                       demo2_stack, sizeof(demo2_stack), 4, 10) == RT_EOK)
    {
        rt_thread_startup(tid2);
    }
    else
    {
        rt_kprintf("create demo2 thread fail!\r\n");
    }
#endif

    /**************************************************************************
     * XXX 把 main() 当作一个工作线程, 不退出
     */

    for ( ; ; )
    {
        ticks = rt_tick_get();

        rt_kprintf("main() tick count = %i\r\n", ticks);

        g_pmu->WdtFeed = WDTFEED_FOOD;      // 不喂狗的话会复位, 4秒?

        rt_thread_delay(2000);
    }

    /*
     * NEVER goto HERE!
     */
    rt_kprintf("main() is exit!\r\n");

	/*
	 * Finsh as another thread...
	 */
    return 0;

}

/*
 * @@ End
 */
