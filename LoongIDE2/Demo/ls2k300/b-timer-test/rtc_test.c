/*
 * rtc_test.c
 *
 * created: 2022-03-08
 *  author: 
 */

#include "bsp.h"

#if BSP_USE_RTC

#include <stdio.h>
#include <time.h>

#include "ls2k300.h"

#include "ls2k_rtc.h"

static void rtc_match0_isr(int vector, void *arg)
{
    int device, index;

    if (arg == NULL)
        return;

    device = (long)arg & 0xFF00;
    index  = (long)arg & 0x00FF;

    printk("isr from device=%i, index=%i\r\n", device, index - 1);

}

static void rtc_match0_callback(int device, unsigned match, int *stop)
{
    struct tm dt;

    switch (device & 0xFF00)
    {
        case LS2K_TOY:
            ls2k_toymatch_to_tm(&dt, match);
            normalize_tm(&dt, false);
            printk("isr = %i.%i.%i-%i:%i:%i <-\r\n",
                    dt.tm_year, dt.tm_mon, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
            break;

        case LS2K_RTC:
            printk("->rtc match%i\r\n", (device & 0xFF) - 1);
            break;
    }
}

void rtc_test(void)
{
    unsigned char *s;
    struct tm dt;
    rtc_cfg_t cfg;

    // 2025.2.28-23:59:40
    dt.tm_year = 2025;      // - 1900;  /* 年份, 其值等于实际年份减去1900 */
    dt.tm_mon  = 2;         // - 1;     /* 月份 (从一月开始，0代表一月)- 取值区间为[0, 11] */
    dt.tm_mday = 28;
    dt.tm_hour = 23;
    dt.tm_min  = 59;
    dt.tm_sec  = 55;

    ls2k_rtc_set_datetime(&dt);

    /*
     * 查看是否在变化
     */
    ls2k_rtc_get_datetime(&dt);
    delay_ms(1000);
    s = asctime(&dt);
    printk("%s\n", s);
    
    ls2k_rtc_get_datetime(&dt);
    delay_ms(2000);
    s = asctime(&dt);
    printk("%s\n", s);

    /*
     * 中断回调
     */
    cfg.interval_ms   = 1000;   // 重复触发中断模式.
    cfg.trig_datetime = NULL;
    cfg.cb            = rtc_match0_callback;
    cfg.isr           = NULL;

#if 0
    ls2k_rtc_timer_start(DEVICE_RTCMATCH0, &cfg);
#else
    ls2k_rtc_timer_start(DEVICE_TOYMATCH0, &cfg);

#endif

}

#endif // #if BSP_USE_RTC


