/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * context.S
 *
 * created: 2024-7-16
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

    ld.d        sp, a0, 0               /* get new task stack pointer */

    RESTORE_CONTEXT_SWITCH
    ertn

END(rt_hw_context_switch_to)

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch(rt_uint64 from, rt_uint64 to)
// param:    a0 --> from
//           a1 --> to
//-------------------------------------------------------------------------------------------------

LEAF(rt_hw_context_switch)

    csrwr       ra, LA_CSR_EPC
    
    SAVE_CONTEXT_ALL

    st.d        sp, a0, 0               /* store sp in preempted tasks TCB */
    ld.d        sp, a1, 0               /* get new task stack pointer */

    RESTORE_CONTEXT_SWITCH
    ertn

END(rt_hw_context_switch)

//-------------------------------------------------------------------------------------------------
// function: void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to) 
//-------------------------------------------------------------------------------------------------

LEAF(rt_hw_context_switch_interrupt)

    la.abs      t0, rt_thread_switch_interrupt_flag
    ld.d        t1, t0, 0
    bnez        t1, lbl_switch

    li.d        t1, 1                           /* set rt_thread_switch_interrupt_flag to 1 */
    st.d        t1, t0, 0
    la.abs      t0, rt_interrupt_from_thread    /* set rt_interrupt_from_thread */
    st.d        a0, t0, 0

lbl_switch:
    la.abs      t0, rt_interrupt_to_thread      /* set rt_interrupt_to_thread */
    st.d        a1, t0, 0
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
    
    csrrd       t7, LA_CSR_KS2              /* Restore t7 from LA_CSR_KS2 */
    csrrd       t8, LA_CSR_KS3              /* Restore t7 from LA_CSR_KS3 */

    SAVE_CONTEXT_ALL

    csrwr       sp, LA_CSR_KS0              /* let LA_CSR_KS0 keep the current context sp */
    la          sp, _system_stack           /* switch to kernel stack */
    addi.d      sp, sp, -0x40               /* PAD needed? */

    la.abs      t8, rt_interrupt_enter
    jirl        ra, t8, 0
    
    csrrd       a0, LA_CSR_KS0
    la.abs      t8, c_interrupt_handler
    jirl        ra, t8, 0

    la.abs      t8, rt_interrupt_leave
    jirl        ra, t8, 0

    csrrd       sp, LA_CSR_KS0              /* switch sp back to thread context */

    /*
     * if rt_thread_switch_interrupt_flag set, jump to
     * rt_hw_context_switch_interrupt_do and do not return
     */
    la.abs      t0, rt_thread_switch_interrupt_flag
    ld.d        t1, t0, 0
    beqz        t1, exit_interrupt
    st.d        zero, t0, 0                     /* clear flag */

    /*
     * switch to the new thread
     */
    la.abs      t0, rt_interrupt_from_thread
    ld.d        t1, t0, 0
    st.d        sp, t1, 0                       /* store sp in preempted task TCB */

    la.abs      t0, rt_interrupt_to_thread
    ld.d        t1, t0, 0
    ld.d        sp, t1, 0                       /* get new task stack pointer */

exit_interrupt:

    RESTORE_CONTEXT_ISR
    ertn

END(rt_interrupt_handler)

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */

