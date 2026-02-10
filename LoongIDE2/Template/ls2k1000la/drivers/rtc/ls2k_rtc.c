/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_rtc.c
 *
 * created: 2022-12-13
 *  author: Bian
 */

/******************************************************************************
 * 使用 32.768kHZ 计数器时钟
 */

#include "bsp.h"

#if BSP_USE_RTC

#include <stdio.h>
#include <string.h>

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_rtc_hw.h"
#include "ls2k_rtc.h"

//-------------------------------------------------------------------------------------------------
// definition
//-------------------------------------------------------------------------------------------------

struct rtc_dev
{
    unsigned int        interval_ms;            /* timer irq trigger gap of ms */
    unsigned int        as_clocks;              /* timer interval as 32768 clocks */
    
    int                 irq_num;                /* 中断号 */
    irq_handler_t       isr;                    /* User defined match-isr */
    rtctimer_callback_t callback;               /* callback when match-irq ocurred */
	osal_event_t        p_event;                /* Send the RTOS event when irq ocurred */

    int                 busy;
};

typedef struct
{
	HW_RTC_t *hwRTC;

    struct rtc_dev rtcs[3];                     /* rtc devices array */
    struct rtc_dev toys[3];                     /* toy devices array */

    int rtc_active_count;
    int toy_active_count;

    int initialized;
} RTC_t;

/*
 * RTC device
 */
static RTC_t ls2k_RTC =
{
    .hwRTC = NULL,
    .initialized = 0,
};

static RTC_t *pRTC = &ls2k_RTC;

const void *devRTC = (const void *)&ls2k_RTC;

//-------------------------------------------------------------------------------------------------
// macros
//-------------------------------------------------------------------------------------------------

#define GET_DEVICE(v)   ((unsigned int)((unsigned long)v & 0xFF00ULL))
#define GET_INDEX(v)    ((unsigned int)((unsigned long)v & 0x00FFULL))

//-------------------------------------------------------------------------------------------------
// TOY match format convert
//-------------------------------------------------------------------------------------------------
/*
 * 转换 struct tm 是否使用实际日期
 */
void normalize_tm(struct tm *tm, bool tm_format)
{
    if (tm_format)
    {
        if (tm->tm_year >= 1900)
        {
            tm->tm_year -= 1900;
            tm->tm_mon -= 1;
        }
    }
    else
    {
        if (tm->tm_year < 1900)
        {
            tm->tm_year += 1900;
            tm->tm_mon += 1;
        }
    }
}

/******************************************************************************
 *  TOY_date 格式:
 *   9:4   TOY_SEC      W  秒,   范围 0~59
 *   15:10 TOY_MIN      W  分,   范围 0~59
 *   20:16 TOY_HOUR     W  小时, 范围 0~23
 *   25:21 TOY_DAY      W  日,   范围 1~31
 *   31:26 TOY_MONTH    W  月,   范围 1~12
 */
static void tm_to_toy_datetime(struct tm *dt, unsigned int *hi, unsigned int *lo)
{
    if (dt->tm_year < 1900)
    {
        *lo = ((dt->tm_sec      & 0x3F) <<  4) |
              ((dt->tm_min      & 0x3F) << 10) |
              ((dt->tm_hour     & 0x1F) << 16) |
              ((dt->tm_mday     & 0x1F) << 21) |
             (((dt->tm_mon + 1) & 0x3F) << 26);
        *hi = dt->tm_year + 1900;
    }
    else
    {
        *lo = ((dt->tm_sec  & 0x3F) <<  4) |
              ((dt->tm_min  & 0x3F) << 10) |
              ((dt->tm_hour & 0x1F) << 16) |
              ((dt->tm_mday & 0x1F) << 21) |
              ((dt->tm_mon  & 0x3F) << 26);
        *hi = dt->tm_year;
    }
}

