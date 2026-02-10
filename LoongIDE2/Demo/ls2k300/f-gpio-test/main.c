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

#define GPIO_TEST   1

//-----------------------------------------------------------------------------
// Simple demo of task
//-----------------------------------------------------------------------------

#define USE_DEMO1   0
#define USE_DEMO2   1

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

#if GPIO_TEST
    int out_gpio=6, in4, in5;

    /*
     * GPIO4、GPIO5: 按键输入
     */
    gpio_enable(4, DIR_IN);
    gpio_enable(5, DIR_IN);

    /*
     * GPIO6、GPIO20、GPIO21、GPIO22: LED输出
     */
    gpio_enable( 6, DIR_OUT);
    gpio_enable(20, DIR_OUT);
    gpio_enable(21, DIR_OUT);
    gpio_enable(22, DIR_OUT);
    gpio_write( 6, 0);
    gpio_write(20, 0);
    gpio_write(21, 0);
    gpio_write(22, 0);

#else
	unsigned int tickcount;
#endif

    for ( ; ; )
    {
#if GPIO_TEST

        /*
         * 循环输出
         */
        gpio_write(out_gpio, 1);

        switch (out_gpio)
        {
            case 20:
                gpio_write(6, 0);
                out_gpio = 21;
                break;

            case 21:
                gpio_write(20, 0);
                out_gpio = 22;
                break;

            case 22:
                gpio_write(21, 0);
                out_gpio = 6;
                break;

            default:
                gpio_write(22, 0);
                out_gpio = 20;
                break;
        }

        /*
         * 检查是否有输入
         */
        in4 = gpio_read(4);
        in5 = gpio_read(5);

        if (in4 || in5)
        {
            printf("gpio4=%i, gpio5=%i\r\n", in4, in5);
        }

#else

        tickcount = get_clock_ticks();

        printf("DEMO2 tick count = %i\r\n", tickcount);
#endif

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

