/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq_s.S
 *
 * created: 2022-02-18
 *  author: Bian
 */

#include "cpu.h"
#include "asm.h"
#include "regdef.h"

#include "context.h"

    .text

//-----------------------------------------------------------------------------
// 安装到: EENTRY, TLBRENTRY, MERRENTRY 处
//-----------------------------------------------------------------------------

    .extern     real_exception_entry

LEAF(exception_common_entry)

    csrwr       t7, LA_CSR_KS2              /* Save t7 in LA_CSR_KS2 */
    csrwr       t8, LA_CSR_KS3              /* Save t8 in LA_CSR_KS3 */
    la.abs      t8, real_exception_entry    
    jirl        zero, t8, 0                
    nop
    nop
    nop
    nop
    
END(exception_common_entry)

//-----------------------------------------------------------------------------
// 中断处理入口
//-----------------------------------------------------------------------------

    .extern     RunningInsideISR
    .extern     c_interrupt_handler

LEAF(real_exception_entry)

    csrrd       t7, LA_CSR_ESTAT            /* exception state */
    li.d        t8, CSR_ESTAT_EXC_MASK
    and         t8, t8, t7
    beqz        t8, lbl_interrupt           /* 中断 */
    nop
    la.abs      t8, exception_handler       /* 例外 */
    jirl        zero, t8, 0
    nop

lbl_interrupt:

    la.abs      t7, RunningInsideISR        /* 设置运行在中断中的标志 */
    addi.w      t8, zero, 1
    st.w        t8, t7, 0

    csrrd       t7, LA_CSR_KS2              /* Restore t7 from LA_CSR_KS2 */
    csrrd       t8, LA_CSR_KS3              /* Restore t8 from LA_CSR_KS3 */

    SAVE_CONTEXT_ALL

    /*
     * 跳转到 c_interrupt_handler 执行
     */
    csrwr       sp, LA_CSR_KS0              /* use system stack */
    la.abs      sp, __stack_end
    addi.d      sp, sp, -0x40               /* PAD is needed */

    csrrd       a0, LA_CSR_KS0
    la.abs      t8, c_interrupt_handler
    jirl        ra, t8, 0

    csrrd       sp, LA_CSR_KS0              /* restore saved stack */

    la.abs      t7, RunningInsideISR        /* 清除运行在中断中的标志 */
    st.w        zero, t7, 0

    RESTORE_CONTEXT_ISR

    ertn                                    /* 中断返回 */
    nop

END(real_exception_entry)

//-----------------------------------------------------------------------------

    .extern     c_exception_handler

LEAF(exception_handler)
    csrrd       t7, LA_CSR_KS2              /* Restore t7 from LA_CSR_KS2 */
    csrrd       t8, LA_CSR_KS3              /* Restore t8 from LA_CSR_KS3 */

    SAVE_CONTEXT_ALL

    move        a0, sp
    la.abs      t8, c_exception_handler
    jirl        zero, t8, 0
    nop

END(exception_handler)

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