static void toy_datetime_to_tm(struct tm *dt, unsigned int hi, unsigned int lo)
{
    dt->tm_sec  = (lo >>  4) & 0x3F;
    dt->tm_min  = (lo >> 10) & 0x3F;
    dt->tm_hour = (lo >> 16) & 0x1F;
    dt->tm_mday = (lo >> 21) & 0x1F;
    dt->tm_mon  = (lo >> 26) & 0x3F;
    dt->tm_year =  hi;

    dt->tm_year -= 1900;
    dt->tm_mon  -= 1;
}

/******************************************************************************
 *  TOYMATCH 格式:
 *   31:26  YEAR   RW  年,   范围 0~63
 *   25:22  MONTH  RW  月,   范围 1~12
 *   21:17  DAY    RW  日,   范围 1~31
 *   16:12  HOUR   RW  小时, 范围 0~23
 *   11:6   MIN    RW  分,   范围 0~59
 *   5:0    SEC    RW  秒,   范围 0~59
 */
void ls2k_tm_to_toymatch(struct tm *dt, unsigned int *match)
{
    if (dt->tm_year < 1900)
    {
        *match = ((dt->tm_sec          & 0x3F) <<  0) |
                 ((dt->tm_min          & 0x3F) <<  6) |
                 ((dt->tm_hour         & 0x1F) << 12) |
                 ((dt->tm_mday         & 0x1F) << 17) |
                (((dt->tm_mon + 1)     & 0x0F) << 22) |
                (((dt->tm_year + 1900) & 0x3F) << 26);
    }
    else
    {
        *match = ((dt->tm_sec  & 0x3F) <<  0) |
                 ((dt->tm_min  & 0x3F) <<  6) |
                 ((dt->tm_hour & 0x1F) << 12) |
                 ((dt->tm_mday & 0x1F) << 17) |
                 ((dt->tm_mon  & 0x0F) << 22) |
                 ((dt->tm_year & 0x3F) << 26);
    }
}

void ls2k_toymatch_to_tm(struct tm *dt, unsigned int match)
{
    dt->tm_sec  = (match >>  0) & 0x3F;
    dt->tm_min  = (match >>  6) & 0x3F;
    dt->tm_hour = (match >> 12) & 0x1F;
    dt->tm_mday = (match >> 17) & 0x1F;
    dt->tm_mon  = (match >> 22) & 0x0F;
    dt->tm_year = (match >> 26) & 0x3F;

    dt->tm_year += 84;
    dt->tm_mon  -= 1;
}

//-------------------------------------------------------------------------------------------------
// Forward declaration
//-------------------------------------------------------------------------------------------------

static int ls2k_get_system_datetime(struct tm *dt);

static int ls2k_rtc_install_isr(int device, int index);

//-------------------------------------------------------------------------------------------------
// RTC operator
//-------------------------------------------------------------------------------------------------

/*
 * 毫秒数转换为 rtc_clocks
 */
static int convert_microseconds_to_rtcclocks(unsigned int ms)
{
    RTC_t *pRTC = &ls2k_RTC;
    unsigned int trim;
    int clocks;

    trim = pRTC->hwRTC->rtctrim;
    if (0 == trim)
        trim = 1;

    clocks = ms * 32768 / 1000 / trim;

    return clocks;
}

static int ls2k_rtc_set_trim(unsigned int trim)
{
    return 0;
}

/*
 * 启动 rtcmatch
 */
static int ls2k_rtc_start(int index, rtc_cfg_t *cfg)
{
    int clocks;
    unsigned int next_match;

    if (cfg == NULL)
        return -1;

    if (pRTC->rtcs[index].busy)
        return 1;

    clocks = convert_microseconds_to_rtcclocks(cfg->interval_ms);
    if (clocks <= 0)
        return -1;

#if RTC_TEN_REN_LATER
    pRTC->hwRTC->rtcctrl |= RTC_CTRL_REN;
#endif

    pRTC->rtcs[index].interval_ms = cfg->interval_ms;
    pRTC->rtcs[index].as_clocks   = clocks;
    pRTC->rtcs[index].isr         = cfg->isr;
    pRTC->rtcs[index].callback    = cfg->cb;
    pRTC->rtcs[index].p_event     = (osal_event_t)cfg->event;

    ls2k_rtc_install_isr(LS2K_RTC, index);              /* Install interrupt handler */

    next_match = pRTC->hwRTC->rtcread + clocks;
    pRTC->hwRTC->rtcmatch[index] = next_match;          /* Set rtc match value */

    pRTC->rtcs[index].busy = 1;
    pRTC->rtc_active_count++;

    return 0;
}

