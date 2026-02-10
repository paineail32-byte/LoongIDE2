/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_pwm.c
 *
 * created: 2024-08-04
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_PWM

#include <stdio.h>
#include <stdint.h>

#include "cpu.h"
#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_pwm_hw.h"
#include "ls2k_pwm.h"

//-------------------------------------------------------------------------------------------------
// PWM device
//-------------------------------------------------------------------------------------------------

typedef struct
{
	HW_PWM_t    *hwPWM;                 /* pointer to HW registers */
	unsigned int bus_freq;              /* 总线频率 */
	unsigned int irq_num;               /* interrupt num */
	
	unsigned int lo_level_ns;           /* 低电平时间 ns */
	unsigned int hi_level_ns;           /* 高电平时间 ns */
	int          work_mode;             /* timer or pulse */
    int          single;                /* 单次或者连续 */

    /*
     * 当工作在定时器模式时
     */
    irq_handler_t       isr;            /* user defined interrupt handler */
    pwmtimer_callback_t callback;       /* callback when irq ocurred */

	osal_event_t  p_event;              /* Send the RTOS event when irq ocurred */

    char dev_name[16];
    int  initialized;
    int  busy;
} PWM_t;

//-------------------------------------------------------------------------------------------------

#define PWM_INDEX(pwm) \
        ((VA_TO_PHYS(pwm->hwPWM) == PWM0_BASE)  ?  0 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM1_BASE)  ?  1 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM2_BASE)  ?  2 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM3_BASE)  ?  3 : -1)

//-------------------------------------------------------------------------------------------------
// 微秒转换为 bus clock 数: (ns * bus_freq) / 1000000000
//-------------------------------------------------------------------------------------------------

static inline unsigned int NS_2_BUS_CLOCKS(unsigned int bus_freq, unsigned int ns)
{
    unsigned int clocks;
    clocks = (unsigned int)((double)ns * bus_freq / 1000000000.0);
    return clocks;
}

//-------------------------------------------------------------------------------------------------
// PWM operator
//-------------------------------------------------------------------------------------------------

#define PWM_RESET(pwm) do { pwm->hwPWM->ctrl = PWM_CTRL_RESET | PWM_CTRL_IFLAG; } while (0)

#define PWM_TIMER_START(pwm) \
        do { \
            pwm->hwPWM->ctrl = PWM_CTRL_EN | PWM_CTRL_IEN | PWM_CTRL_SINGLE | PWM_CTRL_OEN; \
        } while (0)

//-------------------------------------------------------------------------------------------------

static int ls2k_pwm_start(PWM_t *pwm)
{
    if ((NULL == pwm) || (!pwm->initialized))
    {
        return -1;
    }

    PWM_RESET(pwm);                                     /* Reset PWM */

    switch (pwm->work_mode)
    {
        case PWM_SINGLE_TIMER:
        case PWM_CONTINUE_TIMER:
            ls2k_interrupt_enable(pwm->irq_num);        /* Enable PWM Interrupt */
            PWM_TIMER_START(pwm);                       /* Start the timer */
            break;

        case PWM_SINGLE_PULSE:
        case PWM_CONTINUE_PULSE:
            if (pwm->single)
            {
                pwm->hwPWM->ctrl = PWM_CTRL_EN | PWM_CTRL_SINGLE;
            }
            else
            {
                pwm->hwPWM->ctrl = PWM_CTRL_EN;
            }
            break;

        default:
            return -1;
    }

    pwm->busy = 1;

    return 0;
}

static int ls2k_pwm_stop(PWM_t *pwm)
{
    if (NULL == pwm)
    {
        return -1;
    }

    /******************************************************
     * Stop PWM
     */
    PWM_RESET(pwm);

    pwm->hwPWM->ctrl = 0;

    /*
     * 关中断
     */
    ls2k_interrupt_disable(pwm->irq_num);
    
    pwm->busy = 0;

    return 0;
}

/*
 * 处理中断
 */
