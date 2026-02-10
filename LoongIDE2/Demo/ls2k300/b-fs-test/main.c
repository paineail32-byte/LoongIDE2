/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/* main.c
 *
 * created: 2024-07-16
 *  author:
 */

#include <stdio.h>
#include <stdlib.h>

#include "bsp.h"
#include "osal.h"

#include "fs_test.h"

//-----------------------------------------------------------------------------
// Simple demo of task
//-----------------------------------------------------------------------------

#define USE_DEMO1   1
#define USE_DEMO2   0

#if USE_DEMO1

static osal_task_t m_demo1_task = NULL;

static void demo1_task(void *arg)
{
	unsigned int tickcount;

    for ( ; ; )
    {
#if 1
        do_fs_rw_test();

#else
        tickcount = get_clock_ticks();
        printf("demo1 tick count = %i\r\n", tickcount);

#endif

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
    printf("Hello world!\r\n");
    printf("Welcome to Loongson 2K300!\r\n");

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

    //-----------------------------------------------------
    // Bare-Metal Main Loop
    //-----------------------------------------------------

    for (;;)
    {
        pesudoos_run(0);

        /*
         * If you do more work here, don't use any pesudo-os functions.
         */

    }

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */
