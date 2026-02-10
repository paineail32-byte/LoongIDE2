/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
 /*
 * ls2k_adc_fast.S
 *
 * created: 2024-08-08
 *  author: Bian
 */

#include "cpu.h"
#include "asm.h"
#include "regdef.h"

//-----------------------------------------------------------------------------

    .text

/*
 * 单通道采样, 大约提升15~20%的速度.
 */

/*
 * int adc_read_1_fast(int channel, int samptime)
 */
LEAF(adc_read_1_fast)

    li.d        t0, 0x800000001611c000      /* ADC device base */
    li.w        t1, 1                       /* set sqr3@0x34 bit[4:0] as channel */
    blt         a0, t1, 1f
    nop
    li.w        t2, 8
    bgt         a0, t2, 1f
    nop
    li.w        t2, 5     
    blt         a0, t2, 2f
    nop
    addi.w      a0, a0, 4
2:
    sub.w       a0, a0, t1
    ld.w        t2, t0, 0x34
    li.w        t1, ~0x1F
    and         t2, t2, t1
    or          t2, t2, a0
    st.w        t2, t0, 0x34

    andi        a1, a1, 0x7                 /* set smpr2@0x10 bit[2:0] samptime */
    ld.w        t2, t0, 0x10
    li.w        t1, ~0x7
    and         t2, t2, t1
    or          t2, t2, a1
    st.w        t2, t0, 0x10

    ld.w        t3, t0, 0x0                 /* clear sr@0x00 bit[1] EOC */
    li.w        t1, ~(1<<1)
    and         t3, t3, t1
    st.w        t3, t0, 0x0

    ld.w        t2, t0, 0x8                 /* set cr2@0x08 bit[20] EXTTRIG && bit[22] SWSTART */
    li.w        t1, (1<<20) | (1<<22)
    or          t2, t2, t1
    st.w        t2, t0, 0x8

    li.w        t1, (1<<1)                  /* read sr@0x00 bit[1] EOC for wait done */
3:
    ld.w        t3, t0, 0x0
    and         t3, t3, t1
    beqz        t3, 3b
    nop

    ld.w        a0, t0, 0x4c                /* read dr@0x4c as result */
    b           9f
    nop
1: 
    move        a0, zero                    /* channel number is not 1~8 */
    nop
9:
    jirl        zero, ra, 0
    nop
END(adc_read_1_fast)