static inline void ls2k_pwm_timer_irq_process(PWM_t *pwm)
{
    int stopme = 0;
    
    if (NULL == pwm)
        return;
        
    /*
     * Continue mode, Restart the timer
     */
    if (!pwm->single)
    {
        PWM_RESET(pwm);             /* Reset PWM */
        PWM_TIMER_START(pwm);
    }

    if (NULL != pwm->callback)      /* Timer callback */
    {
        pwm->callback((void *)pwm, &stopme);
    }

    if (pwm->single || stopme)
    {
        ls2k_pwm_stop(pwm);
    }

    /******************************************************
     * Timer Event of RTOS
     */
    if (pwm->p_event)
    {
        osal_event_send(pwm->p_event, PWM_TIMER_EVENT);
    }
}

/*
 * PWM timer interrupt, only timer mode
 */
static void ls2k_pwm_timer_shared_isr(int vector, void *arg)
{
    PWM_t *pwm;

#if USE_EXTINT

    pwm = (PWM_t *)arg;
    if (NULL != pwm)
    {
        ls2k_pwm_timer_irq_process(pwm);
    }

#else // #if USE_EXTINT

    unsigned int ctrl;
    
    if (vector == INTC0_PWM_0_1_IRQ)
    {
  #if BSP_USE_PWM0
        pwm = (PWM_t *)devPWM0;
        ctrl = pwm->hwPWM->ctrl;
        if ((ctrl & PWM_CTRL_EN) && (ctrl & PWM_CTRL_IEN) && (ctrl & PWM_CTRL_IFLAG))
        {
            ls2k_pwm_timer_irq_process(pwm);
        }
  #endif
  
  #if BSP_USE_PWM1
        pwm = (PWM_t *)devPWM1;
        ctrl = pwm->hwPWM->ctrl;
        if ((ctrl & PWM_CTRL_EN) && (ctrl & PWM_CTRL_IEN) && (ctrl & PWM_CTRL_IFLAG))
        {
            ls2k_pwm_timer_irq_process(pwm);
        }
  #endif
    }
    
    else if (vector == INTC0_PWM_2_3_IRQ)
    {
  #if BSP_USE_PWM2
        pwm = (PWM_t *)devPWM2;
        ctrl = pwm->hwPWM->ctrl;
        if ((ctrl & PWM_CTRL_EN) && (ctrl & PWM_CTRL_IEN) && (ctrl & PWM_CTRL_IFLAG))
        {
            ls2k_pwm_timer_irq_process(pwm);
        }
  #endif

  #if BSP_USE_PWM3
        pwm = (PWM_t *)devPWM3;
        ctrl = pwm->hwPWM->ctrl;
        if ((ctrl & PWM_CTRL_EN) && (ctrl & PWM_CTRL_IEN) && (ctrl & PWM_CTRL_IFLAG))
        {
            ls2k_pwm_timer_irq_process(pwm);
        }
  #endif
    }

#endif // #if USE_EXTINT

}

//-------------------------------------------------------------------------------------------------
// PWM driver implement
//-------------------------------------------------------------------------------------------------

extern int ls2k_pwm_init_hook(const void *dev);

STATIC_DRV int PWM_initialize(const void *dev, void *arg)
{
    PWM_t *pwm = (PWM_t *)dev;

    if (NULL == pwm)
    {
        return -1;
    }

    if (pwm->initialized)
    {
        return 0;
    }

    ls2k_pwm_init_hook(dev);
    
    pwm->bus_freq = apb_frequency;

    pwm->hi_level_ns = 0;
    pwm->lo_level_ns = 0;
    pwm->work_mode   = 0;
    pwm->single      = 0;
    pwm->isr         = NULL;
    pwm->callback    = NULL;
    pwm->p_event     = NULL;

    /******************************************************************
     * Config PWM interrupt
     */
    //
    
    pwm->initialized = 1;

    return 0;
}

