/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_pwm.h
 *
 * created: 2021/3/7
 *  author: Bian
 */

#ifndef _DRV_PWM_H
#define _DRV_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO control command, args is as pwm_cfg_t *
 */
#define IOCTL_PWM_START         0x01
#define IOCTL_PWM_STOP          0x02

void rt_ls2k_pwm_install(void);

extern const char *ls1x_pwm_get_device_name(void *pwm);

#ifdef __cplusplus
}
#endif

#endif // _DRV_PWM_H

