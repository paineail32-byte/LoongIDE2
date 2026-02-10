/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * FreeRTOS Kernel V10.2.1
 */

#include "cpu.h"
#include "regdef.h"
#include "asm.h"

#include "context.h"

//-----------------------------------------------------------------------------

    .text

    .extern     pxCurrentTCB
    .extern     vTaskSwitchContext
    .extern     uxYieldFromISRFlag
    .extern     uxInsideInterrupt
    .extern     c_interrupt_handler
    
//-----------------------------------------------------------------------------
// switch into first task
//-----------------------------------------------------------------------------

LEAF(vPortStartFirstTask)

    la.abs      t0, pxCurrentTCB            /* the first ready task */
    ld.d        t1, t0, 0
    ld.d        sp, t1, 0

    RESTORE_CONTEXT_SWITCH
    ertn

END(vPortStartFirstTask)

//-----------------------------------------------------------------------------
// switch task from one to another
//-----------------------------------------------------------------------------

LEAF(vPortYield)

    csrwr       t7, LA_CSR_KS1              /* XXX disable interrupt while task switch */
    li.d        t7, CSR_CRMD_IE
    csrxchg     zero, t7, LA_CSR_CRMD
    csrrd       t7, LA_CSR_KS1

    csrwr       ra, LA_CSR_EPC              /* store RA ==> EPC, because ertn use epc not ra */

    SAVE_CONTEXT_ALL

    la.abs      t0, pxCurrentTCB            /* save the current task TCB */
    ld.d        t1, t0, 0
    st.d        sp, t1, 0

    la.abs      t0, vTaskSwitchContext      /* switch the task */
    jirl        ra, t0, 0

    la.abs      t0, pxCurrentTCB            /* fetch new task TCB */
    ld.d        t1, t0, 0
    ld.d        sp, t1, 0

    RESTORE_CONTEXT_SWITCH
    ertn

END(vPortYield)

//-----------------------------------------------------------------------------
// set the task switch flag for interrupt handler
//-----------------------------------------------------------------------------

LEAF(vPortYieldFromISR)

    la.abs      t0, uxYieldFromISRFlag
    st.d        a0, t0, 0
    jirl        zero, ra, 0

END(vPortYieldFromISR)
 
//-----------------------------------------------------------------------------
// FreeRTOS interrupt handler
//-----------------------------------------------------------------------------

LEAF(vPortInterruptHandler)

    csrrd       t7, LA_CSR_KS2              /* restore t7 from LA_CSR_KS2 */
    csrrd       t8, LA_CSR_KS3              /* restore t8 from LA_CSR_KS3 */

    SAVE_CONTEXT_ALL
    
    la.abs      t0, uxInsideInterrupt       /* set in interrupt flag */
    li.d        t1, 1
    st.d        t1, t0, 0

    csrwr       sp, LA_CSR_KS0              /* use system stack */
    la.abs      sp, __stack_end
    addi.d      sp, sp, -0x40               /* PAD needed? */

    csrrd       a0, LA_CSR_KS0
    la.abs      t0, c_interrupt_handler
    jirl        ra, t0, 0

    csrrd       sp, LA_CSR_KS0              /* restore saved stack */

    la.abs      t0, uxYieldFromISRFlag
    ld.d        t1, t0, 0
    bnez        t1, lbl_contextswitch

    la.abs      t0, uxInsideInterrupt       /* clear in interrupt flag */
    st.d        zero, t0, 0
    
    RESTORE_CONTEXT_ISR                     /* return A */
    ertn

lbl_contextswitch:

    st.d        zero, t0, 0                 /* Clear the Int Switch flag */

    la.abs      t0, pxCurrentTCB            /* save the current task TCB */
    ld.d        t1, t0, 0
    st.d        sp, t1, 0

    la.abs      t0, vTaskSwitchContext      /* switch the task */
    jirl        ra, t0, 0

    la.abs      t0, pxCurrentTCB            /* fetch new task TCB */
    ld.d        t1, t0, 0
    ld.d        sp, t1, 0

    la.abs      t0, uxInsideInterrupt       /* clear in interrupt flag */
    st.d        zero, t0, 0

    RESTORE_CONTEXT_ISR                     /* return B */
    ertn

END(vPortInterruptHandler)

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

