/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_hpet.c
 *
 * created: 2024-08-05
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_HPET

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_hpet_hw.h"
#include "ls2k_hpet.h"

#include "osal.h"

//-------------------------------------------------------------------------------------------------
// definition
//-------------------------------------------------------------------------------------------------

struct hpet_priv;

struct hpet_timer
{
    int           ID;                   /* 序号: 0~2 */
    unsigned long interval_ns;          /* timer irq trigger gap of ns */
    unsigned long as_clocks;
    int           work_mode;            /* 周期/边缘, 周期/电平 */
    int           periodic;             /* 周期 */

    int           irq_num;              /* 中断号 */
    irq_handler_t user_isr;             /* 用户自定义中断 */
    hpetimer_callback_t cb;             /* callback when match-irq ocurred */

    osal_event_t  p_event;

    struct hpet_priv *owner;            /* HPET_t * */
    volatile int busy;
};

typedef struct hpet_priv
{
	HW_HPET_t *hwHPET;

    struct hpet_timer timers[3];        /* hpet virtual devices array */

#if !USE_EXTINT
    /*
     * LS2K500 的HPET是一个设备的三个定时器共享一个中断.
     */
    int irq_Num;                        /* 中断号 */
#endif

    int initialized;
} HPET_t;

/*
 * HPET device
 */
#if BSP_USE_HPET0
static HPET_t ls2k_HPET0 =
{
    .hwHPET  = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET0_BASE),
#if USE_EXTINT
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = EXTI1_HPET0_0_IRQ, .owner = &ls2k_HPET0, },
                { .ID = HPET_TIMER1, .irq_num = EXTI1_HPET0_1_IRQ, .owner = &ls2k_HPET0, },
                { .ID = HPET_TIMER2, .irq_num = EXTI1_HPET0_2_IRQ, .owner = &ls2k_HPET0, }},
#else
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = 0, .owner = &ls2k_HPET0, },
                { .ID = HPET_TIMER1, .irq_num = 0, .owner = &ls2k_HPET0, },
                { .ID = HPET_TIMER2, .irq_num = 0, .owner = &ls2k_HPET0, }},
    .irq_Num = INTC0_HPET0_IRQ,
#endif
    .initialized = 0,
};

const void *devHPET0 = (void *)&ls2k_HPET0;
#endif

#if BSP_USE_HPET1
static HPET_t ls2k_HPET1 =
{
    .hwHPET  = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET1_BASE),
#if USE_EXTINT
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = EXTI1_HPET1_0_IRQ, .owner = &ls2k_HPET1, },
                { .ID = HPET_TIMER1, .irq_num = EXTI1_HPET1_1_IRQ, .owner = &ls2k_HPET1, },
                { .ID = HPET_TIMER2, .irq_num = EXTI1_HPET1_2_IRQ, .owner = &ls2k_HPET1, }},
#else
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = 0, .owner = &ls2k_HPET1, },
                { .ID = HPET_TIMER1, .irq_num = 0, .owner = &ls2k_HPET1, },
                { .ID = HPET_TIMER2, .irq_num = 0, .owner = &ls2k_HPET1, }},
    .irq_Num = INTC0_HPET1_IRQ,
#endif
    .initialized = 0,
};

const void *devHPET1 = (void *)&ls2k_HPET1;
#endif

#if BSP_USE_HPET2
static HPET_t ls2k_HPET2 =
{
    .hwHPET  = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET2_BASE),
#if USE_EXTINT
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = EXTI1_HPET2_0_IRQ, .owner = &ls2k_HPET2, },
                { .ID = HPET_TIMER1, .irq_num = EXTI1_HPET2_1_IRQ, .owner = &ls2k_HPET2, },
                { .ID = HPET_TIMER2, .irq_num = EXTI1_HPET2_2_IRQ, .owner = &ls2k_HPET2, }},
#else
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = 0, .owner = &ls2k_HPET2, },
                { .ID = HPET_TIMER1, .irq_num = 0, .owner = &ls2k_HPET2, },
                { .ID = HPET_TIMER2, .irq_num = 0, .owner = &ls2k_HPET2, }},
    .irq_Num = INTC0_HPET2_IRQ,
#endif
    .initialized = 0,
};

const void *devHPET2 = (void *)&ls2k_HPET2;
#endif

#if BSP_USE_HPET3
static HPET_t ls2k_HPET3 =
{
    .hwHPET  = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET3_BASE),
#if USE_EXTINT
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = EXTI1_HPET3_0_IRQ, .owner = &ls2k_HPET3, },
                { .ID = HPET_TIMER1, .irq_num = EXTI1_HPET3_1_IRQ, .owner = &ls2k_HPET3, },
                { .ID = HPET_TIMER2, .irq_num = EXTI1_HPET3_2_IRQ, .owner = &ls2k_HPET3, }},
