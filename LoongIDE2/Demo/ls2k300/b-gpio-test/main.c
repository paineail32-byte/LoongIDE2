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

#include "ls2k_gpio.h"

#define GPIO_TEST       1
#if GPIO_TEST
#define GPIO_INT_TEST   0
#endif

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

        osal_task_sleep(500);
    }
}

#endif

//-----------------------------------------------------------------------------

#if USE_DEMO2

static osal_task_t m_demo2_task = NULL;

static void demo2_task(void *arg)
{
#if GPIO_TEST
    int out_gpio=14;
    int in12, in13;

    /*
     * GPIO12、GPIO13: 按键输入
     */
    gpio_enable(12, DIR_IN);
    gpio_enable(13, DIR_IN);

    /*
     * GPIO14、GPIO20、GPIO21、GPIO22: LED输出
     */
    gpio_enable(14, DIR_OUT);
    gpio_enable(20, DIR_OUT);
    gpio_enable(21, DIR_OUT);
    gpio_enable(22, DIR_OUT);
    gpio_write(14, 0);
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
                gpio_write(14, 0);
                out_gpio = 21;
                break;

            case 21:
                gpio_write(20, 0);
                out_gpio = 22;
                break;

            case 22:
                gpio_write(21, 0);
                out_gpio = 14;
                break;

            default:
                gpio_write(22, 0);
                out_gpio = 20;
                break;
        }

        /*
         * 检查是否有输入
         */

        in12 = gpio_read(12);
        in13 = gpio_read(13);

        if (in12 || in13)
        {
            printf("gpio12=%i, gpio13=%i\r\n", in12, in13);
        }

#else

        tickcount = get_clock_ticks();

        printf("DEMO2 tick count = %i\r\n", tickcount);
#endif

        osal_task_sleep(500);   // 250
    }
}

#endif // #if USE_DEMO2

//-----------------------------------------------------------------------------

#if GPIO_INT_TEST
int gpio_trig_task_start(void);
#endif

//-----------------------------------------------------------------------------

void ls2k301_gpio_test_start(void);

/*
 * 主程序
 */
int main(void)
{
    printf("Hello world!\r\n");
    printf("Welcome to Loongson 2K300!\r\n");

    #if 0
    {
      #if 1
      
        double tmp = -1.23;
      
        struct ldieee1
        {
            unsigned manl3:16;
            unsigned manl2:32;
            unsigned manl:32;
            unsigned manh:32;
            unsigned exp:15;
            unsigned sign:1;
        } __attribute__((packed));

        union
	    {
	        volatile struct ldieee1 ieee;
	        volatile long double /*_LONG_DOUBLE*/ val;
	    } ld;
	
	
        // printf("sizeof(long double)=%i\r\n", sizeof(long double));
        
        ld.val = tmp;
        
        printf("id.ieee.sign=%i\r\n", ld.ieee.sign);
        printf("id.ieee.sign=%i\r\n", ld.ieee.sign);
      #endif
      
      #if 1
        double a = -1.23;
        double b = 5.55;
        double c = -3.21;
        double d = 6.66;
        
        printf("a=%f, b=%f, c=%f, d=%f\r\n", a, b, c, d);
        printf("a=%f, b=%f, c=%f, d=%f\r\n", a, b, c, d);
        printf("a=%f, b=%f, c=%f, d=%f\r\n", a, b, c, d);
      #endif
    }
    #endif

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

    #if GPIO_INT_TEST
    /*
     * GPIO 中断测试
     */
    gpio_trig_task_start();
    
    #endif

    /*
     * LS2K301 主板 V1.0
     */
    #if !GPIO_TEST
    ls2k301_gpio_test_start();
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