STATIC_DRV int PWM_open(const void *dev, void *arg)
{
    PWM_t *pwm = (PWM_t *)dev;
    pwm_cfg_t *cfg = (pwm_cfg_t *)arg;
    unsigned int hi_clocks, lo_clocks;

    if ((NULL == pwm) || (!pwm->initialized))
    {
        return -1;
    }

    if (pwm->busy)
    {
        return -2;
    }

    pwm->hi_level_ns = cfg->hi_ns;
    pwm->lo_level_ns = cfg->lo_ns;
    pwm->work_mode   = cfg->mode;

    pwm->single = (cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_SINGLE_TIMER);

    /*
     * Configure the PWM
     */
    hi_clocks = NS_2_BUS_CLOCKS(pwm->bus_freq, pwm->hi_level_ns);
    lo_clocks = NS_2_BUS_CLOCKS(pwm->bus_freq, pwm->lo_level_ns);

    switch (pwm->work_mode)
    {
        case PWM_SINGLE_PULSE:
        case PWM_CONTINUE_PULSE:
            if ((hi_clocks == 0) || (lo_clocks == 0))
            {
                printk("Error: PWM%i register zero, high=0x%08x, low=0x%08x, too small.\r\n",
                        PWM_INDEX(pwm), hi_clocks, lo_clocks);
                return -1;
            }

            pwm->hwPWM->counter   = 0;
            pwm->hwPWM->lowlevel  = lo_clocks - 1;
            pwm->hwPWM->fullpulse = hi_clocks + lo_clocks - 1;
            break;

        case PWM_SINGLE_TIMER:
        case PWM_CONTINUE_TIMER:
            if (hi_clocks == 0)
            {
                printk("Error: PWM%i register zero, high=0x%08x, too small.\r\n",
                        PWM_INDEX(pwm), hi_clocks);
                return -1;
            }

            pwm->hwPWM->counter   = 0;
            pwm->hwPWM->lowlevel  = hi_clocks >> 1;
            pwm->hwPWM->fullpulse = hi_clocks - 1;
            break;
    }

    /******************************************************************
     * Install PWM Timer isr
     */
    if ((pwm->work_mode == PWM_SINGLE_TIMER) || (pwm->work_mode == PWM_CONTINUE_TIMER))
    {
        pwm->isr      = cfg->isr;
        pwm->callback = cfg->cb;
        pwm->p_event  = (osal_event_t)cfg->event;

        if (pwm->isr && (((long)cfg->isr >> 32) == 0x90000000)) /* 加个验证  */
        {
            ls2k_install_irq_handler(pwm->irq_num, pwm->isr, (void *)pwm);
        }
        else
        {
            ls2k_install_irq_handler(pwm->irq_num, ls2k_pwm_timer_shared_isr, (void *)pwm);
        }

#if !USE_EXTINT
        ls2k_set_irq_routeip(pwm->irq_num, INT_ROUTE_IP2);
#endif
    }
    else
    {
        pwm->isr      = NULL;
        pwm->callback = NULL;
        pwm->p_event  = NULL;
    }

    /******************************************************
     * Start the timer/pulsing
     */
    return ls2k_pwm_start(pwm);
}

STATIC_DRV int PWM_close(const void *dev, void *arg)
{
    int rt;
    PWM_t *pwm = (PWM_t *)dev;

    if (NULL == pwm)
    {
        return -1;
    }

    if (!pwm->busy)
    {
        return 0;
    }

    rt = ls2k_pwm_stop(pwm);

    if ((pwm->work_mode == PWM_SINGLE_TIMER) || (pwm->work_mode == PWM_CONTINUE_TIMER))
    {
        ls2k_remove_irq_handler(pwm->irq_num);      /* uninstall isr? */
    }

    pwm->callback = NULL;
    pwm->p_event  = NULL;

    return rt;
}

//-------------------------------------------------------------------------------------------------
// PWM devices
//-------------------------------------------------------------------------------------------------

/* PWM 0 */
#if BSP_USE_PWM0
static PWM_t ls2k_PWM0 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM0_BASE),
#if USE_EXTINT
	.irq_num  = EXTI0_PWM0_IRQ,
