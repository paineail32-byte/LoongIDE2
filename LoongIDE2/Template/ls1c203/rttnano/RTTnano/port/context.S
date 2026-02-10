/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * context.S
 *
 * created: 2022/4/7
 *  author: Bian
 */

#include "asm.h"
#include "regdef.h"
#include "cpu.h"

#include "context.h"
    
//-------------------------------------------------------------------------------------------------

    .data

    .align 3

rt_thread_switch_interrupt_flag:
    .dword  0
rt_interrupt_from_thread:
    .dword  0
rt_interrupt_to_thread:
    .dword  0

//-------------------------------------------------------------------------------------------------

    .text

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch_to(rt_uint64 to)
// params:   a0 --> to
//-------------------------------------------------------------------------------------------------

LEAF(rt_hw_context_switch_to)

    ld.w        sp, a0, 0               /* get new task stack pointer */

    RESTORE_CONTEXT_SWITCH

    csrrd       x, LA_CSR_ERA
    ertn
    nop

END(rt_hw_context_switch_to)

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch(rt_uint64 from, rt_uint64 to)
// param:    a0 --> from
//           a1 --> to
//-------------------------------------------------------------------------------------------------

LEAF(rt_hw_context_switch)

    csrwr       ra, LA_CSR_ERA

    SAVE_CONTEXT_ALL

    st.w        sp, a0, 0               /* store sp in preempted tasks TCB */
    ld.w        sp, a1, 0               /* get new task stack pointer */

    RESTORE_CONTEXT_SWITCH
    ertn
    nop

END(rt_hw_context_switch)

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to) 
//-------------------------------------------------------------------------------------------------

LEAF(rt_hw_context_switch_interrupt)

    la.abs      t0, rt_thread_switch_interrupt_flag
    ld.w        t1, t0, 0
    bnez        t1, lbl_switch
    
    li.w        t1, 1                           /* set rt_thread_switch_interrupt_flag to 1 */
    st.w        t1, t0, 0
    la.abs      t0, rt_interrupt_from_thread    /* set rt_interrupt_from_thread */
    st.w        a0, t0, 0
    
lbl_switch:
    la.abs      t0, rt_interrupt_to_thread      /* set rt_interrupt_to_thread */
    st.w        a1, t0, 0
    jirl        zero, ra, 0
    nop

END(rt_hw_context_switch_interrupt)

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch_interrupt_do(rt_base_t flag)
//-------------------------------------------------------------------------------------------------

    .extern rt_interrupt_enter
    .extern rt_interrupt_leave
    .extern c_interrupt_handler

LEAF(rt_interrupt_handler)
    
    csrrd       t7, LA_CSR_SAVE0            /* Restore t7 from LA_CSR_SAVE0 */
    csrrd       t8, LA_CSR_SAVE1            /* Restore t7 from LA_CSR_SAVE1 */

    SAVE_CONTEXT_ALL

    la.abs      t8, rt_interrupt_enter
    jirl        ra, t8, 0
    
    move        a0, sp
    la.abs      t8, c_interrupt_handler
    jirl        ra, t8, 0

    la.abs      t8, rt_interrupt_leave
    jirl        ra, t8, 0

    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and do not return
     */
    la.abs      t0, rt_thread_switch_interrupt_flag
    ld.w        t1, t0, 0
    beqz        t1, exit_interrupt
    st.w        zero, t0, 0                     /* clear flag */

    /*
     * switch to the new thread
     */
    la.abs      t0, rt_interrupt_from_thread
    ld.w        t1, t0, 0
    st.w        sp, t1, 0                       /* store sp in preempted task TCB */

    la.abs      t0, rt_interrupt_to_thread
    ld.w        t1, t0, 0
    ld.w        sp, t1, 0                       /* get new task stack pointer */

exit_interrupt:

    RESTORE_CONTEXT_ISR
    ertn
    nop

END(rt_interrupt_handler)

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */

