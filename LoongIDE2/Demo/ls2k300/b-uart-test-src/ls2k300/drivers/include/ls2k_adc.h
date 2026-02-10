/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_adc.h
 *
 * created: 2024-07-28
 *  author: Bian
 */

#ifndef _LS2K_ADC_H
#define _LS2K_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// ADC Channels
//-----------------------------------------------------------------------------

#define ADC_CH_1		        1           /* 通道 1 */
#define ADC_CH_2		        2           /* 通道 2 */
#define ADC_CH_3		        3		    /* 通道 3 */
#define ADC_CH_4		        4		    /* 通道 4 */
#define ADC_CH_5		        5		    /* 通道 5 */
#define ADC_CH_6		        6		    /* 通道 6 */
#define ADC_CH_7		        7		    /* 通道 7 */
#define ADC_CH_8		        8		    /* 通道 8 */

#define REGULAR_COUNT           8           /* 规则通道数 */

#define INJECT_COUNT            4           /* 注入通道数 */

//-----------------------------------------------------------------------------
// ADC 采样周期
//-----------------------------------------------------------------------------

#define ADC_SAMP_1P			    0		    /* 1 个周期 */
#define ADC_SAMP_2P			    1		    /* 2 个周期 */
#define ADC_SAMP_4P			    2		    /* 4 个周期 */
#define ADC_SAMP_8P			    3		    /* 8 个周期 */
#define ADC_SAMP_16P		    4		    /* 16 个周期 */
#define ADC_SAMP_32P		    5		    /* 32 个周期 */
#define ADC_SAMP_64P		    6		    /* 64 个周期 */
#define ADC_SAMP_128P		    7		    /* 128 个周期 */

//-----------------------------------------------------------------------------
// ADC Regular Channel
//-----------------------------------------------------------------------------

/*
 * 控制信号相位调节
 */
#define ADC_OPHASE_SAME		    0		    /* 与 ADCCLK 上升沿同时刻 */
#define ADC_OPHASE_BEFORE_1	    1		    /* 较 ADCCLK 上升沿前一pclk */
#define ADC_OPHASE_AFTER_1      2		    /* 较 ADCCLK 上升沿后一pclk */

/*
 * 规则通道外部触发源
 */
#define ADC_TRIG_SWATART	    0		    /* 不用外部事件启动转换 */
#define ADC_TRIG_ATIM_CC1       1		    /* 使用外部 ATIM_CC1 事件 */
#define ADC_TRIG_ATIM_CC2       2	      	/* 使用外部 ATIM_CC2 事件 */
#define ADC_TRIG_ATIM_CC3       3		    /* 使用外部 ATIM_CC3 事件 */
#define ADC_TRIG_GTIM_CC2       4		    /* 使用外部 GTIM_CC2 事件 */
#define ADC_TRIG_EXTI_11	    5		    /* 使用外部 EXTI 11 */

//---------------------------------------------------------

/**
 * 初始化结构, 同时设置规则通道
 */
typedef struct
{
	int OutPhaseSel;                        /* 控制信号相位调节 */
	int ClkDivider;                         /* 分频系数 */
	int DiffMode;                           /* 差分输入 */
	int ScanMode;                           /* 扫描模式 */
	int ContinuousMode;                     /* 连续转换 TODO */
	int TrigEdgeDown;                       /* 时钟触发沿: 1=下降沿 */
	int ExternalTrigSrc;                    /* 规则通道外部触发源 */
	int DataAlignLeft;                      /* 转换结果左对齐 */
	int RegularChannelCount;                /* 规则通道数 */
    int RegularChannels[REGULAR_COUNT];     /* 规则通道 */
    int SampleClocks[REGULAR_COUNT];        /* 采样周期 */
} ADC_Mode_t;

//-----------------------------------------------------------------------------
// ADC Inject Channel. runtime?
//-----------------------------------------------------------------------------

/*
 * 注入触发模式
 */
#define ADC_JTRIG_IMMEDIATE	    0		    /* 结束当前规则组转换并立即开始注入通道转换 */
#define ADC_JTRIG_END_RESET     1		    /* 结束当前规则组转换并在插入一拍ADC 复位控制信号后开始注入通道转换 */
#define ADC_JTRIG_END	        2		    /* 在当前规则组转换结束后开始注入通道转换 */

/*
 * 注入通道外部触发源
 */
#define ADC_JTRIG_JSWSTART	    0		    /* 不用外部事件启动转换 */
#define ADC_JTRIG_ATIM_TRGO	    1		    /* 使用外部 ATIM_TRGO 事件 */
#define ADC_JTRIG_ATIM_CC4	    2		    /* 使用外部 ATIM_CC4 事件 */
#define ADC_JTRIG_GTIM_TRGO	    3		    /* 使用外部 GTIM_TRGO 事件 */
#define ADC_JTRIG_GTIM_CC1	    4		    /* 使用外部 GTIM_CC1 事件 */
#define ADC_JTRIG_EXTI_15		5		    /* 使用外部 EXTI 15 */