#else
    .timers  = {{ .ID = HPET_TIMER0, .irq_num = 0, .owner = &ls2k_HPET3, },
                { .ID = HPET_TIMER1, .irq_num = 0, .owner = &ls2k_HPET3, },
                { .ID = HPET_TIMER2, .irq_num = 0, .owner = &ls2k_HPET3, }},
    .irq_Num = INTC0_HPET3_IRQ,
#endif
    .initialized = 0,
};

const void *devHPET3 = (void *)&ls2k_HPET3;
#endif

//-------------------------------------------------------------------------------------------------
// HPET funcs
//-------------------------------------------------------------------------------------------------

extern unsigned int apb_frequency;

/*
 * 纳秒数转换为 hpet clocks
 */
static unsigned long convert_nanoseconds_to_hpetclocks(unsigned long ns)
{
    unsigned long clocks;
    clocks = ns * apb_frequency / 1000000000;
    return clocks;
}

//-----------------------------------------------------------------------------

static void ls2k_hpet_shared_isr(int vector, void *arg);

/*
 * 启动 hpet timer n
 */
static int ls2k_hpet_start_internal(HPET_t *pHPET, int timerID, hpet_cfg_t *cfg)
{
    unsigned long clocks;
    unsigned long next_match;
    struct hpet_timer *ptimer;

    switch (timerID)
    {
        case HPET_TIMER0: ptimer = &pHPET->timers[0]; break;
        case HPET_TIMER1: ptimer = &pHPET->timers[1]; break;
        case HPET_TIMER2: ptimer = &pHPET->timers[2]; break;
        default: return -1;
    }

    if (ptimer->busy)                   /* 定时器已经启动 */
    {
        return 1;
    }

    clocks = convert_nanoseconds_to_hpetclocks(cfg->interval_ns);
    if (clocks == 0)
    {
        return -1;
    }

    if (cfg->isr && !IS_CACHED_ADDR((unsigned long)cfg->isr))
    {
        printk("user isr handle error: 0x%016lx\r\n", (long)cfg->isr);
        return -1;
    }

    if (cfg->cb && !IS_CACHED_ADDR((unsigned long)cfg->cb))
    {
        printk("user callback function error: 0x%016lx\r\n", (long)cfg->cb);
        return -1;
    }

    ptimer->periodic    = (cfg->work_mode == HPET_MODE_CYCLE) ? 1 : 0;
    ptimer->interval_ns = cfg->interval_ns;
    ptimer->as_clocks   = clocks;
    ptimer->work_mode   = cfg->work_mode;

    ptimer->user_isr    = cfg->isr;
    ptimer->cb          = cfg->cb;

    ptimer->p_event     = (osal_event_t)cfg->event;

#if USE_EXTINT
    /*
     * 使用用户自定义中断函数
     */
    if (ptimer->user_isr)
    {
        ls2k_install_irq_handler(ptimer->irq_num, ptimer->user_isr, (void *)ptimer);
    }

    ls2k_interrupt_enable(ptimer->irq_num);
#else
    ls2k_interrupt_enable(pHPET->irq_Num);
#endif

    /*
     * 启动目标定时器
     */
    next_match = pHPET->hwHPET->counter + clocks;

    switch (timerID)
    {
        case HPET_TIMER0:
            pHPET->hwHPET->tm0cmp = next_match;
            pHPET->hwHPET->tm0cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;

        case HPET_TIMER1:
            pHPET->hwHPET->tm1cmp = next_match;
            pHPET->hwHPET->tm1cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;

        case HPET_TIMER2:
            pHPET->hwHPET->tm2cmp = next_match;
            pHPET->hwHPET->tm2cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;
    }

    /*
     * 开启主计数器
     */
    if (!(pHPET->hwHPET->config & HPET_ENABLE_CNF))
    {
        pHPET->hwHPET->config = HPET_ENABLE_CNF;
    }

    ptimer->busy = 1;

    return 0;
}

/*
 * 停止 hpet match
 */
