/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

/*
*********************************************************************************************************
*                                              uC/OS-III
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           LOONGSON 2K500
*
* File    : os_cpu_a.S
* Version : V3.08.01
*********************************************************************************************************
*/

#include "asm.h"
#include "regdef.h"
#include "cpu.h"

#include "context.h"

/*
*********************************************************************************************************
*                                          PUBLIC FUNCTIONS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                           CONSTANTS USED TO ACCESS TASK CONTEXT STACK
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           OSStartHighRdy()
*
* Description: Starts the highest priority task that is available to run.
*
* Note(s): 1) OSTaskStkInit(), which is responsible for initializing each task's stack, sets bit 0 of the
*             entry corresponding to the CRMD register.  Thus, interrupts will be enabled when each
*             task first runs.
*********************************************************************************************************
*/

    .text
    
    .extern     OSTaskSwHook
    .extern     OSTCBHighRdyPtr

LEAF(OSStartHighRdy)

    la.abs      t0, OSTaskSwHook            /* Call user defined task switch hook */
    jirl        ra, t0, 0
    nop
    
    la.abs      t0, OSTCBHighRdyPtr         /* Get highest priority task TCB address */
    ld.d        t1, t0, 0                   /* get stack pointer */
    ld.d        sp, t1, 0                   /* switch to the new stack */

    RESTORE_CONTEXT_SWITCH
    ertn
    nop

END(OSStartHighRdy)

/*
*********************************************************************************************************
*                                               OSCtxSw
*
* Description: Performs a Context switch from a task.  This function is ALWAYS called with interrupts
*              DISABLED.
*
*********************************************************************************************************
*/

    .extern     OSPrioHighRdy
    .extern     OSPrioCur
    .extern     OSTCBCurPtr

LEAF(OSCtxSw)

    csrwr       t7, LA_CSR_KS1              /* XXX disable interrupt while task switch */
    li.d        t7, CSR_CRMD_IE
    csrxchg     zero, t7, LA_CSR_CRMD
    csrrd       t7, LA_CSR_KS1

    csrwr       ra, LA_CSR_EPC              /* store RA ==> EPC, because ertn use epc not ra */

    SAVE_CONTEXT_ALL

    la.abs      t0, OSPrioHighRdy           /* Update the current priority */
    ld.b        t1, t0, 0
    la.abs      t2, OSPrioCur
    st.b        t1, t2, 0

    la.abs      t0, OSTCBCurPtr             /* Save the current task's stack pointer */
    ld.d        t1, t0, 0
    st.d        sp, t1, 0

    la.abs      t0, OSTaskSwHook            /* Call OSTaskSwHook() */
    jirl        ra, t0, 0
    nop

    la.abs      t0, OSTCBHighRdyPtr         /* Update the current TCB */
    ld.d        t1, t0, 0
    la.abs      t2, OSTCBCurPtr
    st.d        t1, t2, 0

    ld.d        sp, t1, 0                   /* Load the new task's stack pointer */

    RESTORE_CONTEXT_SWITCH
    ertn
    nop

END(OSCtxSw)

/*
*********************************************************************************************************
*                                             OSIntCtxSw()
*
* Description: This function is used to perform a context switch following an ISR.
*
*********************************************************************************************************
*/

    .extern     OSIntCtxSwFlag

LEAF(OSIntCtxSw)

    la.abs      t0, OSIntCtxSwFlag
    li.w        t1, 1
    st.w        t1, t0, 0
    jirl        zero, ra, 0
    nop

END(OSIntCtxSw)

/*
*********************************************************************************************************
*                                       CoreTimerIntHandler
*
* Description: The core timer, which is implemented by the Loongarch Timer,
*              is the source of uC/OS-III's tick interrupts.  This function handles those interrupts,
*              saving the current task's context and then calling OSTimeTick(), uC/OS-III's CPU-
*              independent routine for processing tick interrupts.
*
*********************************************************************************************************
*/

    .extern     OSIntEnter
    .extern     OSIntExit
    .extern     _gp
    .extern     __stack_end
    .extern     c_interrupt_handler

LEAF(UCOS_INTHandler)

    csrrd       t7, LA_CSR_KS2              /* restore t7 from LA_CSR_KS2 */
    csrrd       t8, LA_CSR_KS3              /* restore t8 from LA_CSR_KS3 */

    SAVE_CONTEXT_ALL

    csrwr       sp, LA_CSR_KS0              /* use system stack */
    la.abs      sp, __stack_end
    addi.d      sp, sp, -0x40               /* PAD needed? */

    la.abs      t0, OSIntEnter
    jirl        ra, t0, 0

    csrrd       a0, LA_CSR_KS0
    la.abs      t0, c_interrupt_handler
    jirl        ra, t0, 0
    
    la.abs      t0, OSIntExit
    jirl        ra, t0, 0

    csrrd       sp, LA_CSR_KS0              /* restore saved stack */
    
    la.abs      t0, OSIntCtxSwFlag          /* Interrupt Switch flag */
    ld.w        t1, t0, 0
    bnez        t1, _IntCtxSw

    RESTORE_CONTEXT_ISR                     /* return A */
    ertn

_IntCtxSw:

    st.w        zero, t0, 0                 /* Clear the Int Switch flag */

    la.abs      t0, OSPrioHighRdy           /* Update the current priority */
    ld.b        t1, t0, 0
    la.abs      t2, OSPrioCur
    st.b        t1, t2, 0

    la.abs      t0, OSTCBCurPtr             /* Save the current task's stack pointer */
    ld.d        t1, t0, 0
    st.d        sp, t1, 0

    la.abs      t0, OSTaskSwHook            /* Call OSTaskSwHook() */
    jirl        ra, t0, 0
    nop

    la.abs      t0, OSTCBHighRdyPtr         /* Update the current TCB */
    ld.d        t1, t0, 0
    la.abs      t2, OSTCBCurPtr
    st.d        t1, t2, 0
    ld.d        sp, t1, 0                   /* Load the new task's stack pointer */

    RESTORE_CONTEXT_ISR                     /* return B */
    ertn

END(UCOS_INTHandler)

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */
    
