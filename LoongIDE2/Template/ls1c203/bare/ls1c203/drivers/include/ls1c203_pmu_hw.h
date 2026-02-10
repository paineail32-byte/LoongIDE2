/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls1c203_pmu_hw.h
 *
 * created: 2025-05-20
 *  author: Bian
 */

#ifndef _LS1C_PMU_HW_H
#define _LS1C_PMU_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// PMU 设备
//-------------------------------------------------------------------------------------------------

#define PMU_BASE		0x0000c000

/*
 * PMU 控制器
 */
typedef struct
{
    volatile unsigned int ChipCtrl;             // 0x00 全局配置
    volatile unsigned int CmdSts;               // 0x04 命令与状态
    volatile unsigned int Count;                // 0x08 时间计数器
    volatile unsigned int Compare;              // 0x0c 唤醒时间配置
    volatile unsigned int RstCtrl;              // 0x10 复位控制寄存器
    volatile unsigned int ClkCtrl;              // 0x14 时钟控制寄存器
    volatile unsigned int rsv1[6];
    volatile unsigned int WdtCfg;               // 0x30 看门狗配置
    volatile unsigned int WdtFeed;              // 0x34 看门狗重置
    volatile unsigned int PowerCfg;             // 0x38 电源配置
    volatile unsigned int CommandW;             // 0x3c 状态清除寄存器
    volatile unsigned int rsv2[8];
    volatile unsigned int UserDat0;             // 0x60 用户数据0
    volatile unsigned int UserDat1;             // 0x64 用户数据1
    volatile unsigned int UserDat2;             // 0x68 用户数据2
    volatile unsigned int UserDat3;             // 0x6c 用户数据3
} HW_PMU_t;

/**
 * 芯片全局配置 - ChipCtrl
 */
#define CHIPCTRL_INPUT_HOLD     bit(10)         // 输入保持
#define CHIPCTRL_32K_SEL        bit(5)          // 32K时钟选择. 0=内部时钟; 1=外部时钟
#define CHIPCTRL_32K_TRIM_MASK  0x1F            // bit[4:0] 内部32K OSC Trimming值

/**
 * 命令与状态 - CmdSts
 */
#define CMDSR_32K_FAIL         bit(29)          // 32K外部时钟失效. 1=失效
#define CMDSR_32K_SEL          bit(28)          // 32K时钟选择. 1=外部时钟
#define CMDSR_RSTSRC_MASK      (3<<26)          // bit[27:26] 复位来源
#define CMDSR_RSTSRC_OUTER     0                // 外部复位
#define CMDSR_RSTSRC_WD1       (1<<26)          // 看门狗复位, 01/10每次复位时切换
#define CMDSR_RSTSRC_WD2       (2<<26)
#define CMDSR_RSTSRC_WAKE      (3<<26)          // 休眠唤醒
#define CMDSR_EXTINT_EN        bit(25)          // 外部中断使能

#define CMDSR_INTSRC_MASK      0x1FF0000        // bit[24:16] 中断状态, 往CommandW对应位写1可清除中断状态
#define CMDSR_INTSRC_EXTINT    0x1000000        // [8]: e_ExtInt
#define CMDSR_INTSRC_RTC       0x0100000        // [4]: e_RTC
#define CMDSR_INTSRC_8MFAIL    0x0080000        // [3]: e_C8MFail
#define CMDSR_INTSRC_32KFAIL   0x0040000        // [2]: e_C32KFail
#define CMDSR_INTSRC_BATFAIL   0x0020000        // [1]: e_BatFail
#define CMDSR_INTSRC_WAKE      0x0010000        // [0]: e_Wake

#define CMDSR_INTEN_MASK       0xFF00           // bit[15:8] 中断使能, 每一位对应一个中断源
#define CMDSR_INTEN_RTC        0x1000           // [4]: e_RTC
#define CMDSR_INTEN_8MFAIL     0x0800           // [3]: e_C8MFail
#define CMDSR_INTEN_32KFAIL    0x0400           // [2]: e_C32KFail
#define CMDSR_INTEN_BATFAIL    0x0200           // [1]: e_BatFail
#define CMDSR_INTEN_WAKE       0x0100           // [0]: e_Wake

#define CMDSR_WAKE_EN          bit(7)           // 定时唤醒使能
#define CMDSR_SLEEP_EN         bit(0)           // 进入休眠状态

/**
 * 时间计数器 - Count
 */
#define RTC_COUNT_MASK          0xFFFFF         // bit[19:0] RTC 时间计数器, 每1/256秒加1

/**
 * 唤醒时间配置 - Compare
 */
#define RTC_WAKECMP_MASK        0xFFFFF         // bit[19:0] WakeCmp 唤醒时间配置.
                                                // 当该值与Count相等且WakeEn=1时产生唤醒事件

/**
 * 软件复位 - RstCtrl
 */
#define RSTCTRL_GPIOB           bit(18)         // gpiob 复位
#define RSTCTRL_GPIOA           bit(17)         // gpioa 复位
#define RSTCTRL_AFIO            bit(16)         // afio 复位

/**
 * 时钟关断 - ClkCtrl
 */
#define CLKCTRL_RTC             bit(20)         // rtc 时钟
#define CLKCTRL_WDT             bit(19)         // wdt 时钟
#define CLKCTRL_GPIOB           bit(18)         // gpiob 时钟
#define CLKCTRL_GPIOA           bit(17)         // gpioa 时钟
#define CLKCTRL_AFIO            bit(16)         // afio 时钟

/**
 * 看门狗配置寄存器 - WdtCfg
 *
 * wdtcfg_hi = ~wdtcfg_lo
 *
 */
#define WDT_HI_MASK             0xFFFF0000      // 看门狗配置高位
#define WDT_LO_MASK             0x0000FFFF      // 看门狗配置低位

/**
 * 看门狗重置寄存器 - WdtFeed
 *
 * 写入 0xa55a55aa 喂狗
 *
 */
#define WDTFEED_FOOD            0xa55a55aa

/**
 * 电源配置 - PowerCfg
 */
#define POWERCFG_VBGTRIM_MASK   (0x7<<23)       // bit[27:23] 电压调节参数, 硬件自动配置, 不建议更改

/**
 * 命令写端口 - CommandW
 */
#define CMDW_INTCLR_MASK        0x1F0000        // bit[20:16] 中断状态清除
#define CMDW_INTCLR_RTC         bit(20)         // [4]: e_RTC
#define CMDW_INTCLR_8MFAIL      bit(19)         // [3]: e_C8MFail
#define CMDW_INTCLR_32KFAIL     bit(18)         // [2]: e_C32KFail
#define CMDW_INTCLR_BATFAIL     bit(17)         // [1]: e_BatFail
#define CMDW_INTCLR_WAKE        bit(16)         // [0]: e_Wake

#define CMDW_SLEEP              bit(0)          // 进入休眠状态, SLeepEn=1


#ifdef __cplusplus
}
#endif

#endif // _LS1C_PMU_HW_H