static int ls2k_hpet_stop_internal(HPET_t *pHPET, int timerID)
{
    struct hpet_timer *ptimer;

    switch (timerID)
    {
        case HPET_TIMER0:
            pHPET->hwHPET->tm0cfg &= ~HPET_INT_EN;
            ptimer = &pHPET->timers[0];
            break;

        case HPET_TIMER1:
            pHPET->hwHPET->tm1cfg &= ~HPET_INT_EN;
            ptimer = &pHPET->timers[1];
            break;

        case HPET_TIMER2:
            pHPET->hwHPET->tm2cfg &= ~HPET_INT_EN;
            ptimer = &pHPET->timers[2];
            break;

        default:
            return -1;
    }

#if USE_EXTINT
    ls2k_interrupt_disable(ptimer->irq_num);
    if (ptimer->user_isr)
    {
        /*
         * 换回共享中断
         */
        ls2k_install_irq_handler(ptimer->irq_num, ls2k_hpet_shared_isr, (void *)ptimer);
    }
#endif

    ptimer->as_clocks = 0;
    ptimer->work_mode = 0;
    ptimer->cb = NULL;
    ptimer->p_event = NULL;

    ptimer->busy = 0;


    /*
     * 三个定时器全部停止
     */
    if (!(pHPET->timers[0].busy || pHPET->timers[1].busy || pHPET->timers[2].busy))
    {
        pHPET->hwHPET->config &= ~HPET_ENABLE_CNF;      /* 关闭主计数器 */
#if !USE_EXTINT
        ls2k_interrupt_disable(pHPET->irq_Num);
#endif
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Interrupt
//-------------------------------------------------------------------------------------------------

static int ls2k_hpet_do_irq_callback(HPET_t *pHPET, int timerID)
{
    int stop = 0;
    struct hpet_timer *ptimer;

    switch (timerID)
    {
        case HPET_TIMER0: ptimer = &pHPET->timers[0]; break;
        case HPET_TIMER1: ptimer = &pHPET->timers[1]; break;
        case HPET_TIMER2: ptimer = &pHPET->timers[2]; break;
        default: return -1;
    }

    /******************************************************
     * 回调函数
     */
    if (ptimer->cb)
    {
        (ptimer->cb)((void *)pHPET, timerID, &stop);

        if (stop && ptimer->periodic)       /* Stop it */
        {
            ls2k_hpet_stop_internal(pHPET, timerID);
        }

        return 0;           /* if callback, ingore event */
    }

    /******************************************************
     * Timer Event
     */
    if (ptimer->p_event)
    {
        osal_event_send(ptimer->p_event, HPET_TIMER_EVENT);
    }

    return 0;
}

/*
 * HPET 共享中断处理程序
 */
static void ls2k_hpet_shared_isr(int vector, void *arg)
{
    unsigned int hpet_isr;
    HPET_t *pHPET;

    if (NULL == arg)
        return;

#if USE_EXTINT
    struct hpet_timer *ptimer = (struct hpet_timer *)arg;
    pHPET = ptimer->owner;
    if (NULL == pHPET)
        return;

#else
    pHPET = (HPET_t *)arg;
#endif

    /*
     * HPET中断状态
     */
    hpet_isr = pHPET->hwHPET->isr;

    /*
     * HPET_TIMER0 产生中断
     */
    if ((hpet_isr & HPET_TM0_INT_STS)
#if USE_EXTINT
        && (ptimer == &pHPET->timers[0])
#endif
       )
    {
        pHPET->hwHPET->isr |= HPET_TM0_INT_STS;                         /* clear isr flag */

        if (pHPET->timers[0].periodic && pHPET->timers[0].as_clocks)    /* Set next timer match */
        {
            pHPET->hwHPET->tm0cmp += pHPET->timers[0].as_clocks;
            pHPET->hwHPET->tm0cfg |= HPET_INT_EN;
        }

#if !USE_EXTINT
        /**************************************************
         * 用户自定义中断
         */
        if (pHPET->timers[0].user_isr)
        {
            pHPET->timers[0].user_isr(HPET_TIMER0, arg);
        }
        else
#endif

        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER0);
    }

    /*
     * HPET_TIMER1 产生中断
     */
    if ((hpet_isr & HPET_TM1_INT_STS)
#if USE_EXTINT
        && (ptimer == &pHPET->timers[1])
#endif
       )
    {
        pHPET->hwHPET->isr |= HPET_TM1_INT_STS;                         /* clear isr flag */

        if (pHPET->timers[1].periodic && pHPET->timers[1].as_clocks)    /* Set next timer match */
        {
            pHPET->hwHPET->tm1cmp += pHPET->timers[1].as_clocks;
            pHPET->hwHPET->tm1cfg |= HPET_INT_EN;
        }

#if !USE_EXTINT
        /**************************************************
         * 用户自定义中断
         */
        if (pHPET->timers[1].user_isr)
        {
            pHPET->timers[1].user_isr(HPET_TIMER1, arg);
        }
        else
#endif

        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER1);
    }

    /*
     * HPET_TIMER2 产生中断
     */
    if ((hpet_isr & HPET_TM2_INT_STS)
#if USE_EXTINT
        && (ptimer == &pHPET->timers[2])
#endif
       )
    {
        pHPET->hwHPET->isr |= HPET_TM2_INT_STS;                         /* clear isr flag */

        if (pHPET->timers[2].periodic && pHPET->timers[2].as_clocks)    /* Set next timer match */
        {
            pHPET->hwHPET->tm2cmp += pHPET->timers[2].as_clocks;
            pHPET->hwHPET->tm2cfg |= HPET_INT_EN;
        }

#if !USE_EXTINT
        /**************************************************
         * 用户自定义中断
         */
        if (pHPET->timers[2].user_isr)
        {
            pHPET->timers[2].user_isr(HPET_TIMER2, arg);
        }
        else
#endif

        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER2);
    }
}

