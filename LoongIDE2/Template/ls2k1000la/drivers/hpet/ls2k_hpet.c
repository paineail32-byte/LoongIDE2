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

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

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
    unsigned int        interval_ns;            /* timer irq trigger gap of ns */
    unsigned int        as_clocks;
    int                 work_mode;              /* 周期/边缘, 周期/电平 */
    int                 periodic;               /* 周期 */
    int                 irq_num;                /* 中断号 */
    irq_handler_t       isr;                    /* User defined match-isr */
    hpetimer_callback_t callback;               /* callback when match-irq ocurred */
	osal_event_t        p_event;                /* Send the RTOS event when irq ocurred */
    int                 busy;
};

typedef struct
{
	HW_HPET_t *hwHPET;
	
    struct hpet_dev hpets[3];                   /* hpet virtual timers array */
    
    int initialized;
} HPET_t;

/*
 * HPET device
 */
static HPET_t ls2k_HPET0 =
{
    .hwHPET = NULL,
    .initialized = 0,
};

const void *devHPET0 = (void *)&ls2k_HPET0;

//-------------------------------------------------------------------------------------------------
// Forward declaration
//-------------------------------------------------------------------------------------------------

static int ls2k_hpet_install_isr(int timer);

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
 * 启动 HPET_TIMERn
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

    
    pHPET->hpets[index].periodic = (cfg->work_mode == HPET_MODE_CYCLE) ? 1 : 0;

    pHPET->hpets[index].interval_ns = cfg->interval_ns;
    pHPET->hpets[index].as_clocks   = (unsigned int)clocks;
    pHPET->hpets[index].work_mode   = cfg->work_mode;
    pHPET->hpets[index].isr         = cfg->isr;
    pHPET->hpets[index].callback    = cfg->cb;
    pHPET->hpets[index].p_event     = (osal_event_t)cfg->event;

    ls2k_hpet_install_isr(timer);           /* Install interrupt handler */

    /*
     * 开启主计数器
     */
    if (!(pHPET->hwHPET->config & HPET_ENABLE_CNF))
    {
        pHPET->hwHPET->config = HPET_ENABLE_CNF;
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

    ls2k_remove_irq_handler(pHPET->hpets[index].irq_num);

    pHPET->hpets[index].as_clocks = 0;
    pHPET->hpets[index].work_mode = 0;
    pHPET->hpets[index].isr       = NULL;
    pHPET->hpets[index].callback  = NULL;
    pHPET->hpets[index].p_event   = NULL;
    pHPET->hpets[index].busy      = 0;

    /*
     * 三个定时器全部停止
     */
    if (!(pHPET->hpets[0].busy || pHPET->hpets[1].busy || pHPET->hpets[2].busy))
    {
        pHPET->hwHPET->config &= ~HPET_ENABLE_CNF;      /* 关闭主计数器 */
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Interrupt
//-------------------------------------------------------------------------------------------------

/*
 * 中断响应程序
 */
static void ls2k_hpet_common_isr(int vector, void *arg)
{
    int timer, index, stop = 0;
    HPET_t *pHPET = &ls2k_HPET0;
    timer = (long)arg;
    
    switch (timer)
    {
        case HPET_TIMER0:
            pHPET->hwHPET->isr |= HPET_TM0_INT_STS;                     /* clear isr flag */
            if (pHPET->hpets[0].periodic && pHPET->hpets[0].as_clocks)  /* Set next timer match */
            {
                pHPET->hwHPET->tm0cmp += pHPET->hpets[0].as_clocks;
                pHPET->hwHPET->tm0cfg |= HPET_INT_EN;
            }
            index = 0;
            break;

        case HPET_TIMER1:
            pHPET->hwHPET->isr |= HPET_TM1_INT_STS;                     /* clear isr flag */
            if (pHPET->hpets[1].periodic && pHPET->hpets[1].as_clocks)  /* Set next timer match */
            {
                pHPET->hwHPET->tm1cmp += pHPET->hpets[1].as_clocks;
                pHPET->hwHPET->tm1cfg |= HPET_INT_EN;
            }
            index = 1;
            break;

        case HPET_TIMER2:
            pHPET->hwHPET->isr |= HPET_TM2_INT_STS;                     /* clear isr flag */
            if (pHPET->hpets[2].periodic && pHPET->hpets[2].as_clocks)  /* Set next timer match */
            {
                pHPET->hwHPET->tm2cmp += pHPET->hpets[2].as_clocks;
                pHPET->hwHPET->tm2cfg |= HPET_INT_EN;
            }
            index = 2;
            break;

        default:
            return;
    }

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

        return;                                 /* ingore event */
    }

    /******************************************************
     * Timer Event
     */
    if (pHPET->hpets[index].p_event)
    {
        osal_event_send(pHPET->hpets[index].p_event, HPET_TIMER_EVENT);
    }
}

static int ls2k_hpet_install_isr(int timer)
{
    int index, irqnum;
    irq_handler_t isr = NULL;
    HPET_t *pHPET = &ls2k_HPET0;
    
    switch (timer)
    {
        case HPET_TIMER0: index = 0; break;
        case HPET_TIMER1: index = 1; break;
        case HPET_TIMER2: index = 2; break;
        default: return -1;
    }

    irqnum = pHPET->hpets[index].irq_num;
    ls2k_interrupt_disable(irqnum);

    isr = pHPET->hpets[index].isr;
    if (isr == NULL)
    {
        ls2k_install_irq_handler(irqnum, ls2k_hpet_common_isr, (void *)(unsigned long)timer);
        printk("install hpet common isr: %i\n", irqnum);
    }
    else
    {
        ls2k_install_irq_handler(irqnum, isr, (void *)(unsigned long)timer);
        printk("install hpet isr: %i\n", irqnum);
    }

    ls2k_set_irq_routeip(irqnum, INT_ROUTE_IP3);
    // ls2k_set_irq_triggermode(irqnum, INT_TRIGGER_LEVEL);

    ls2k_interrupt_enable(irqnum);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// driver implement
//-------------------------------------------------------------------------------------------------

/*
 * parameters:
 *  dev:  NULL/devHPET
 *  arg:  NULL
 */
STATIC_DRV int HPET_initialize(const void *dev, void *arg)
{
    unsigned long apb_address0;
    HPET_t *pHPET = &ls2k_HPET0;
    
    if (pHPET->initialized)
    {
        return 0;
    }

    memset((void *)pHPET, 0, sizeof(HPET_t));

    /*
     * LS2K1000LA 从APB配置头读 HPET IO base
     */
    apb_address0 = READ_REG64(APB_CFG_HEAD_BASE + 0x10) & ~0x0Ful;
    pHPET->hwHPET = (HW_HPET_t *)PHYS_TO_UNCACHED(apb_address0 + 0x4000);

    pHPET->hwHPET->config  = 0;             /* 停止HPET基数 */
    pHPET->hwHPET->counter = 0;

    pHPET->hpets[0].irq_num = INTC0_HPET0_IRQ;
    pHPET->hpets[1].irq_num = INTC1_HPET1_IRQ;
    pHPET->hpets[2].irq_num = INTC1_HPET2_IRQ;

    pHPET->initialized = 1;

    return 0;
}

/*
 * 开启定时器
 *
 * parameters:
 *  dev:  NULL/HPET
 *  arg:  hpet_cfg_t *
 */
STATIC_DRV int HPET_open(const void *dev, void *arg)
{
    HPET_t *pHPET = &ls2k_HPET0;
    
    if (pHPET->initialized && arg)
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
 *  dev:  HPET_TIMERx
 *  arg:  NULL
 */
STATIC_DRV int HPET_close(const void *dev, void *arg)
{
    HPET_t *pHPET = &ls2k_HPET0;

    if (pHPET->initialized && arg)
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
    HPET_t *pHPET = &ls2k_HPET0;

    if (NULL != cfg)
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
    HPET_t *pHPET = &ls2k_HPET0;

    if (pHPET->initialized)
    {
        return ls2k_hpet_stop_internal(pHPET, timer);
    }

    return -1;
}

#endif // #if defined(BSP_USE_HPET)

/*
 * @@ END
 */
 
