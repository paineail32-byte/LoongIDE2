/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_hpet.c
 *
 * created: 2022-10-13
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_HPET

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "ls2k500.h"
#include "ls2k500_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_hpet_hw.h"
#include "ls2k_hpet.h"

//-------------------------------------------------------------------------------------------------
// definition
//-------------------------------------------------------------------------------------------------

struct hpet_dev
{
    unsigned int interval_ns;           /* timer irq trigger gap of ns */
    unsigned int as_clocks;
    int          work_mode;             /* 周期/边缘, 周期/电平 */
    int          periodic;              /* 周期 */
    hpetimer_callback_t callback;       /* callback when match-irq ocurred */
	osal_event_t p_event;               /* Send the RTOS event when irq ocurred */
    int          busy;
};

typedef struct
{
	HW_HPET_t *hwHPET;

    /*
     * LS2K500 的HPET是一个设备的三个定时器共享一个中断.
     */
    int    irq_num;                         /* 中断号 */
    struct hpet_dev hpets[3];               /* hpet virtual devices array */
    int    initialized;
} HPET_t;

/*
 * HPET device
 */
#if BSP_USE_HPET0
static HPET_t ls2k_HPET0 =
{
    .hwHPET      = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET0_BASE),
#if USE_EXTINT
    .irq_num     = EXTINTC0_HPET0_IRQ,
#else
    .irq_num     = INTC0_HPET0_IRQ,
#endif
    .hpets       = {{ 0 }, { 0 }, { 0 }},
    .initialized = 0,
};

const void *devHPET0 = (void *)&ls2k_HPET0;
#endif

#if BSP_USE_HPET1
static HPET_t ls2k_HPET1 =
{
    .hwHPET      = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET1_BASE),
#if USE_EXTINT
    .irq_num     = EXTINTC0_HPET1_IRQ,
#else
    .irq_num     = INTC1_HPET1_IRQ,
#endif
    .hpets       = {{ 0 }, { 0 }, { 0 }},
    .initialized = 0,
};

const void *devHPET1 = (void *)&ls2k_HPET1;
#endif

#if BSP_USE_HPET2
static HPET_t ls2k_HPET2 =
{
    .hwHPET      = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET2_BASE),
#if USE_EXTINT
    .irq_num     = EXTINTC0_HPET2_IRQ,
#else
    .irq_num     = INTC1_HPET2_IRQ,
#endif
    .hpets       = {{ 0 }, { 0 }, { 0 }},
    .initialized = 0,
};

const void *devHPET2 = (void *)&ls2k_HPET2;
#endif

#if BSP_USE_HPET3
static HPET_t ls2k_HPET3 =
{
    .hwHPET      = (HW_HPET_t *)PHYS_TO_UNCACHED(HPET3_BASE),
#if USE_EXTINT
    .irq_num     = EXTINTC0_HPET3_IRQ,
#else
    .irq_num     = INTC1_HPET3_IRQ,
#endif
    .hpets       = {{ 0 }, { 0 }, { 0 }},
    .initialized = 0,
};

const void *devHPET3 = (void *)&ls2k_HPET3;
#endif

//-------------------------------------------------------------------------------------------------
// HPET operator
//-------------------------------------------------------------------------------------------------

extern unsigned int apb_frequency;

/*
 * 纳秒数转换为 hpet clocks
 */
static int convert_nanoseconds_to_hpetclocks(unsigned int ns)
{
    long clocks;
    clocks = (long)ns * apb_frequency / 1000000000;
    return (int)clocks;
}

/*
 * 启动 hpet timer n
 */
static int ls2k_hpet_start_internal(HPET_t *pHPET, int timer, hpet_cfg_t *cfg)
{
    int index, clocks;
    unsigned int next_match;

    switch (timer)
    {
        case HPET_TIMER0: index = 0; break;
        case HPET_TIMER1: index = 1; break;
        case HPET_TIMER2: index = 2; break;
        default: return -1;
    }

    if (pHPET->hpets[index].busy)                   /* 定时器已经启动 */
        return 1;

    /*
     * 这里计算clocks 有个超出 0xffffffff的问题
     */
    clocks = convert_nanoseconds_to_hpetclocks(cfg->interval_ns);
    if (clocks <= 0)
        return -1;

    pHPET->hpets[index].periodic    = (cfg->work_mode == HPET_MODE_CYCLE) ? 1 : 0;
    pHPET->hpets[index].interval_ns = cfg->interval_ns;
    pHPET->hpets[index].as_clocks   = (unsigned int)clocks;
    pHPET->hpets[index].work_mode   = cfg->work_mode;
    pHPET->hpets[index].callback    = cfg->cb;
    pHPET->hpets[index].p_event     = (osal_event_t)cfg->event;

    ls2k_interrupt_enable(pHPET->irq_num);              /* 能中断 */

    if (!(pHPET->hwHPET->config & HPET_ENABLE_CNF))
    {
        pHPET->hwHPET->config = HPET_ENABLE_CNF;        /* 开启主计数器 */
    }

    /*
     * 启动目标定时器
     */
    next_match = pHPET->hwHPET->counter + clocks;

    switch (index)
    {
        case 0:
            pHPET->hwHPET->tm0cmp = next_match;
            pHPET->hwHPET->tm0cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;

        case 1:
            pHPET->hwHPET->tm1cmp = next_match;
            pHPET->hwHPET->tm1cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;

        case 2:
            pHPET->hwHPET->tm2cmp = next_match;
            pHPET->hwHPET->tm2cfg = HPET_INT_EN | HPET_INT_LEVEL;
            break;
    }

    pHPET->hpets[index].busy = 1;

    return 0;
}

