/*
 * hpet_test.c
 *
 * created: 2022-10-20
 *  author: 
 */

#include "bsp.h"

#if BSP_USE_HPET

#include <stdlib.h>

#include "ls2k_hpet.h"

static int t0 = 0;
static int t1 = 0;
static int t2 = 0;

extern void console_putch(char ch);

#if BSP_USE_HPET1
static void hpet1_callback(void *hpet, int timer, int *stop)
{
    switch (timer)
    {
        case HPET_TIMER0:
            console_putch('A'); // printk("A");
            if (t0++ > 10)
                *stop = 1;
            break;

        case HPET_TIMER1:
            console_putch('B'); // printk("B");
            if (t1++ > 20)
                *stop = 1;
            break;

        case HPET_TIMER2:
            console_putch('C'); // printk("C");
            if (t2++ > 30)
                *stop = 1;
            break;

        default:
            console_putch('+'); // printk("+");
            break;
    }

}
#endif

#if BSP_USE_HPET2
static void hpet2_callback(void *hpet, int timer, int *stop)
{
    switch (timer)
    {
        case HPET_TIMER0:
            console_putch('a'); // printk("A");
            if (t0++ > 10)
                *stop = 1;
            break;

        case HPET_TIMER1:
            console_putch('b'); // printk("B");
            if (t1++ > 20)
                *stop = 1;
            break;

        case HPET_TIMER2:
            console_putch('b'); // printk("C");
            if (t2++ > 30)
                *stop = 1;
            break;

        default:
            console_putch('-'); // printk("+");
            break;
    }

}
#endif

#if BSP_USE_HPET3
static void hpet3_callback(void *hpet, int timer, int *stop)
{
    switch (timer)
    {
        case HPET_TIMER0:
            console_putch('x'); // printk("A");
            if (t0++ > 10)
                *stop = 1;
            break;

        case HPET_TIMER1:
            console_putch('y'); // printk("B");
            if (t1++ > 20)
                *stop = 1;
            break;

        case HPET_TIMER2:
            console_putch('z'); // printk("C");
            if (t2++ > 30)
                *stop = 1;
            break;

        default:
            console_putch('?'); // printk("+");
            break;
    }

}
#endif

void hpet_test(void)
{
    hpet_cfg_t cfg = { 0 };

    cfg.work_mode   = HPET_MODE_CYCLE; // HPET_MODE_SINGLE; //
    cfg.interval_ns = 1000*1000*1000;
    cfg.cb          = hpet1_callback;

#if BSP_USE_HPET1
    /*
     * 启动定时器 1
     */
    ls2k_hpet_timer_start(devHPET1, HPET_TIMER1, &cfg);
#endif

#if BSP_USE_HPET2
    /*
     * 启动定时器 2
     */
    cfg.interval_ns = 300*1000*1000;
    cfg.cb          = hpet2_callback;
    ls2k_hpet_timer_start(devHPET2, HPET_TIMER2, &cfg);
#endif

#if BSP_USE_HPET3
    /*
     * 启动定时器 3
     */
    cfg.interval_ns = 500*1000*1000;
    cfg.cb          = hpet3_callback;
    ls2k_hpet_timer_start(devHPET3, HPET_TIMER0, &cfg);
#endif


}


#endif // #if BSP_USE_HPET

