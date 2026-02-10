/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_hpet_hw.h
 *
 * created: 2022-10-13
 *  author: Bian
 */

#ifndef _LS2K_HPET_HW_H
#define _LS2K_HPET_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * HPET 控制器
 */
typedef struct
{
	volatile unsigned long cap_id;      /* 0x000 RO General Capabilities and ID Register */
	volatile unsigned long rsv1;
    volatile unsigned long config;      /* 0x010 RW General Configuration Register */
    volatile unsigned long rsv2;
	volatile unsigned long isr;         /* 0x020 RW General Interrupt Status Register */
	volatile unsigned long rsv3[25];
	volatile unsigned long counter;     /* 0x0F0 RW Main Counter Value Register */
	volatile unsigned long rsv4;
	volatile unsigned long tm0cfg;      /* 0x100 RW Timer 0 Configuration and Capability Register */
    volatile unsigned long tm0cmp;      /* 0x108 RW Timer 0 Comparator Value Register */
	volatile unsigned long rsv5[2];
	volatile unsigned long tm1cfg;      /* 0x120 RW Timer 1 Configuration and Capability Register */
	volatile unsigned long tm1cmp;      /* 0x128 RW Timer 1 Comparator Value Register */
	volatile unsigned long rsv6[2];
	volatile unsigned long tm2cfg;      /* 0x140 RW Timer 2 Configuration and Capability Register */
	volatile unsigned long tm2cmp;      /* 0x148 RW Timer 2 Comparator Value Register */
} HW_HPET_t;

/*
 * General Capabilities and ID Register
 */
#define HPET_CNT_CLK_PERIOD_MASK    (0xFFFFFFFFul<<32)  /* bit[63:32] RO 这个域标示了主计时器的计时频率, 以fs(10^-15s)为单位. */
#define HPET_CNT_CLK_PERIOD_SHIFT   32                  /*               这个值必须大于0, 且小于或等于0x05F5E100(100ns, 即10MHz) */
#define HPET_VENDOR_ID_MASK         (0xFFFFul<<16)      /* bit[31:16] RO */
#define HPET_VENDOR_ID_SHIFT        16
#define HPET_COUNT_SIZE_64          (1ul<<13)           /* RO 主计时器的宽度: 0=32 bits; 1=64 bits */
#define HPET_NUM_TIMERS_MASK        0x0F00ul            /* bit[12:8] RO 定时器的个数-1 */
#define HPET_NUM_TIMERS_SHIFT       8
#define HPET_REV_ID_MASK            0xFFul

/*
 * General Configuaration Register
 */
#define HPET_ENABLE_CNF             0x1                 /* 1=主计时器计时且允许定时器产生中断 */

/*
 * General Interrupt Status Register
 */
/*
 * 功能依赖于这个定时器的中断触发模式是电平触发还是边沿触发:
 * 如果是电平触发模式:
 *     这位默认是0. 当对应的定时器发生中断, 那么有硬件将其置1.一旦被置位, 软件往这位写1将会清空这位. 往这位写0, 则无意义.
 * 如果边沿触发模式:
 *     软件将忽略这位. 软件通常往这位写0.
 */
#define HPET_TM2_INT_STS            (1<<2)              /* Timer 2 Interrupt Active */
#define HPET_TM1_INT_STS            (1<<1)              /* Timer 1 Interrupt Active */
#define HPET_TM0_INT_STS            (1<<0)              /* Timer 0 Interrupt Active */

/*
 * Main Counter Value Register. 32位?
 */

/*
 * Timer N Configuration and Capabilities Register
 */
#define HPET_32MODE              (1<<8)          /* RO 0=定时器为32位 */

#define HPET_VAL_SET             (1<<6)          /* RW 只有能产生周期性中断的定时器才会使用这个域. 通过对这位写1,
                                                       软件能直接修改周期性定时期的累加器. 软件无需对这位清0.
                                                       只有Timer 0能产生周期性中断, 因此对Timer0来讲, 这位是可读可写.
                                                       而对于Timer1, Timer2, 这位默认为0, 且为只读. */
                                                       
#define HPET_SIZE_CAP            (1<<5)          /* RO Timer N的宽度(N为0-2). 0=32位宽 */

#define HPET_PERIODIC_INT_CAP    (1<<4)          /* RO Timer N Periodic Interrupt Capable(N为0-2):
                                                       1=定时器能产生周期性中断；0=定时器不能产生周期性中断 */
                                                       
#define HPET_PERIODIC_INT_EN     (1<<3)          /* Rx Timer N type(N为0-2):
                                                       如果对应的Tn_PER_INT_CAP位为0, 那么这位为只读, 且默认为0.
                                                       若对应的Tn_PER_INT_CAP位为1,那么这位可读可写. 用作使能相应的定时器产生周期性中断.
                                                       1: 使能定时器产生周期性中断 	R/W
                                                       0: 使能定时器产生非周期性中断 */
                                                       
#define HPET_INT_EN              (1<<2)          /* RW Timer N interrupt Enable(N为0-2):使能定时器产生中断 */

#define HPET_INT_LEVEL           (1<<1)          /* RW Timer N Interrupt Type(N为0-2):
                                                       0: 定时器的中断触发模式为边沿触发；这意位着对应的定时器将产生边沿触发中断.
                                                          若另外的的中断产生, 那么将产生另外的边沿.
                                                       1: 定时器的中断触发模式为电平触发；这意味着对应的定时器将产生电平触发中断.
                                                          这个中断将一直有效直到被软件清掉(General Interrupt Status Register). */

/*
 * Timer N Comparator Value Register. 32位?
 */

#ifdef __cplusplus
}
#endif

#endif // _LS2K_HPET_HW_H

