/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_pwm_hw.h
 *
 * created: 2022-03-10
 *  author: Bian
 */

#ifndef _LS2K_PWM_HW_H
#define _LS2K_PWM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// PWM 设备
//-------------------------------------------------------------------------------------------------

#define PWM0_BASE       0x1ff5c000
#define PWM1_BASE       0x1ff5c010
#define PWM2_BASE       0x1ff5c020
#define PWM3_BASE       0x1ff5c030
#define PWM4_BASE       0x1ff5c040
#define PWM5_BASE       0x1ff5c050
#define PWM6_BASE       0x1ff5c060
#define PWM7_BASE       0x1ff5c070
#define PWM8_BASE       0x1ff5c080
#define PWM9_BASE       0x1ff5c090
#define PWM10_BASE      0x1ff5c0A0
#define PWM11_BASE      0x1ff5c0B0
#define PWM12_BASE      0x1ff5c0C0
#define PWM13_BASE      0x1ff5c0D0
#define PWM14_BASE      0x1ff5c0E0
#define PWM15_BASE      0x1ff5c0F0

/*
 * PWM 控制器
 */
typedef struct
{
	volatile unsigned int counter;      /* 0x00 主计数器 */
	volatile unsigned int lowlevel;     /* 0x04 低脉冲缓冲寄存器*/
	volatile unsigned int fullpulse;    /* 0x08 脉冲周期缓冲寄存器 */
	volatile unsigned int ctrl;         /* 0x0C 控制寄存器 */
} HW_PWM_t;

/*
 * 控制寄存器
 */
#define PWM_CTRL_EN         (1<<0)      /* 计数器使能位, 1=CNTR用来计数; 0=CNTR停止计数 */
#define PWM_CTRL_OEN        (1<<3)      /* 脉冲输出使能控制位,低有效. 0=脉冲输出使能, 1=脉冲输出屏蔽 */
#define PWM_CTRL_SINGLE     (1<<4)      /* 单脉冲控制位, 1=脉冲仅产生一次, 0=脉冲持续产生 */
#define PWM_CTRL_IEN        (1<<5)      /* 中断使能位, 1=当full_pulse到1时送中断, 0=不产生中断 */
#define PWM_CTRL_IFLAG      (1<<6)      /* 中断位, 1=有中断产生, 0=没有中断. 写入1: 清中断 */
#define PWM_CTRL_RESET      (1<<7)      /* 1=计数器重置(从buffer读,输出低电平), 0=计数器正常工作 */
#define PWM_CTRL_CAPTE      (1<<8)      /* 1=测量脉冲使能 */
#define PWM_CTRL_INVERT     (1<<9)      /* 1=脉冲在输出去发生信号翻转(周期以高电平开始),0=使脉冲保持原始输出(周期以低电平开始) */
#define PWM_CTRL_DZONE      (1<<10)     /* 1= 该计数模块需要启用防死区功能, 0=该模块无需防死区功能 */

#ifdef __cplusplus
}
#endif

#endif // _LS2K_PWM_HW_H