#else
	.irq_num  = INTC0_PWM_0_1_IRQ,
#endif
	.dev_name = "pwm0",
	.initialized = 0,
};
const void *devPWM0 = (void *)&ls2k_PWM0;
#endif

/* PWM 1 */
#if BSP_USE_PWM1
static PWM_t ls2k_PWM1 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM1_BASE),
#if USE_EXTINT
	.irq_num  = EXTI0_PWM1_IRQ,
#else
	.irq_num  = INTC0_PWM_0_1_IRQ,
#endif
	.dev_name = "pwm1",
	.initialized = 0,
};
const void *devPWM1 = (void *)&ls2k_PWM1;
#endif

/* PWM 2 */
#if BSP_USE_PWM2
static PWM_t ls2k_PWM2 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM2_BASE),
#if USE_EXTINT
	.irq_num  = EXTI0_PWM2_IRQ,
#else
	.irq_num  = INTC0_PWM_2_3_IRQ,
#endif
	.dev_name = "pwm2",
	.initialized = 0,
};
const void *devPWM2 = (void *)&ls2k_PWM2;
#endif

/* PWM 3 */
#if BSP_USE_PWM3
static PWM_t ls2k_PWM3 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM3_BASE),
#if USE_EXTINT
	.irq_num  = EXTI0_PWM3_IRQ,
#else
	.irq_num  = INTC0_PWM_2_3_IRQ,
#endif
	.dev_name = "pwm3",
	.initialized = 0,
};
const void *devPWM3 = (void *)&ls2k_PWM3;
#endif

#if (PACK_DRV_OPS)
/******************************************************************************
 * PWM driver operators
 */
static const driver_ops_t ls2k_pwm_drv_ops =
{
    .init_entry  = PWM_initialize,
    .open_entry  = PWM_open,
    .close_entry = PWM_close,
    .read_entry  = NULL,
    .write_entry = NULL,
    .ioctl_entry = NULL,
};

const driver_ops_t *pwm_drv_ops = &ls2k_pwm_drv_ops;
#endif

/******************************************************************************
 * user api
 */
int ls2k_pwm_pulse_start(void *pwm, pwm_cfg_t *cfg)
{
    if ((pwm == NULL) || (cfg == NULL))
        return -1;

    if ((cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_CONTINUE_PULSE))
    {
        if (PWM_initialize(pwm, NULL) == 0)
        {
            return PWM_open(pwm, cfg);
        }
    }

    return -1;
}

int ls2k_pwm_pulse_stop(void *pwm)
{
    if (pwm == NULL)
        return -1;

    if ((((PWM_t *)pwm)->work_mode == PWM_SINGLE_PULSE) ||
        (((PWM_t *)pwm)->work_mode == PWM_CONTINUE_PULSE))
    {
        return PWM_close(pwm, NULL);
    }

    return -1;
}

int ls2k_pwm_timer_start(void *pwm, pwm_cfg_t *cfg)
{
    if ((pwm == NULL) || (cfg == NULL))
        return -1;

    if ((cfg->mode == PWM_SINGLE_TIMER) || (cfg->mode == PWM_CONTINUE_TIMER))
    {
        if (PWM_initialize(pwm, NULL) == 0)
        {
            return PWM_open(pwm, cfg);
        }
    }

    return -1;
}

int ls2k_pwm_timer_stop(void *pwm)
{
    if (pwm == NULL)
        return -1;

    if ((((PWM_t *)pwm)->work_mode == PWM_SINGLE_TIMER) ||
        (((PWM_t *)pwm)->work_mode == PWM_CONTINUE_TIMER))
    {
        return PWM_close(pwm, NULL);
    }

    return -1;
}

/******************************************************************************
 * Device name
 */
const char *ls2k_pwm_get_device_name(const void *pwm)
{
    if (NULL == pwm)
        return NULL;
    return ((PWM_t *)pwm)->dev_name;
}

#endif // #if BSP_USE_PWM


