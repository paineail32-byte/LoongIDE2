/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq_s.S
 *
 * created: 2023-06-10
 *  author: Bian
 */

#include "asm.h"
#include "cpu.h"
#include "regdef.h"

#include "context.h"

    .text

//-----------------------------------------------------------------------------
// 安装到: EENTRY, TLBRENTRY, MERRENTRY 处
//-----------------------------------------------------------------------------

    .extern     real_exception_entry

LEAF(exception_common_entry)

    csrwr       t7, LA_CSR_SAVE0            /* Save t7 in LA_CSR_SAVE0 */
    csrwr       t8, LA_CSR_SAVE1            /* Save t8 in LA_CSR_SAVE1 */
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

    .extern     c_interrupt_handler
    .extern     c_exception_handler
    
LEAF(real_exception_entry)

    csrrd       t7, LA_CSR_ESTAT            /* exception state */
    li.w        t8, CSR_ESTAT_ECODE_MASK
    and         t8, t8, t7
    bnez        t8, lbl_exception           /* 例外或者中断 */
    nop

    /**************
     * 中断
     **************/
     
    la.abs      t8, rt_interrupt_handler    /* content.S */
    jirl        zero, t8, 0
    nop

    /**************
     * 例外
     **************/
     
lbl_exception:
    
    csrrd       t7, LA_CSR_SAVE0            /* Restore t7 from LA_CSR_SAVE0 */
    csrrd       t8, LA_CSR_SAVE1            /* Restore t8 from LA_CSR_SAVE1 */

    SAVE_CONTEXT_ALL

    move        a0, sp
    la.abs      t7, c_exception_handler
    jirl        zero, t7, 0
    nop

END(real_exception_entry)

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

