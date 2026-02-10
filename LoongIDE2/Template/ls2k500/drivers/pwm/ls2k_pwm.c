/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_pwm.c
 *
 * created: 2022-03-10
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_PWM

#include <stdio.h>
#include <stdint.h>

#include "cpu.h"
#include "ls2k500.h"
#include "ls2k500_irq.h"

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
    irq_handler_t isr;                  /* user defined interrupt handler */
    pwmtimer_callback_t callback;       /* callback when irq ocurred */
	osal_event_t p_event;               /* Send the RTOS event when irq ocurred */

    char dev_name[16];
    int  initialized;
    int  busy;
} PWM_t;

//-------------------------------------------------------------------------------------------------

#define PWM_INDEX(pwm) \
        ((VA_TO_PHYS(pwm->hwPWM) == PWM0_BASE)  ?  0 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM1_BASE)  ?  1 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM2_BASE)  ?  2 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM3_BASE)  ?  3 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM4_BASE)  ?  4 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM5_BASE)  ?  5 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM6_BASE)  ?  6 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM7_BASE)  ?  7 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM8_BASE)  ?  8 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM9_BASE)  ?  9 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM10_BASE) ? 10 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM11_BASE) ? 11 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM12_BASE) ? 12 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM13_BASE) ? 13 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM14_BASE) ? 14 : \
         (VA_TO_PHYS(pwm->hwPWM) == PWM15_BASE) ? 15 : -1)

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
            ls2k_interrupt_enable(pwm->irq_num);     /* Enable PWM Interrupt */
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
 * PWM timer interrupt, only timer mode
 */
static void ls2k_pwm_timer_common_isr(int vector, void *arg)
{
    PWM_t *pwm = (PWM_t *)arg;
    int stopme = 0;

    if (NULL == pwm)
    {
        return;
    }

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
     * Timer Event
     */
    if (pwm->p_event != NULL)
    {
        osal_event_send(pwm->p_event, PWM_TIMER_EVENT);
    }
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

        if (pwm->isr == NULL)
        {
            ls2k_install_irq_handler(pwm->irq_num, ls2k_pwm_timer_common_isr, (void *)pwm);
        }
        else
        {
            ls2k_install_irq_handler(pwm->irq_num, pwm->isr, (void *)pwm);
        }
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
	.irq_num  = EXTINTC1_PWM0_IRQ,
#else
	.irq_num  = INTC0_PWM0_3_IRQ,
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
	.irq_num  = EXTINTC1_PWM1_IRQ,
#else
	.irq_num  = INTC0_PWM0_3_IRQ,
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
	.irq_num  = EXTINTC1_PWM2_IRQ,
#else
	.irq_num  = INTC0_PWM0_3_IRQ,
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
	.irq_num  = EXTINTC1_PWM3_IRQ,
#else
	.irq_num  = INTC0_PWM0_3_IRQ,
#endif
	.dev_name = "pwm3",
	.initialized = 0,
};

const void *devPWM3 = (void *)&ls2k_PWM3;
#endif

/* PWM 4 */
#if BSP_USE_PWM4
static PWM_t ls2k_PWM4 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM4_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM4_IRQ,
#else
	.irq_num  = INTC0_PWM4_7_IRQ,
#endif
	.dev_name = "pwm4",
	.initialized = 0,
};

const void *devPWM4 = (void *)&ls2k_PWM4;
#endif

/* PWM 5 */
#if BSP_USE_PWM5
static PWM_t ls2k_PWM5 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM5_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM5_IRQ,
#else
	.irq_num  = INTC0_PWM4_7_IRQ,
#endif
	.dev_name = "pwm5",
	.initialized = 0,
};

const void *devPWM5 = (void *)&ls2k_PWM5;
#endif

/* PWM 6 */
#if BSP_USE_PWM6
static PWM_t ls2k_PWM6 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM6_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM6_IRQ,
#else
	.irq_num  = INTC0_PWM4_7_IRQ,
#endif
	.dev_name = "pwm6",
	.initialized = 0,
};

const void *devPWM6 = (void *)&ls2k_PWM6;
#endif

/* PWM 7 */
#if BSP_USE_PWM7
static PWM_t ls2k_PWM7 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM7_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM7_IRQ,
#else
	.irq_num  = INTC0_PWM4_7_IRQ,
#endif
	.dev_name = "pwm7",
	.initialized = 0,
};

const void *devPWM7 = (void *)&ls2k_PWM7;
#endif

/* PWM 8 */
#if BSP_USE_PWM8
static PWM_t ls2k_PWM8 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM8_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM8_IRQ,
#else
	.irq_num  = INTC0_PWM8_11_IRQ,
#endif
	.dev_name = "pwm8",
	.initialized = 0,
};

const void *devPWM8 = (void *)&ls2k_PWM8;
#endif

/* PWM 9 */
#if BSP_USE_PWM9
static PWM_t ls2k_PWM9 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM9_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM9_IRQ,
#else
	.irq_num  = INTC0_PWM8_11_IRQ,
#endif
	.dev_name = "pwm9",
	.initialized = 0,
};

const void *devPWM9 = (void *)&ls2k_PWM9;
#endif

/* PWM 10 */
#if BSP_USE_PWM10
static PWM_t ls2k_PWM10 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM10_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM10_IRQ,
#else
	.irq_num  = INTC0_PWM8_11_IRQ,
#endif
	.dev_name = "pwm10",
	.initialized = 0,
};

const void *devPWM10 = (void *)&ls2k_PWM10;
#endif

/* PWM 11 */
#if BSP_USE_PWM11
static PWM_t ls2k_PWM11 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM11_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM11_IRQ,
#else
	.irq_num  = INTC0_PWM8_11_IRQ,
#endif
	.dev_name = "pwm11",
	.initialized = 0,
};

const void *devPWM11 = (void *)&ls2k_PWM11;
#endif

/* PWM 12 */
#if BSP_USE_PWM12
static PWM_t ls2k_PWM12 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM12_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM12_IRQ,
#else
	.irq_num  = INTC0_PWM12_15_IRQ,
#endif
	.dev_name = "pwm12",
	.initialized = 0,
};

const void *devPWM12 = (void *)&ls2k_PWM12;
#endif

/* PWM 13 */
#if BSP_USE_PWM13
static PWM_t ls2k_PWM13 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM13_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM13_IRQ,
#else
	.irq_num  = INTC0_PWM12_15_IRQ,
#endif
	.dev_name = "pwm13",
	.initialized = 0,
};

const void *devPWM13 = (void *)&ls2k_PWM13;
#endif

/* PWM 14 */
#if BSP_USE_PWM14
static PWM_t ls2k_PWM14 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM14_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM14_IRQ,
#else
	.irq_num  = INTC0_PWM12_15_IRQ,
#endif
	.dev_name = "pwm14",
	.initialized = 0,
};

const void *devPWM14 = (void *)&ls2k_PWM14;
#endif

/* PWM 15 */
#if BSP_USE_PWM15
static PWM_t ls2k_PWM15 =
{
	.hwPWM    = (HW_PWM_t *)PHYS_TO_UNCACHED(PWM15_BASE),
#if USE_EXTINT
	.irq_num  = EXTINTC1_PWM15_IRQ,
#else
	.irq_num  = INTC0_PWM12_15_IRQ,
#endif
	.dev_name = "pwm15",
	.initialized = 0,
};

const void *devPWM15 = (void *)&ls2k_PWM15;
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
int ls2k_pwm_pulse_start(const void *pwm, pwm_cfg_t *cfg)
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

int ls2k_pwm_pulse_stop(const void *pwm)
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

int ls2k_pwm_timer_start(const void *pwm, pwm_cfg_t *cfg)
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

int ls2k_pwm_timer_stop(const void *pwm)
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
 * device name
 */
const char *ls2k_pwm_get_device_name(const void *pwm)
{
    return ((PWM_t *)pwm)->dev_name;
}

#endif // #if BSP_USE_PWM