/*
 * 停止 rtcmatch
 */
static int ls2k_rtc_stop(int index)
{
    /* Uninstall interrupt handler
     */
    ls2k_remove_irq_handler(pRTC->rtcs[index].irq_num);

    pRTC->rtcs[index].interval_ms = 0;
    pRTC->rtcs[index].as_clocks   = 0;
    pRTC->rtcs[index].isr         = NULL;
    pRTC->rtcs[index].callback    = NULL;
    pRTC->rtcs[index].p_event     = NULL;

    pRTC->hwRTC->rtcmatch[index] = 0;               /* Set rtc match zero */

    pRTC->rtcs[index].busy = 0;
    pRTC->rtc_active_count--;

#if RTC_TEN_REN_LATER
    if (pRTC->rtc_active_count == 0)
    {
        pRTC->hwRTC->rtcctrl &= ~RTC_CTRL_REN;
    }
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// TOY operator
//-------------------------------------------------------------------------------------------------

/*
 * LS2K1000LA 从APB配置头读 RTC IO base
 */
static void ls2k1000la_set_rtc_base(void)
{
    unsigned long apb_address0;
    apb_address0 = READ_REG64(APB_CFG_HEAD_BASE + 0x10) & ~0x0Ful;
    pRTC->hwRTC = (HW_RTC_t *)PHYS_TO_UNCACHED(apb_address0 + 0x7800);
}

/*
 * 秒数转换为 toymatch time
 */
unsigned int ls2k_seconds_to_toymatch(unsigned int seconds)
{
    unsigned int match;
    struct tm dt;
    time_t secs = seconds;

    localtime_r((const time_t *)&secs, &dt);	/* seconds 转换为 struct tm */
    ls2k_tm_to_toymatch(&dt, &match);       	/* struct tm 转换为 toymatch 日期 */

    return match;
}

/*
 * toymatch time 转换为秒数
 */
unsigned int ls2k_toymatch_to_seconds(unsigned int match)
{
    time_t secs;
    struct tm dt;

    ls2k_toymatch_to_tm(&dt, match);        	/* toymatch 日期转换为 struct tm */
    secs = mktime(&dt);                  	    /* struct tm 转换为秒数 */

    return (unsigned int)secs;
}

static unsigned int ls2k_get_future_toymatch(unsigned int after_seconds)
{
    time_t next;
    struct tm dt;

    ls2k_get_system_datetime(&dt);          	/* 获取当前日期 struct tm */
    next = mktime(&dt);                     	/* struct tm 转换为秒数 */
    next += after_seconds;                  	/* 加上延迟秒数 */

    localtime_r((const time_t *)&next, &dt);	/* seconds 转换为 struct tm */

    ls2k_tm_to_toymatch(&dt, (unsigned int *)&next);    /* struct tm 转换为 toymatch 日期 */

    return (unsigned int)next;
}

/*
 * 设置系统日期
 */
static int ls2k_set_system_datetime(struct tm *dt)
{
    unsigned int hi, lo;

    if (dt == NULL)
        return -1;

    if (!pRTC->hwRTC)
    {
        ls2k1000la_set_rtc_base();
    }

    tm_to_toy_datetime(dt, &hi, &lo);
    pRTC->hwRTC->toywritelo = lo;
    pRTC->hwRTC->toywritehi = hi;

    return 0;
}

/*
 * 获取系统日期
 */
static int ls2k_get_system_datetime(struct tm *dt)
{
    unsigned int hi, lo;

    if (dt == NULL)
        return -1;

    if (!pRTC->hwRTC)
    {
        ls2k1000la_set_rtc_base();
    }

    lo = pRTC->hwRTC->toyreadlo;
    hi = pRTC->hwRTC->toyreadhi;
    toy_datetime_to_tm(dt, hi, lo);

    return 0;
}

static int ls2k_toy_set_trim(unsigned int trim)
{
    return 0;
}

/*
 * 启动 toymatch
 */
static int ls2k_toy_start(int index, rtc_cfg_t *cfg)
{
    unsigned int next_match, interval_sec;

    if (cfg == NULL)
        return -1;

    if (pRTC->toys[index].busy)
        return 1;

    interval_sec = cfg->interval_ms / 1000;         /* 转换为秒数 */

    if ((interval_sec < 1) && (!cfg->trig_datetime))
    {
        printk("invalid argument when start toy.");
        return -1;
    }

    if (cfg->trig_datetime)
    {
        ls2k_tm_to_toymatch(cfg->trig_datetime, &next_match);
        cfg->interval_ms = 0;       /* only once flag*/
    }
    else
    {
        next_match = ls2k_get_future_toymatch(interval_sec);
    }

    if (next_match == 0)
        return -1;

#if RTC_TEN_REN_LATER
    pRTC->hwRTC->rtcctrl |= RTC_CTRL_TEN;
#endif

    pRTC->toys[index].interval_ms = cfg->interval_ms;
    pRTC->toys[index].as_clocks   = 0;
    pRTC->toys[index].isr         = cfg->isr;
    pRTC->toys[index].callback    = cfg->cb;
    pRTC->toys[index].p_event     = (osal_event_t)cfg->event;

    ls2k_rtc_install_isr(LS2K_TOY, index);          /* Install interrupt handler */

    pRTC->hwRTC->toymatch[index] = next_match;      /* Set toy match value */

    pRTC->toys[index].busy = 1;
    pRTC->toy_active_count++;

    return 0;
}

/*
 * 停止 toymatch
 */
static int ls2k_toy_stop(int index)
{
    /* Uninstall interrupt handler
     */
    ls2k_remove_irq_handler(pRTC->toys[index].irq_num);

    pRTC->toys[index].interval_ms = 0;
    pRTC->toys[index].as_clocks   = 0;
    pRTC->toys[index].isr         = NULL;
    pRTC->toys[index].callback    = NULL;
    pRTC->toys[index].p_event     = NULL;

    pRTC->hwRTC->toymatch[index] = 0;               /* Set toy match zero */

    pRTC->toys[index].busy = 0;
    pRTC->toy_active_count--;

#if RTC_TEN_REN_LATER
    if (pRTC->toy_active_count == 0)
    {
        pRTC->hwRTC->rtcctrl &= ~RTC_CTRL_TEN;
    }
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Interrupt
//-------------------------------------------------------------------------------------------------

/*
 * 中断处理程序
 */
static void ls2k_rtc_share_isr(int vector, void *arg)
{
    int device, index, stop = 0;
    unsigned int cur_match;
    osal_event_t event = NULL;

    if (arg == NULL)
        return;

    device = (long)arg & 0xFF00ULL;
    index  = (long)arg & 0x00FFULL;

    if (!(index & 0x03))
        return;

    index -= 1;

    switch (device)
    {
        case LS2K_RTC:
            cur_match = pRTC->hwRTC->rtcmatch[index];

            if (pRTC->rtcs[index].interval_ms > 0)      /* Set next rtc match */
            {
                pRTC->hwRTC->rtcmatch[index] = cur_match + pRTC->rtcs[index].as_clocks;
            }

            if (pRTC->rtcs[index].callback)             /* 回调函数 */
            {
                (pRTC->rtcs[index].callback)((long)arg, cur_match, &stop);

                if ((stop) || (!pRTC->rtcs[index].interval_ms)) /* Stop it */
                {
                    ls2k_rtc_stop(index);
                }

                return;                                 /* ingore event */
            }

            event = pRTC->rtcs[index].p_event;
            break;

        case LS2K_TOY:
            cur_match = pRTC->hwRTC->toymatch[index];

            if (pRTC->toys[index].interval_ms >= 1000)  /* Set next toy match */
            {
                unsigned int seconds;
                seconds  = ls2k_toymatch_to_seconds(cur_match);
                seconds += pRTC->toys[index].interval_ms / 1000;
                seconds  = ls2k_seconds_to_toymatch(seconds);
                pRTC->hwRTC->toymatch[index] = seconds;
            }

            if (pRTC->toys[index].callback)             /* 回调函数 */
            {
                (pRTC->toys[index].callback)((long)arg, cur_match, &stop);

                if ((stop) || (!pRTC->toys[index].interval_ms)) /* Stop it */
                {
                    ls2k_toy_stop(index);
                }

                return;                                 /* ingore event */
            }

            event = pRTC->toys[index].p_event;
            break;
    }

    /******************************************************
     * Timer Event
     */
    if (event)
    {
        osal_event_send(event, RTC_TIMER_EVENT);
    }
}

/*
 * parameters:
 *
 *  device: DEVICE_RTC or DEVICE_TOY
 *   index: 0~2
 */
static int ls2k_rtc_install_isr(int device, int index)
{
    unsigned long arg;
    irq_handler_t isr = NULL;
    unsigned int irqnum;
    arg = (unsigned int)device | (index + 1);

    switch (device)
    {
        case LS2K_RTC:
            irqnum = pRTC->rtcs[index].irq_num;
            isr    = pRTC->rtcs[index].isr;
            break;

        case LS2K_TOY:
            irqnum = pRTC->toys[index].irq_num;
            isr    = pRTC->toys[index].isr;
            break;

        default:
            return -1;
    }

    ls2k_interrupt_disable(irqnum);

    printk("install rtc isr: %i\n", (int)irqnum);

    if (isr == NULL)
    {
        ls2k_install_irq_handler(irqnum, ls2k_rtc_share_isr, (void *)arg);
    }
    else
    {
        ls2k_install_irq_handler(irqnum, isr, (void *)arg);
    }

    ls2k_set_irq_routeip(irqnum, INT_ROUTE_IP2);
    // ls2k_set_irq_triggermode(irqnum, INT_TRIGGER_LEVEL);

    ls2k_interrupt_enable(irqnum);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// driver implement
//-------------------------------------------------------------------------------------------------

/*
 * parameters:
 *  dev:  NULL
 *  arg:  struct tm *, if not NULL then initialize datetime => toywrite
 */
STATIC_DRV int RTC_initialize(const void *dev, void *arg)
{
    int i;
    unsigned int rtc_ctrl;
    
    if (pRTC->initialized)
    {
        return 0;
    }

    memset((void *)pRTC, 0, sizeof(RTC_t));

    for (i=0; i<3; i++)
    {
        pRTC->rtcs[i].irq_num = (i == 0) ? INTC1_RTC0_IRQ :
                                (i == 1) ? INTC1_RTC1_IRQ : INTC1_RTC2_IRQ;
        pRTC->toys[i].irq_num = (i == 0) ? INTC1_TOY0_IRQ :
                                (i == 1) ? INTC1_TOY1_IRQ : INTC1_TOY2_IRQ;
    }
    
    ls2k1000la_set_rtc_base();

    /******************************************************
     * 如果RTC 是正常运行的, 且读出有日期...
     ******************************************************/

    rtc_ctrl = pRTC->hwRTC->rtcctrl;
    if (rtc_ctrl & RTC_CTRL_32S)
    {
        struct tm dt;

        ls2k_set_system_datetime((struct tm *)arg); /* 尝试设置日期 */

        ls2k_get_system_datetime(&dt);
        normalize_tm(&dt, false);

        printk("RTC current time is %i.%i.%i-%i:%i:%i\r\n",
                dt.tm_year, dt.tm_mon, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);

#if !RTC_TEN_REN_LATER
        pRTC->hwRTC->rtcctrl |= RTC_CTRL_REN | RTC_CTRL_TEN;
#endif
        pRTC->initialized = 1;
        return 0;   /* 不执行初始化直接返回 */
    }

    /**
     * 启动 RTC
     */
    pRTC->hwRTC->rtcctrl = 0;               /* 寄存器复位 */
    pRTC->hwRTC->toytrim = 0;
    pRTC->hwRTC->rtctrim = 0;

    for (i=0; i<3; i++)
    {
        pRTC->hwRTC->toymatch[i] = 0;
        pRTC->hwRTC->rtcmatch[i] = 0;
    }

	/* 有初始化日期参数 */
    ls2k_set_system_datetime((struct tm *)arg);

    pRTC->hwRTC->rtcwrite = 0;

    /**************************************************************************
     * 使能 RTC 和 TOY
     */
    pRTC->hwRTC->rtcctrl = RTC_CTRL_EO;     /* 使能 32.768k晶振 */

    i = 0;
    while (!(pRTC->hwRTC->rtcctrl & RTC_CTRL_32S))
    {
        /*
         * 等待1ms: 32.768k晶振正常工作
         */
        if (i++ > 1000)
            return -1;
        delay_us(1);
    }

#if !RTC_TEN_REN_LATER
    pRTC->hwRTC->rtcctrl |= RTC_CTRL_REN | RTC_CTRL_TEN;
#endif

    pRTC->initialized = 1;

    return 0;
}

/*
 * parameters:
 *  dev:  DEVICE_XXX
 *  arg:  rtc_cfg_t *
 */
STATIC_DRV int RTC_open(const void *dev, void *arg)
{
    if (arg && (GET_INDEX(dev) & 0x03))
    {
        switch (GET_DEVICE(dev))
        {
            case LS2K_RTC: return ls2k_rtc_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            case LS2K_TOY: return ls2k_toy_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
        }
    }

    return -1;
}

/*
 * parameters:
 *  dev:  DEVICE_XXX
 *  arg:  NULL
 */
STATIC_DRV int RTC_close(const void *dev, void *arg)
{
    if (GET_INDEX(dev) & 0x03)
    {
        switch (GET_DEVICE(dev))
        {
            case LS2K_RTC: return ls2k_rtc_stop(GET_INDEX(dev)-1);
            case LS2K_TOY: return ls2k_toy_stop(GET_INDEX(dev)-1);
        }
    }

    return -1;
}

/*
 * get current datetime <= toyread
 *
 * parameters:
 *  dev:  NULL
 *  buf:  struct tm *
 *  size: sizeof(struct tm)
 *  arg:  NULL
 */
STATIC_DRV int RTC_read(const void *dev, void *buf, int size, void *arg)
{
    if (buf && (size == sizeof(struct tm)))
    {
        if (ls2k_get_system_datetime((struct tm *)buf) == 0)
            return size;
        else
            return 0;
    }

    return -1;
}

/*
 * set current datetime => toywrite
 *
 * parameters:
 *  dev:  NULL
 *  buf:  struct tm *
 *  size: sizeof(struct tm)
 *  arg:  NULL
 */
STATIC_DRV int RTC_write(const void *dev, void *buf, int size, void *arg)
{
    if (buf && (size == sizeof(struct tm)))
    {
        if (ls2k_set_system_datetime((struct tm *)buf) == 0)
            return size;
        else
            return 0;
    }

    return -1;
}

/*
 * parameters:
 *  dev:  NULL or DEVICE_XXX
 *  arg:  if dev==NULL then do TRIM access
 *        else is as rtc_cfg_t *
 */
STATIC_DRV int RTC_ioctl(const void *dev, int cmd, void *arg)
{
    int rt = 0;

    switch (cmd)
    {
    	case IOCTL_SET_SYS_DATETIME:        // struct tm *
    		rt = ls2k_set_system_datetime((struct tm *)arg);
    		break;

    	case IOCTL_GET_SYS_DATETIME:        // struct tm *
    		rt = ls2k_get_system_datetime((struct tm *)arg);
    		break;

        /*
         * control rtc
         */
        case IOCTL_RTC_SET_TRIM:            // unsigned int *
            rt = ls2k_rtc_set_trim(*((unsigned int *)arg));
            break;

        case IOCTL_RTC_GET_TRIM:            // unsigned int *
            if (arg)
                *((unsigned int *)arg) = pRTC->hwRTC->rtctrim;
            else
                rt = -1;
            break;

        case IOCTL_RTCMATCH_START:          // rtc_cfg_t *
            if ((GET_DEVICE(dev) == LS2K_RTC) && (GET_INDEX(dev) & 0x03))
                rt = ls2k_rtc_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            else
                rt = -1;
            break;

        case IOCTL_RTCMATCH_STOP:
            if ((GET_DEVICE(dev) == LS2K_RTC) && (GET_INDEX(dev) & 0x03))
                rt = ls2k_rtc_stop(GET_INDEX(dev)-1);
            else
                rt = -1;
            break;

        /*
         * control toy
         */
        case IOCTL_TOY_SET_TRIM:            // unsigned int *
            rt = ls2k_toy_set_trim(*((unsigned int *)arg));
            break;

        case IOCTL_TOY_GET_TRIM:            // unsigned int *
            if (arg)
                *((unsigned int *)arg) = pRTC->hwRTC->toytrim;
            else
                rt = -1;
            break;

        case IOCTL_TOYMATCH_START:          // rtc_cfg_t *
            if ((GET_DEVICE(dev) == LS2K_TOY) && (GET_INDEX(dev) & 0x03))
                rt = ls2k_toy_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            else
                rt = -1;
            break;

        case IOCTL_TOYMATCH_STOP:
            if ((GET_DEVICE(dev) == LS2K_TOY) && (GET_INDEX(dev) & 0x03))
                rt = ls2k_toy_stop(GET_INDEX(dev)-1);
            else
                rt = -1;
            break;

        default:
            rt = -1;
            break;
    }

    return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * RTC driver operators
 */
static const driver_ops_t ls2k_rtc_drv_ops =
{
    .init_entry  = RTC_initialize,
    .open_entry  = RTC_open,
    .close_entry = RTC_close,
    .read_entry  = RTC_read,
    .write_entry = RTC_write,
    .ioctl_entry = RTC_ioctl,
};

const driver_ops_t *rtc_drv_ops = &ls2k_rtc_drv_ops;
#endif

/******************************************************************************
 * User API
 */
int ls2k_rtc_set_datetime(struct tm *dt)
{
    return ls2k_set_system_datetime(dt);
}

int ls2k_rtc_get_datetime(struct tm *dt)
{
    return ls2k_get_system_datetime(dt);
}

int ls2k_rtc_timer_start(unsigned device, rtc_cfg_t *cfg)
{
    return RTC_open((void *)(unsigned long)device, (void *)cfg);
}

int ls2k_rtc_timer_stop(unsigned device)
{
    return RTC_close((void *)(unsigned long)device, NULL);
}

/*
 * 获取当前秒数
 */
time_t ls2k_rtc_get_seconds(void)
{
    time_t secs = 0;
    struct tm dt;

    if (0 == ls2k_get_system_datetime(&dt))
    {
        secs = mktime(&dt);             /* struct tm 转换为秒数 */
    }

    return secs;
}

/*
 * 获取当前RTC的 struct timeval
 */
int ls2k_rtc_get_timeval(struct timeval *tv)
{
    unsigned int hi, lo;
    struct tm dt;

    if (tv == NULL)
        return -1;

    lo = pRTC->hwRTC->toyreadlo;
    hi = pRTC->hwRTC->toyreadhi;
    toy_datetime_to_tm(&dt, hi, lo);

    tv->tv_sec  = mktime(&dt);
    tv->tv_usec = (lo & 0x0F) * 100 * 1000;

    return 0;
}

#endif // #if defined(BSP_USE_RTC)

/*
 * @@ END
 */
