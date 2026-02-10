/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_rtc.h
 *
 * created: 2022-12-13
 *  author: Bian
 */

#ifndef _LS2K_RTC_H
#define _LS2K_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

#include "ls2k_drv_io.h"

//-----------------------------------------------------------------------------
// RTC 设备
//-----------------------------------------------------------------------------

#if BSP_USE_RTC
extern const void *devRTC;
#endif

//-----------------------------------------------------------------------------
// 定时器类别
//-----------------------------------------------------------------------------

#define LS2K_RTC                0x0500
#define LS2K_TOY                0x0A00

//-----------------------------------------------------------------------------
// 定时器子设备
//-----------------------------------------------------------------------------

#define DEVICE_RTCMATCH0       (LS2K_RTC | 1)
#define DEVICE_RTCMATCH1       (LS2K_RTC | 2)
#define DEVICE_RTCMATCH2       (LS2K_RTC | 3)

#define DEVICE_TOYMATCH0       (LS2K_TOY | 1)
#define DEVICE_TOYMATCH1       (LS2K_TOY | 2)
#define DEVICE_TOYMATCH2       (LS2K_TOY | 3)

//-----------------------------------------------------------------------------
// 定时器中断回调函数
//-----------------------------------------------------------------------------

/*
 * 参数:    device  定时器子设备产生的中断
 *          match   当前RTCMATCH或者TOYMATCH的寄存器值
 *          stop    如果给*stop 赋非零值, 该定时器将停止不再工作, 否则定时器会自动重新
 *                  载入interval_ms值, 等待下一次定时器计时阀值产生中断.
 */
typedef void (*rtctimer_callback_t)(int device, unsigned int match, int *stop);

//-----------------------------------------------------------------------------
// RTC parameter
//-----------------------------------------------------------------------------

typedef struct rtc_cfg
{
    /*
     * This parameter's unit is millisecond(ms). After interrupt ocurred,
     * this value will auto loaded for next match interrupt.
     * if trig_datetime is NULL, toymatch use this parameter.
     *
     */
    int        interval_ms;

    /*
     * This parameter is used by toymatch. When toymatch arrives this datetime
     * toymatch interrupt will be triggered. This interrupt ocurred only once.
     *
     * if This parameter is NULL, use interval_ms as irq trigger interval
     *
     */
    struct tm *trig_datetime;

    irq_handler_t       isr;            /* 优先级: 高. User defined match-isr */
    rtctimer_callback_t cb;             /* 优先级: 中. called by match-isr */
    void               *event;          /* 优先级: 低. RTOS event created by user */

} rtc_cfg_t;

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_SET_SYS_DATETIME      0x8001      // struct tm *
#define IOCTL_GET_SYS_DATETIME      0x8002      // struct tm *

/**
 * control rtc
 */
#define IOCTL_RTC_SET_TRIM          0x0811      // unsigned int *
#define IOCTL_RTC_GET_TRIM          0x0812      // unsigned int *

#define IOCTL_RTCMATCH_START        0x0813      // DEVICE_RTCMATCHx & rtc_cfg_t *
#define IOCTL_RTCMATCH_STOP         0x0814      // DEVICE_RTCMATCHx

/**
 * control toy
 */
#define IOCTL_TOY_SET_TRIM          0x0821      // unsigned int *
#define IOCTL_TOY_GET_TRIM          0x0822      // unsigned int *

#define IOCTL_TOYMATCH_START        0x0823      // DEVICE_TOYMATCHx & rtc_cfg_t *
#define IOCTL_TOYMATCH_STOP         0x0824      // DEVICE_TOYMATCHx

//-----------------------------------------------------------------------------
// RTC driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *rtc_drv_ops;

#define ls2k_rtc_init(rtc, arg)             rtc_drv_ops->init_entry(rtc, arg)
#define ls2k_rtc_open(rtc, arg)             rtc_drv_ops->open_entry(rtc, arg)
#define ls2k_rtc_close(rtc, arg)            rtc_drv_ops->close_entry(rtc, arg)
#define ls2k_rtc_read(rtc, buf, size, arg)  rtc_drv_ops->read_entry(rtc, buf, size, arg)
#define ls2k_rtc_write(rtc, buf, size, arg) rtc_drv_ops->write_entry(rtc, buf, size, arg)
#define ls2k_rtc_ioctl(rtc, cmd, arg)       rtc_drv_ops->ioctl_entry(rtc, cmd, arg)

#else

/*
 * RTC初始化
 * 参数:    dev     总是 NULL
 *          arg     类型: struct tm *. 如果该参数不是 NULL, 其值用于初始化RTC系统时间.
 *
 * 返回:    0=成功
 */
int RTC_initialize(const void *dev, void *arg);