/*
 * 停止 hpet match
 */
static int ls2k_hpet_stop_internal(HPET_t *pHPET, int timer)
{
    int index;

    switch (timer)
    {
        case HPET_TIMER0:
            pHPET->hwHPET->tm0cfg &= ~HPET_INT_EN;
            index = 0;
            break;

        case HPET_TIMER1:
            pHPET->hwHPET->tm1cfg &= ~HPET_INT_EN;
            index = 1;
            break;

        case HPET_TIMER2:
            pHPET->hwHPET->tm2cfg &= ~HPET_INT_EN;
            index = 2;
            break;
            
        default:
            return -1;
    }

    pHPET->hpets[index].as_clocks = 0;
    pHPET->hpets[index].work_mode = 0;
    pHPET->hpets[index].callback  = NULL;
    pHPET->hpets[index].p_event   = NULL;

    pHPET->hpets[index].busy = 0;

    /*
     * 三个定时器全部停止  
     */
    if (!(pHPET->hpets[0].busy || pHPET->hpets[1].busy || pHPET->hpets[2].busy))
    {
        pHPET->hwHPET->config &= ~HPET_ENABLE_CNF;      /* 关闭主计数器 */
        ls2k_interrupt_disable(pHPET->irq_num);         /* 禁止中断 */
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Interrupt
//-------------------------------------------------------------------------------------------------

static int ls2k_hpet_do_irq_callback(HPET_t *pHPET, int timer, int index)
{
    int stop = 0;

    /*
     * 回调函数
     */
    if (pHPET->hpets[index].callback)
    {
        (pHPET->hpets[index].callback)((void *)pHPET, timer, &stop);

        if (stop && pHPET->hpets[index].periodic)   /* Stop it */
        {
            ls2k_hpet_stop_internal(pHPET, timer);
        }

        return 0;               /* if callback, ingore event */
    }

    /******************************************************
     * Timer Event
     */
    if (pHPET->hpets[index].p_event)
    {
        osal_event_send(pHPET->hpets[index].p_event, HPET_TIMER_EVENT);
    }

    return 0;
}

/*
 * 中断处理程序
 */
static void ls2k_hpet_common_isr(int vector, void *arg)
{
    HPET_t *pHPET;
    unsigned int hpet_isr;

    if (NULL == arg)
        return;

    pHPET = (HPET_t *)arg;
    
    /*
     * HPET中断状态
     */
    hpet_isr = pHPET->hwHPET->isr;      

    /*
     * HPET_TIMER0 产生中断
     */
    if (hpet_isr & HPET_TM0_INT_STS)
    {
        pHPET->hwHPET->isr |= HPET_TM0_INT_STS;                     /* clear isr flag */

        if (pHPET->hpets[0].periodic && pHPET->hpets[0].as_clocks)  /* Set next timer match */
        {
            pHPET->hwHPET->tm0cmp += pHPET->hpets[0].as_clocks;
            pHPET->hwHPET->tm0cfg |= HPET_INT_EN;
        }

        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER0, 0);
    }

    /*
     * HPET_TIMER1 产生中断
     */
    if (hpet_isr & HPET_TM1_INT_STS)
    {
        pHPET->hwHPET->isr |= HPET_TM1_INT_STS;                     /* clear isr flag */

        if (pHPET->hpets[1].periodic && pHPET->hpets[1].as_clocks)  /* Set next timer match */
        {
            pHPET->hwHPET->tm1cmp += pHPET->hpets[1].as_clocks;
            pHPET->hwHPET->tm1cfg |= HPET_INT_EN;
        }
        
        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER1, 1);
    }
    
    /*
     * HPET_TIMER2 产生中断
     */
    if (hpet_isr & HPET_TM2_INT_STS)
    {
        pHPET->hwHPET->isr |= HPET_TM2_INT_STS;                     /* clear isr flag */

        if (pHPET->hpets[2].periodic && pHPET->hpets[2].as_clocks)  /* Set next timer match */
        {
            pHPET->hwHPET->tm2cmp += pHPET->hpets[2].as_clocks;
            pHPET->hwHPET->tm2cfg |= HPET_INT_EN;
        }
        
        ls2k_hpet_do_irq_callback(pHPET, HPET_TIMER2, 2);
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
    ls2k_interrupt_disable(pHPET->irq_num);
    
    ls2k_install_irq_handler(pHPET->irq_num, ls2k_hpet_common_isr, (void *)pHPET);
    
#if !USE_EXTINT
    ls2k_set_irq_routeip(pHPET->irq_num, INT_ROUTE_IP3);
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

int ls2k_hpet_timer_start(const void *hpet, int timer, hpet_cfg_t *cfg)
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
            return ls2k_hpet_start_internal(pHPET, timer, cfg);
        }
    }

    return -1;
}

int ls2k_hpet_timer_stop(const void *hpet, int timer)
{
    HPET_t *pHPET = (HPET_t *)hpet;
    
    if ((NULL != pHPET) && pHPET->initialized)
    {
        return ls2k_hpet_stop_internal(pHPET, timer);
    }

    return -1;
}

#endif // #if defined(BSP_USE_HPET)

/*
 * @@ END
 */
 
