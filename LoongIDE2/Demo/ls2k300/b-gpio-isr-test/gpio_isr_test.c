/*
 * gpio_isr_test.c
 *
 * created: 2025-11-03
 *  author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp.h"

#include "osal.h"

#include "ls2k_gpio.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

//-----------------------------------------------------------------------------

#define GPIO_TRIG       64  // GPIO64 SPI2_CLK
#define GPIO_NUM        67  // GPIO67 SPI2_CS0

//-----------------------------------------------------------------------------
// gpio 中断
//-----------------------------------------------------------------------------

void gpioXXX_isr(int gpioNum, void *arg)
{
    int val = gpio_read(gpioNum);

    printk("gpio_isr = %i\n", val);

}

//-----------------------------------------------------------------------------
// 触发任务
//-----------------------------------------------------------------------------

static osal_task_t m_gpio_trig_task = NULL;

static void gpio_trig_task(void *arg)
{
	int trig = 0;

    for ( ; ; )
    {
        if (trig)
            trig = 0;
        else
            trig = 1;

        gpio_write(GPIO_TRIG, trig);

        osal_task_sleep(500);
    }
}

//-----------------------------------------------------------------------------
// 启动程序
//-----------------------------------------------------------------------------

/*
 * 主程序
 */
int gpio_trig_task_start(void)
{
    gpio_enable(GPIO_TRIG, DIR_OUT);

    gpio_enable(GPIO_NUM, DIR_IN);

    ls2k300_gpio_interrupt_disable(GPIO_NUM);

    ls2k300_gpio_isr_install(GPIO_NUM, gpioXXX_isr, NULL);

    /*
     * 更换不同的触发方式
     */
#if 0
    ls2k300_gpio_interrupt_enable(GPIO_NUM, GPIO_INT_TRIG_EDGE_UP);
#elif 1
    ls2k300_gpio_interrupt_enable(GPIO_NUM, GPIO_INT_TRIG_EDGE_DOWN);
#elif 0
    ls2k300_gpio_interrupt_enable(GPIO_NUM, GPIO_INT_TRIG_DUAL);
#elif 0
    ls2k300_gpio_interrupt_enable(GPIO_NUM, GPIO_INT_TRIG_LEVEL_HIGH);
#elif 1
    ls2k300_gpio_interrupt_enable(GPIO_NUM, GPIO_INT_TRIG_LEVEL_LOW);
#endif

#if USE_EXTINT
  #if GPIO_NUM == 67
    ls2k_interrupt_enable(EXTI2_GPIO_64_67_IRQ);
  #elif GPIO_NUM == 47
    ls2k_interrupt_enable(EXTI2_GPIO_44_47_IRQ);
  #else
    #error "error gpio irq number"
  #endif
#else
  #if GPIO_NUM == 67
    ls2k_interrupt_enable(INTC1_GPIO_64_79_IRQ);
  #elif GPIO_NUM == 47
    ls2k_interrupt_enable(INTC1_GPIO_32_47_IRQ);
  #else
    #error "error gpio irq number"
  #endif
#endif

    m_gpio_trig_task = osal_task_create("gpioTrig",
                                        4096,
                                        0,
                                        0,
                                        gpio_trig_task,
                                        NULL );
    if (m_gpio_trig_task)
    {
        printk("create demotask1 successful\r\n");
        return 0;
    }

    printk("create demotask1 fail.\r\n");
    return -1;
}


