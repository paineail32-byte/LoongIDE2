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
#include "ls2k_gpio.h"
#include "ls2k_uart.h"
#include "ls2k_rtc.h"
#include <string.h>
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
 * 主程序
 */
int main(void)
{
    printk("Hello world!\r\n");
    printk("Welcome to Loongson 2K300!\r\n\r\n");
    
    gpio_mux(68, 3);
    gpio_mux(69, 3);
    gpio_enable(68, 0);
    gpio_enable(69, 0);
    gpio_write(68, 1);
    gpio_write(69, 0);
    
    gpio_mux(44, 3);
    gpio_mux(45, 3);
    gpio_enable(44, 0);
    gpio_enable(45, ~0);

    char ch;
    ls2k_uart_init(devUART2, NULL);
    ls2k_uart_open(devUART2, NULL);
    int mode = UART_WORK_POLL;
    ls2k_uart_ioctl(devUART2, IOCTL_UART_SET_RXTX_MODE, (void *)mode);
    char *msg = "UART2 loopback test\r\n";
    ls2k_uart_write(devUART2, msg, strlen(msg), NULL);
    while (1) {
        if (ls2k_uart_read(devUART2, &ch, 1, (void *)1) == 1) {
            /* 收到一个字节，立刻再发回去 */
            ls2k_uart_write(devUART2, &ch, 1, NULL);
        }
    }
    #if USE_DEMO1
    {
        m_demo1_task = osal_task_create("demotask1",
                                         4096,
                                         21,    /* priority */
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
                                         20,    /* priority */
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
     * Start the tasks running.
     */
	vTaskStartScheduler();

    /*
     * Never goto here!
     */
    printk("Exit!\r\n");

	/* If all is well we will never reach here as the scheduler will now be
	 * running.  If we do reach here then it is likely that there was insufficient
	 * heap available for the idle task to be created.
     */

	for ( ; ; );

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

