/*
 * pwm_timer_test.c
 *
 * created: 2024-08-04
 *  author: 
 */

#include "bsp.h"

#if BSP_USE_PWM

#include <stdlib.h>

#include "ls2k_pwm.h"

/*
 * PWM 测试
 */
void PWM3_TIM_callback(void *pwm, int *stopit)
{
    printk("3.");
//    *stopit = 1;
}

void PWM2_TIM_callback(void *pwm, int *stopit)
{
    printk("2.");
//    *stopit = 1;
}

void pwm_timer_test(void)
{
    pwm_cfg_t cfg;

    memset((void *)&cfg, 0, sizeof(pwm_cfg_t));  // 这个结构必须清零

    /*
     * PWM2 as timer
     */
    cfg.mode = PWM_CONTINUE_TIMER;
    cfg.hi_ns = 200*1000*1000;
    cfg.cb = PWM2_TIM_callback;          //定时器中断回调函数

    ls2k_pwm_timer_start(devPWM2, &cfg);

  #if 1
    /*
     * PWM3 as timer
     */
    cfg.mode = PWM_CONTINUE_TIMER;
    cfg.hi_ns = 500*1000*1000;
    cfg.cb = PWM3_TIM_callback;          //定时器中断回调函数

    ls2k_pwm_timer_start(devPWM3, &cfg);
  #endif
}

#endif // #if BSP_USE_PWM

/*
 * @@ END
 */



