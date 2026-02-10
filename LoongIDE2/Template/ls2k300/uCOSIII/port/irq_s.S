/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq_c.S
 *
 * created: 2024-07-18
 *  author: Bian
 */

#include "cpu.h"
#include "asm.h"
#include "regdef.h"

#include "context.h"

//-----------------------------------------------------------------------------

    .text

//-----------------------------------------------------------------------------
// 安装到: EENTRY, TLBRENTRY, MERRENTRY 处
//-----------------------------------------------------------------------------

    .extern     real_exception_entry

LEAF(exception_common_entry)
    csrwr       t7, LA_CSR_KS2              /* Save t7 to LA_CSR_KS2 */
    csrwr       t8, LA_CSR_KS3              /* Save t8 to LA_CSR_KS3 */
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

    .extern     UCOS_INTHandler
    .extern     exception_handler
    
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
    la.abs      t8, UCOS_INTHandler
    jirl        zero, t8, 0
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

