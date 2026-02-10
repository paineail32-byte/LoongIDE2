/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * err_s.S
 *
 * created: 2022-04-21
 *  author: Bian
 */

#include "cpu.h"
#include "asm.h"
#include "regdef.h"

#include "context.h"

//-----------------------------------------------------------------------------

    .text

//-----------------------------------------------------------------------------
// 机器错误处理入口, 安装到: MERRENTRY
//-----------------------------------------------------------------------------

    .extern     real_machine_error_entry

LEAF(machine_error_entry)
    csrwr       t8, LA_CSR_MERRSAVE             /* Save t8 in LA_CSR_MERRSAVE */
    la.abs      t8, real_machine_error_entry
    jirl        zero, t8, 0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
END(machine_error_entry)

//-----------------------------------------------------------------------------

    .extern     c_machine_error_handler

LEAF(real_machine_error_entry)
    csrrd       t8, LA_CSR_MERRCTL
    andi        t8, t8, CSR_MERRCTL_ISMERR
    beqz        t8, lbl_return
    
    csrrd       t8, LA_CSR_MERRCTL
    andi        t8, t8, CSR_MERRCTL_REPAIRABLE
    bnez        t8, lbl_return

    csrrd       t8, LA_CSR_MERRSAVE             /* Restore t8 from LA_CSR_MERRSAVE */
    SAVE_CONTEXT_ALL

    move        a0, sp
    la.abs      t8, c_machine_error_handler 
    jirl        zero, t8, 0                     /* no return, display info & deadloop */
    nop

lbl_return:
    csrrd       t8, LA_CSR_MERRSAVE             /* Restore t8 from LA_CSR_MERRSAVE */
    ertn
    nop
    
END(real_machine_error_entry)

//-----------------------------------------------------------------------------
// TLB 重填错误处理入口, 安装到: TLBRENTRY
//-----------------------------------------------------------------------------

    .extern     real_tlbrefill_error_entry

LEAF(tlbrefill_error_entry)
    csrwr       t8, LA_CSR_TLBRSAVE             /* Save t8 in LA_CSR_TLBRSAVE */
    la.abs      t8, real_tlbrefill_error_entry
    jirl        zero, t8, 0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
END(tlbrefill_error_entry)

//-----------------------------------------------------------------------------

    .extern     c_tlbrefill_error_handler

LEAF(real_tlbrefill_error_entry)

    csrrd       t8, LA_CSR_TLBRSAVE             /* Restore t8 from LA_CSR_TLBRSAVE */

    SAVE_CONTEXT_ALL

    move        a0, sp
    la.abs      t8, c_tlbrefill_error_handler
    jirl        zero, t8, 0                     /* no return, display info & deadloop */
    nop

END(real_tlbrefill_error_entry)

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

