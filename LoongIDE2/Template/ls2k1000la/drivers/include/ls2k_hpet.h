/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_hpet.h
 *
 * created: 2022-10-13
 *  author: Bian
 */

#ifndef _LS2K_HPET_H
#define _LS2K_HPET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls2k_drv_io.h"

//-----------------------------------------------------------------------------
// HPET 设备
//-----------------------------------------------------------------------------

#if BSP_USE_HPET
extern const void *devHPET0;
#endif

//-----------------------------------------------------------------------------
// 每个 HPET 有三个定时器子设备
//-----------------------------------------------------------------------------

#define HPET_TIMER0             0x501
#define HPET_TIMER1             0x502
#define HPET_TIMER2             0x503

//-----------------------------------------------------------------------------
// HPET 工作模式
//-----------------------------------------------------------------------------

#define HPET_MODE_SINGLE        0x01    // 产生单次中断
#define HPET_MODE_CYCLE         0x02    // 产生周期中断

//-----------------------------------------------------------------------------
// 定时器中断回调函数
//-----------------------------------------------------------------------------

/*
 * 参数:    hpet    HPET 设备
 *          timer   HPET 定时器子设备 TIMERn
 *          stop    如果给*stop 赋非零值, 该定时器将停止不再工作, 否则定时器会自动重新
 *                  载入interval_ns值, 等待下一次定时器计时阀值产生中断.
 */
typedef void (*hpetimer_callback_t)(const void *hpet, int timer, int *stop);

//-----------------------------------------------------------------------------
// HPET parameter
//-----------------------------------------------------------------------------

typedef struct hpet_cfg
{
    int timer;                      /* HPET 定时器子设备: HPET_TIMERn */

    /*
     * This parameter's unit is nanosecond(ns). After interrupt ocurred,
     * this value will auto loaded for next match interrupt.
     */
    int interval_ns;                /* 纳秒为单位进行定时 */
    int work_mode;                  /* 单次/周期, 边缘/电平 */

    irq_handler_t       isr;        /* 优先级: 高. User defined match-isr */
    hpetimer_callback_t cb;         /* 优先级: 中. called by match-isr */
    void *event;                    /* 优先级: 低. RTOS event created by user */
    
} hpet_cfg_t;

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HPET driver operators
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)

extern const driver_ops_t *hpet_drv_ops;

#define ls2k_hpet_init(hpet, arg)       hpet_drv_ops->init_entry(hpet, arg)
#define ls2k_hpet_open(hpet, arg)       hpet_drv_ops->open_entry(hpet, arg)
#define ls2k_hpet_close(hpet, arg)      hpet_drv_ops->close_entry(hpet, arg)

#else

/*
 * HPET初始化
 * 参数:    dev     NULL/HPET 设备
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int HPET_initialize(const void *dev, void *arg);

/*
 * 打开HPET定时器
 * 参数:    dev     NULL/HPET 设备
 *          arg     类型: hpet_cfg_t *, 用于设置HPET定时器子设备的工作模式并启动
 *
 * 返回:    0=成功
 *
 * 说明:    必须设置参数hpet_cfg_t的interval_ns值, 当HPET计时到达interval_ns阀值时, 将触发HPET定时中断,
 *          这时中断响应:
 *          1. HPET 设备使用共享中断;
 *          2. 如果cb != NULL, 该中断调用cb 回调函数让用户作出定时响应;
 *          3. 如果cb == NULL, 但有event参数, HPET中断将发出HPET_TIMER_EVENT事件.
 *
 *          interval_ns 是否产生连续中断, 有work_mode设置.
 *
 */
int HPET_open(const void *dev, void *arg);

/*
 * 关闭HPET定时器
 * 参数:    dev     NULL/HPET 设备
 *          arg     HPET 定时器子设备: HPET_TIMERn
 *
 * 返回:    0=成功
 */
int HPET_close(const void *dev, void *arg);

#define ls2k_hpet_init(hpet, arg)       HPET_initialize(hpet, arg)
#define ls2k_hpet_open(hpet, arg)       HPET_open(hpet, arg)
#define ls2k_hpet_close(hpet, arg)      HPET_close(hpet, arg)
#endif

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

/*
 * 启动HPET定时器
 * 参数:    dev     NULL/devHPET
 *          timer   HPET_TIMER0~HPET_TIMER2
 *          cfg     类型: hpet_cfg_t *
 *
 * 返回:    0=成功
 */
int ls2k_hpet_timer_start(const void *hpet, int timer, hpet_cfg_t *cfg);

/*
 * 停止HPET定时器
 * 参数:    dev     NULL/devHPET
 *          timer   HPET_TIMER0~HPET_TIMER2
 *
 * 返回:    0=成功
 */
int ls2k_hpet_timer_stop(const void *hpet, int timer);


#ifdef __cplusplus
}
#endif

#endif // _LS2K_HPET_H