/**
 * 设置注入通道
 */
typedef struct
{
    int JTrigAuto;                          /* 开启自动的注入通道组转换 */
    int JTrigMode;                          /* 注入触发模式 */
    int ExternalJTrigSrc;                   /* 注入通道外部触发源 */
	int InjectChannelCount;                 /* 注入通道数 */
    int InjectChannels[REGULAR_COUNT];      /* 注入通道 */
    int InjectOffsets[REGULAR_COUNT];       /* 注入通道偏移 */
    int SampleClocks[REGULAR_COUNT];        /* 采样周期 */
} ADC_Inject_t;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// IOCTL 命令参数
//-----------------------------------------------------------------------------

#define IOCTL_ADC_SET_MODE      0x01        /* 重新初始化ADC设备, 参数: ADC_Mode_t* */

#define IOCTL_ADC_CALIBRATE     0x02        /* 执行 ADC 校正, 参数: NULL */

#define IOCTL_ADC_SET_INJECT    0x03        /* 设置ADC 注入组, 参数: ADC_Inject_t* */

#define IOCTL_ADC_GET_JRESULT   0x04        /* 获取注入通道转换结果,
                                             * 参数: int *, 入口注入通道顺序号, 出口转换结果 */

//-----------------------------------------------------------------------------
// ADC devices
//-----------------------------------------------------------------------------

#if (BSP_USE_ADC)
extern const void *devADC;
#endif

//-----------------------------------------------------------------------------
// ADC driver implements
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *adc_drv_ops;

#define ls2k_adc_init(dev, arg)             adc_drv_ops->init_entry(dev, arg)
#define ls2k_adc_open(dev, arg)             adc_drv_ops->open_entry(dev, arg)
#define ls2k_adc_close(dev, arg)            adc_drv_ops->close_entry(dev, arg)
#define ls2k_adc_read(dev, buf, size, arg)  adc_drv_ops->read_entry(dev, buf, size, arg)
#define ls2k_adc_ioctl(dev, cmd, arg)       adc_drv_ops->ioctl_entry(dev, cmd, arg)

#else

/*
 * 初始化ADC
 * 参数:    dev     NULL 或者 devADC
 *          arg     类型 ADC_Mode_t*, 当该参数为NULL时, 设置为默认值
 *
 * 返回:    0=成功
 */
int ADC_initialize(const void *dev, void *arg);

/*
 * 打开ADC. 使能ADC, 如果配置为中断模式, 使能中断
 * 参数:    dev     NULL 或者 devADC
 *          arg     类型 NULL或者ADC_Mode_t*, 当不为NULL时, 用ADC_Mode_t*设置
 *
 * 返回:    0=成功
 */
int ADC_open(const void *dev, void *arg);

/*
 * 关闭ADC. 停止ADC, 如果配置为中断模式, 关闭中断
 * 参数:    dev     NULL 或者 devADC
 *          arg     NULL.
 *
 * 返回:    0=成功
 */
int ADC_close(const void *dev, void *arg);

/*
 * 从串口读数据(接收)
 * 参数:    dev     NULL 或者 devADC
 *          buf     类型 int *. 规则通道转换结果数组
 *          size    类型 int. 规则通道数组长度*sizeof(int)
 *          arg     类型 int. 规则通道序号:
 *                  如果规则通道数>1: 0=读取全部; 否则读取指定通道转换结果.
 *
 * 返回:    读取的字节数
 *
 */
int ADC_read(const void *dev, void *buf, int size, void *arg);

/*
 * 向ADC发送控制命令
 * 参数:    dev     NULL 或者 devADC
 *          cmd     IOCTL_ADC_xxx 定义
 *          arg
 *
 * 返回:    0=成功
 */
int ADC_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_adc_init(dev, arg)             ADC_initialize(dev, arg)
#define ls2k_adc_open(dev, arg)             ADC_open(dev, arg)
#define ls2k_adc_close(dev, arg)            ADC_close(dev, arg)
#define ls2k_adc_read(dev, buf, size, arg)  ADC_read(dev, buf, size, arg)
#define ls2k_adc_ioctl(dev, cmd, arg)       ADC_ioctl(de, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// ADC device name
//-----------------------------------------------------------------------------

const char *ls2k_adc_get_device_name(void);

//-----------------------------------------------------------------------------
// API after open()
//-----------------------------------------------------------------------------

extern int adc_read_1_fast(int channel, int samptime);

extern int adc_read_1(int channel);

#ifdef __cplusplus
}
#endif

#endif // _LS2K_ADC_H