/*
 * 打开RTC定时器
 * 参数:    dev     要打开的定时器子设备 DEVICE_XXX
 *          arg     类型: rtc_cfg_t *, 用于设置RTC子设备的工作模式并启动
 *
 * 返回:    0=成功
 *
 * 说明:    如果使用的是RTC子设备, 必须设置参数rtc_cfg_t的interval_ms值, 当RTC计时到达interval_ms阀值时,
 *          将触发RTC定时中断, 这时中断响应:
 *          1. 如果传入参数有用户自定义中断 isr(!=NULL), 则响应isr;
 *          2. 如果自定义中断 isr=NULL, 使用RTC默认中断, 该中断调用cb 回调函数让用户作出定时响应;
 *          3. 如果自定义中断 isr=NULL且cb=NULL, 如果有event参数, RTC默认中断将发出RTC_TIMER_EVENT事件.
 *
 *          如果使用的是TOY子设备, 并且设置有interval_ms参数(>1000), 用法和使用RTC子设备一样;
 *          当interval_ms==0且trig_datetime!=NULL时, 表示TOY子设备将在计时到达这个未来时间点时触发中断,
 *          中断处理流程和上面一致.
 *          使用trig_datetime触发的中断仅发生一次.
 *
 *          interval_ms用于间隔产生中断并且一直产生; trig_datetime用于到时产生中断仅产生一次.
 *
 */
int RTC_open(const void *dev, void *arg);

/*
 * 关闭RTC定时器
 * 参数:    dev     要关闭的定时器子设备 DEVICE_XXX
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int RTC_close(const void *dev, void *arg);

/*
 * 读取当前RTC时钟
 * 参数:    dev     NULL
 *          buf     类型: struct tm *, 用于存放读取的时钟值
 *          size    类型: int, 大小=sizeof(struct tm)
 *          arg     NULL
 *
 * 返回:    读取的字节数, 正常为sizeof(struct tm)
 */
int RTC_read(const void *dev, void *buf, int size, void *arg);

/*
 * 设置RTC时钟
 * 参数:    dev     NULL
 *          buf     类型: struct tm *, 用于存放待写入的时钟值
 *          size    类型: int, 大小=sizeof(struct tm)
 *          arg     NULL
 *
 * 返回:    写入的字节数, 正常为sizeof(struct tm)
 */
int RTC_write(const void *dev, void *buf, int size, void *arg);

/*
 * 控制RTC时钟设备
 * 参数:    dev     NULL or DEVICE_XXX
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SET_SYS_DATETIME          |   类型: truct tm *
 *                                          |   用途: 设置RTC系统时间值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GET_SYS_DATETIME          |   类型: struct tm *
 *                                          |   用途: 获取当前RTC系统时间值
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_SET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 设置RTC的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_GET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 获取RTC的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_START            |   类型: rtc_cfg_t *, 启动RTC定时器
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_STOP             |   类型: NULL, 停止RTC定时器
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_SET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 设置TOY的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_GET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 获取TOY的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_START            |   类型: rtc_cfg_t *, 启动TOY定时器
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_STOP             |   类型: NULL, 停止TOY定时器
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *
 *
 * 返回:    0=成功
 */
int RTC_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_rtc_init(rtc, arg)             RTC_initialize(rtc, arg)
#define ls2k_rtc_open(rtc, arg)             RTC_open(rtc, arg)
#define ls2k_rtc_close(rtc, arg)            RTC_close(rtc, arg)
#define ls2k_rtc_read(rtc, buf, size, arg)  RTC_read(rtc, buf, size, arg)
#define ls2k_rtc_write(rtc, buf, size, arg) RTC_write(rtc, buf, size, arg)
#define ls2k_rtc_ioctl(rtc, cmd, arg)       RTC_ioctl(rtc, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

/*
 * 设置RTC时钟值, 参见ls2k_rtc_read()
 */
int ls2k_rtc_set_datetime(struct tm *dt);
/*
 * 获取当前RTC时间, 参见ls2k_rtc_write()
 */
int ls2k_rtc_get_datetime(struct tm *dt);
/*
 * 获取当前RTC的秒数
 */
time_t ls2k_rtc_get_seconds(void);
/*
 * 获取当前RTC的 struct timeval
 */
int ls2k_rtc_get_timeval(struct timeval *tv);

/*
 * 定时器子设备
 */
/*
 * 开启定时器, 参见ls2k_rtc_open()
 */
int ls2k_rtc_timer_start(unsigned device, rtc_cfg_t *cfg);
/*
 * 关闭定时器, 参见ls2k_rtc_close()
 */
int ls2k_rtc_timer_stop(unsigned device);

/*
 * LS2K toymatch 日期格式转换
 */
void ls2k_tm_to_toymatch(struct tm *dt, unsigned int *match);
void ls2k_toymatch_to_tm(struct tm *dt, unsigned int match);

unsigned int ls2k_seconds_to_toymatch(unsigned int seconds);
unsigned int ls2k_toymatch_to_seconds(unsigned int match);

/*
 * struct tm 日期格式转换, +1900/-1900
 */
void normalize_tm(struct tm *tm, bool tm_format);


#ifdef __cplusplus
}
#endif

#endif // _LS2K_RTC_H

