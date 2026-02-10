/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * main.c
 *
 * created: 2024-7-16
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

        printf("demo1 tick count = %i\r\n", tickcount);

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

        printf("DEMO2 tick count = %i\r\n", tickcount);

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
    OS_ERR ucErr;

    printk("Hello world!\r\n");
    printk("Welcome to Loongson 2K300!\r\n\r\n");

    #if USE_DEMO1
    {
        m_demo1_task = osal_task_create("demotask1",
                                         4096,
                                         0,
                                         0,
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
                                         4096,
                                         0,
                                         0,
                                         demo2_task,
                                         NULL );
        if (m_demo2_task)
        {
            printk("create demotask2 successful\r\n");
        }
    }
    #endif

    /*
     * Start uCOS
     */
    OSStart(&ucErr);                /* Start multitasking */

    /*
     * NEVER GO HERE
     */
    for ( ; ; ) ;

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

