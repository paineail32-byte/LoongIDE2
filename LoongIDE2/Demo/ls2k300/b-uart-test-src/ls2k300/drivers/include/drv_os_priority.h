/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_os_priority.h
 *
 * created: 2021/1/2
 *  author: Bian
 *
 */

#ifndef _DRV_OS_PRIORITY_H
#define _DRV_OS_PRIORITY_H

//-----------------------------------------------------------------------------
// LS2K 驱动程序使用的事件
//-----------------------------------------------------------------------------

#define CAN_RX_EVENT        0x0001      /* CAN 接收事件 */
#define CAN_TX_EVENT        0x0002      /* CAN 发送事件 */

#define GMAC_RX_EVENT       0x0004      /* GMAC 接收事件 */
#define GMAC_TX_EVENT       0x0008      /* GMAC 发送事件 */

#define PWM_TIMER_EVENT     0x0010      /* PWM Timer 事件 */
#define RTC_TIMER_EVENT     0x0020      /* RTC Timer 事件 */
#define HPET_TIMER_EVENT    0x0040      /* HPET Timer 事件 */

#define AC97_DMA1_EVENT     0x0100      /* AC97 DMA1 tx 事件 */
#define AC97_DMA2_EVENT     0x0200      /* AC97 DMA2 rx 事件 */

#define TOUCH_CLICK_EVENT   0x1000      /* 触摸事件 */

#endif // _DRV_OS_PRIORITY_H

//-----------------------------------------------------------------------------
// LS2K BSP 任务优先级
//-----------------------------------------------------------------------------

/**
 * RT-Thread 优先级说明：
 *     1. RT_THREAD_PRIORITY_MAX == 32
 *     2. 允许同优先级、时间片
 *     3. 数值越小，优先级越高
 */
/**
 * UCOSII 优先级说明：
 *     1. OS_CFG_PRIO_MAX == 64
 *     2. 允许同优先级
 *     3. 数值越小，优先级越高
 */
/**
 * FreeRTOS 优先级说明：
 *     1. CONFIG_MAX_PRIORITIES == 31
 *     2. 允许同优先级、时间片
 *     3. 数值越大，优先级越高
 *     4. 使用 osal_task_create()创建时, 执行 configMAX_PRIORITIES - priority.
 */

/**
 * 使用 osal 创建任务时：优先级数值越小，优先级越高
 */

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
