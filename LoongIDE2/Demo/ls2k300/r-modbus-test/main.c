/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * main.c
 *
 * created: 2024-7-18
 *  author: Bian
 */

#include <stdio.h>

#include "bsp.h"
#include "osal.h"

//-----------------------------------------------------------------------------
// Simple demo of task
//-----------------------------------------------------------------------------

#define USE_DEMO1   0
#define USE_DEMO2   0

#if USE_DEMO1

static osal_task_t m_demo1_task = NULL;

static void demo1_task(void *arg)
{
	unsigned int tickcount;

    for ( ; ; )
    {
        tickcount = get_clock_ticks();

        printk("demo1 tick count = %i\r\n", tickcount);

        osal_task_sleep(1000);

    }
}

#endif

//-----------------------------------------------------------------------------

#if USE_DEMO2

static osal_task_t m_demo2_task = NULL;

static void demo2_task(void *arg)
{
	unsigned int tickcount;

    for ( ; ; )
    {

        tickcount = get_clock_ticks();

        printk("DEMO2 tick count = %i\r\n", tickcount);

        osal_task_sleep(500);
    }
}

#endif // #if USE_DEMO2

//-----------------------------------------------------------------------------

/*
 * Ö÷³ÌÐò
 */
int main(void)
{
    printk("Hello world!\r\n");
    printk("Welcome to Loongson 2K300!\r\n\r\n");

    /**
     * Modbus ²âÊÔ
     */
    #if USE_MODBUS && (BSP_USE_HPET0 || BSP_USE_RTC)
    {
        extern void modbus_init(uint32_t freq);
        extern void mb_cfg_slave(void);
        extern int start_mb_master_task(void);

        modbus_init(100);                   // 100HZ = 10ms

        #if BSP_USE_UART4
        {
            mb_cfg_slave();
        }
        #endif

        #if BSP_USE_UART6
        {
            start_mb_master_task();
        }
        #endif
    }
    #endif

    #if USE_DEMO1
    {
        m_demo1_task = osal_task_create("demotask1",
                                         8192,
                                         28,    /* priority */
                                         10,    /* slice */
                                         demo1_task,
                                         NULL );
        if (m_demo1_task)
        {
            printk("create demotask1 successful\r\n");
        }
    }
    #endif

    #if USE_DEMO2
    {
        m_demo2_task = osal_task_create("demotask2",
                                         8192,
                                         27,    /* priority */
                                         10,    /* slice */
                                         demo2_task,
                                         NULL );
        if (m_demo2_task)
        {
            printk("create demotask2 successful\r\n");
        }
    }
    #endif

    printk("main() is exit!\r\n");

	/*
	 * Finsh as another thread...
	 */
    return 0;

}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */
