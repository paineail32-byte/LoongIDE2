/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_pwm.c
 *
 * created: 2022/6/7
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_PWM

#include <rtthread.h>
#include <rthw.h>

#include "ls2k_pwm.h"

#include "drv_pwm.h"

//-------------------------------------------------------------------------------------------------
// RTThread device for LS2K
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue PWM device to RTThread.
 */
static rt_err_t rt_pwm_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_pwm_init(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    return RT_EOK;
}

static rt_err_t rt_pwm_control(struct rt_device *dev,
                               int               cmd,
                               void             *args)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);
    RT_ASSERT(args != RT_NULL);
    
    pwm_cfg_t *cfg = (pwm_cfg_t *)args;
    
    switch (cmd)
    {
        case IOCTL_PWM_START:
            if ((cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_CONTINUE_PULSE))
            {
                if (ls2k_pwm_pulse_start(dev->user_data, (pwm_cfg_t *)args) != 0)
                    return RT_EIO;
            }
            else if ((cfg->mode == PWM_SINGLE_TIMER) || (cfg->mode == PWM_CONTINUE_TIMER))
            {
                if (ls2k_pwm_timer_start(dev->user_data, (pwm_cfg_t *)args) != 0)
                    return RT_EIO;
            }
            else
                return RT_EINVAL;

            break;

        case IOCTL_PWM_STOP:
            if ((cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_CONTINUE_PULSE))
            {
                if (ls2k_pwm_pulse_stop(dev->user_data) != 0)
                    return RT_EIO;
            }
            else if ((cfg->mode == PWM_SINGLE_TIMER) || (cfg->mode == PWM_CONTINUE_TIMER))
            {
                if (ls2k_pwm_timer_stop(dev->user_data) != 0)
                    return RT_EIO;
            }
            else
                return RT_EINVAL;

            break;

        default:
            return RT_EINVAL;
    }

    return RT_EOK;
}

/******************************************************************************
 * PWM devices
 */
#if BSP_USE_PWM0
static struct rt_device rt_ls2k_pwm0;
#endif
#if BSP_USE_PWM1
static struct rt_device rt_ls2k_pwm1;
#endif
#if BSP_USE_PWM2
static struct rt_device rt_ls2k_pwm2;
#endif
#if BSP_USE_PWM3
static struct rt_device rt_ls2k_pwm3;
#endif
#if BSP_USE_PWM4
static struct rt_device rt_ls2k_pwm4;
#endif
#if BSP_USE_PWM5
static struct rt_device rt_ls2k_pwm5;
#endif
#if BSP_USE_PWM6
static struct rt_device rt_ls2k_pwm6;
#endif
#if BSP_USE_PWM7
static struct rt_device rt_ls2k_pwm7;
#endif
#if BSP_USE_PWM8
static struct rt_device rt_ls2k_pwm8;
#endif
#if BSP_USE_PWM9
static struct rt_device rt_ls2k_pwm9;
#endif
#if BSP_USE_PWM10
static struct rt_device rt_ls2k_pwm10;
#endif
#if BSP_USE_PWM11
static struct rt_device rt_ls2k_pwm11;
#endif
#if BSP_USE_PWM12
static struct rt_device rt_ls2k_pwm12;
#endif
#if BSP_USE_PWM13
static struct rt_device rt_ls2k_pwm13;
#endif
#if BSP_USE_PWM14
static struct rt_device rt_ls2k_pwm14;
#endif
#if BSP_USE_PWM15
static struct rt_device rt_ls2k_pwm15;
#endif

/******************************************************************************
 * PWM register
 */
static rt_err_t rt_ls2k_pwm_register(struct rt_device *dev, const void *pwm, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(pwm != RT_NULL);

    dev->type        = RT_Device_Class_Miscellaneous;  // RT_Device_Class_PWM
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = rt_pwm_init;
    dev->open        = RT_NULL;
    dev->close       = RT_NULL;
    dev->read        = RT_NULL;
    dev->write       = RT_NULL;
    dev->control     = rt_pwm_control;

    dev->user_data   = (void *)pwm;

    /* register a can device
     */
    return rt_device_register(dev, ls2k_pwm_get_device_name(pwm), flag);
}

//-----------------------------------------------------------------------------
// Register Loongson PWM devices
//-----------------------------------------------------------------------------

void rt_ls2k_pwm_install(void)
{
#if BSP_USE_PWM0
    rt_ls2k_pwm_register(&rt_ls2k_pwm0, devPWM0, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM1
    rt_ls2k_pwm_register(&rt_ls2k_pwm1, devPWM1, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM2
    rt_ls2k_pwm_register(&rt_ls2k_pwm2, devPWM2, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM3
    rt_ls2k_pwm_register(&rt_ls2k_pwm3, devPWM3, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM4
    rt_ls2k_pwm_register(&rt_ls2k_pwm4, devPWM4, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM5
    rt_ls2k_pwm_register(&rt_ls2k_pwm5, devPWM5, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM6
    rt_ls2k_pwm_register(&rt_ls2k_pwm6, devPWM6, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM7
    rt_ls2k_pwm_register(&rt_ls2k_pwm7, devPWM7, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM8
    rt_ls2k_pwm_register(&rt_ls2k_pwm8, devPWM8, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM9
    rt_ls2k_pwm_register(&rt_ls2k_pwm9, devPWM9, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM10
    rt_ls2k_pwm_register(&rt_ls2k_pwm10, devPWM10, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM11
    rt_ls2k_pwm_register(&rt_ls2k_pwm11, devPWM11, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM12
    rt_ls2k_pwm_register(&rt_ls2k_pwm12, devPWM12, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM13
    rt_ls2k_pwm_register(&rt_ls2k_pwm13, devPWM13, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM14
    rt_ls2k_pwm_register(&rt_ls2k_pwm14, devPWM14, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_PWM15
    rt_ls2k_pwm_register(&rt_ls2k_pwm15, devPWM15, RT_DEVICE_FLAG_RDWR);
#endif
}

#endif