//-------------------------------------------------------------------------------------------------
// driver implement
//-------------------------------------------------------------------------------------------------

/*
 * parameters:
 *  dev:  devHPETx
 *  arg:  NULL
 */
STATIC_DRV int HPET_initialize(const void *dev, void *arg)
{
    HPET_t *pHPET = (HPET_t *)dev;

    if (NULL == pHPET)
        return -1;

    if (pHPET->initialized)
        return 0;

    pHPET->hwHPET->config  = 0;             /* 停止HPET基数 */
    pHPET->hwHPET->counter = 0;

    /*
     * 安装中断
     */
#if USE_EXTINT
  #if BSP_USE_HPET0
	if (dev == devHPET0)
        OR_REG32(CHIP_CTRL0_BASE, 1 << 12);
  #endif

  #if BSP_USE_HPET1
    if (dev == devHPET1)
        OR_REG32(CHIP_CTRL0_BASE, 1 << 13);
  #endif

  #if BSP_USE_HPET2
    if (dev == devHPET2)
        OR_REG32(CHIP_CTRL0_BASE, 1 << 14);
  #endif

  #if BSP_USE_HPET3
    if (dev == devHPET3)
        OR_REG32(CHIP_CTRL0_BASE, 1 << 15);
  #endif

    for (int i=0; i<3; i++)
    {
        struct hpet_timer *ptimer = &pHPET->timers[i];

        ls2k_interrupt_disable(ptimer->irq_num);
        ls2k_install_irq_handler(ptimer->irq_num, ls2k_hpet_shared_isr, (void *)ptimer);
    }

#else

    AND_REG32(CHIP_CTRL0_BASE, ~CTRL0_HPET_INT_SHARE_MASK);

    ls2k_interrupt_disable(pHPET->irq_Num);

    ls2k_install_irq_handler(pHPET->irq_Num, ls2k_hpet_shared_isr, (void *)pHPET);

    ls2k_set_irq_routeip(pHPET->irq_Num, INT_ROUTE_IP3);

#endif

    pHPET->initialized = 1;

    return 0;
}

/*
 * 开启定时器
 *
 * parameters:
 *  dev:  devHPETx
 *  arg:  hpet_cfg_t *
 */
STATIC_DRV int HPET_open(const void *dev, void *arg)
{
    HPET_t *pHPET = (HPET_t *)dev;

    if ((NULL != pHPET) && pHPET->initialized && arg)
    {
        hpet_cfg_t *cfg = (hpet_cfg_t *)arg;

        return ls2k_hpet_start_internal(pHPET, cfg->timer, cfg);
    }

    return -1;
}

/*
 * 停止定时器
 *
 * parameters:
 *  dev:  devHPETx
 *  arg:  HPET_TIMERn
 */
STATIC_DRV int HPET_close(const void *dev, void *arg)
{
    HPET_t *pHPET = (HPET_t *)dev;

    if ((NULL != pHPET) && pHPET->initialized && arg)
    {
        int timer = (long)arg;
        return ls2k_hpet_stop_internal(pHPET, timer);
    }

    return -1;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * HPET driver operators
 */
static const driver_ops_t ls2k_hpet_drv_ops =
{
    .init_entry  = HPET_initialize,
    .open_entry  = HPET_open,
    .close_entry = HPET_close,
    .read_entry  = NULL,
    .write_entry = NULL,
    .ioctl_entry = NULL,
};

const driver_ops_t *hpet_drv_ops = &ls2k_hpet_drv_ops;
#endif

/******************************************************************************
 * User API
 */

int ls2k_hpet_timer_start(const void *hpet, int timerID, hpet_cfg_t *cfg)
{
    HPET_t *pHPET = (HPET_t *)hpet;

    if ((NULL != pHPET) && (NULL != cfg))
    {
        if (!pHPET->initialized)
        {
            HPET_initialize(hpet, NULL);
        }

        if (pHPET->initialized)
        {
            return ls2k_hpet_start_internal(pHPET, timerID, cfg);
        }
    }

    return -1;
}

int ls2k_hpet_timer_stop(const void *hpet, int timerID)
{
    HPET_t *pHPET = (HPET_t *)hpet;

    if ((NULL != pHPET) && pHPET->initialized)
    {
        return ls2k_hpet_stop_internal(pHPET, timerID);
    }

    return -1;
}

#endif // #if defined(BSP_USE_HPET)

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
