/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * context.h
 *
 * created: 2023/5/1
 *  author: Bian
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "regdef.h"
#include "cpu.h"

//-----------------------------------------------------------------------------

#define CTX_OFFSET(n)       ((n)*4)

#define CTX_SIZE            (GR_NUMS*4)

#ifdef __ASSEMBLER__

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷ÈëÕ»: ra, tp, sp, a0~a7, t0~t8, x, fp, s0~s8; some CSRs
//-----------------------------------------------------------------------------

.macro SAVE_CONTEXT_ALL

    addi.w      sp, sp, -CTX_SIZE               /* Grown-down Stack pointer*/

    st.w        zero, sp, CTX_OFFSET(R_ZERO)    /* push it maybe some usage? */
    st.w        ra, sp, CTX_OFFSET(R_RA)
    st.w        tp, sp, CTX_OFFSET(R_TP)
//  st.w        sp, sp, CTX_OFFSET(R_SP)        /* need push it? */
    st.w        a0, sp, CTX_OFFSET(R_A0)
    st.w        a1, sp, CTX_OFFSET(R_A1)
    st.w        a2, sp, CTX_OFFSET(R_A2)
    st.w        a3, sp, CTX_OFFSET(R_A3)
    st.w        a4, sp, CTX_OFFSET(R_A4)
    st.w        a5, sp, CTX_OFFSET(R_A5)
    st.w        a6, sp, CTX_OFFSET(R_A6)
    st.w        a7, sp, CTX_OFFSET(R_A7)
    st.w        t0, sp, CTX_OFFSET(R_T0)
    st.w        t1, sp, CTX_OFFSET(R_T1)
    st.w        t2, sp, CTX_OFFSET(R_T2)
    st.w        t3, sp, CTX_OFFSET(R_T3)
    st.w        t4, sp, CTX_OFFSET(R_T4)
    st.w        t5, sp, CTX_OFFSET(R_T5)
    st.w        t6, sp, CTX_OFFSET(R_T6)
    st.w        t7, sp, CTX_OFFSET(R_T7)
    st.w        t8, sp, CTX_OFFSET(R_T8)
    st.w        x,  sp, CTX_OFFSET(R_X)         /* Reserved Register ? */
    st.w        fp, sp, CTX_OFFSET(R_FP)
    st.w        s0, sp, CTX_OFFSET(R_S0)
    st.w        s1, sp, CTX_OFFSET(R_S1)
    st.w        s2, sp, CTX_OFFSET(R_S2)
    st.w        s3, sp, CTX_OFFSET(R_S3)
    st.w        s4, sp, CTX_OFFSET(R_S4)
    st.w        s5, sp, CTX_OFFSET(R_S5)
    st.w        s6, sp, CTX_OFFSET(R_S6)
    st.w        s7, sp, CTX_OFFSET(R_S7)
    st.w        s8, sp, CTX_OFFSET(R_S8)

    csrrd       t8, LA_CSR_CRMD                 /* CSR Registers */
    st.w        t8, sp, CTX_OFFSET(R_CRMD)
    csrrd       t8, LA_CSR_ECFG
    st.w        t8, sp, CTX_OFFSET(R_ECFG)
    csrrd       t8, LA_CSR_ESTAT
    st.w        t8, sp, CTX_OFFSET(R_ESTAT)
    csrrd       t8, LA_CSR_EUEN
    st.w        t8, sp, CTX_OFFSET(R_EUEN)      /* FPU is using? */
    csrrd       t8, LA_CSR_PRMD
    st.w        t8, sp, CTX_OFFSET(R_PRMD)
    csrrd       t8, LA_CSR_ERA
    st.w        t8, sp, CTX_OFFSET(R_ERA)

.endm

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷³öÕ»: ra, tp, sp, a0~a7, t0~t8, x, fp, s0~s8;
//-----------------------------------------------------------------------------

.macro RESTORE_REGISTERS

//  ld.w        zero, sp, CTX_OFFSET(R_ZERO)    /* needn't pop */
    ld.w        ra, sp, CTX_OFFSET(R_RA)
    ld.w        tp, sp, CTX_OFFSET(R_TP)
//  ld.w        sp, sp, CTX_OFFSET(R_SP)        /* needn't pop */
    ld.w        a0, sp, CTX_OFFSET(R_A0)
    ld.w        a1, sp, CTX_OFFSET(R_A1)
    ld.w        a2, sp, CTX_OFFSET(R_A2)
    ld.w        a3, sp, CTX_OFFSET(R_A3)
    ld.w        a4, sp, CTX_OFFSET(R_A4)
    ld.w        a5, sp, CTX_OFFSET(R_A5)
    ld.w        a6, sp, CTX_OFFSET(R_A6)
    ld.w        a7, sp, CTX_OFFSET(R_A7)
    ld.w        t0, sp, CTX_OFFSET(R_T0)
    ld.w        t1, sp, CTX_OFFSET(R_T1)
    ld.w        t2, sp, CTX_OFFSET(R_T2)
    ld.w        t3, sp, CTX_OFFSET(R_T3)
    ld.w        t4, sp, CTX_OFFSET(R_T4)
    ld.w        t5, sp, CTX_OFFSET(R_T5)
    ld.w        t6, sp, CTX_OFFSET(R_T6)
    ld.w        t7, sp, CTX_OFFSET(R_T7)
//  ld.w        t8, sp, CTX_OFFSET(R_T8)        /* pop from stack last */
    ld.w        x,  sp, CTX_OFFSET(R_X)         /* Reserved Register */
    ld.w        fp, sp, CTX_OFFSET(R_FP)
    ld.w        s0, sp, CTX_OFFSET(R_S0)
    ld.w        s1, sp, CTX_OFFSET(R_S1)
    ld.w        s2, sp, CTX_OFFSET(R_S2)
    ld.w        s3, sp, CTX_OFFSET(R_S3)
    ld.w        s4, sp, CTX_OFFSET(R_S4)
    ld.w        s5, sp, CTX_OFFSET(R_S5)
    ld.w        s6, sp, CTX_OFFSET(R_S6)
    ld.w        s7, sp, CTX_OFFSET(R_S7)
    ld.w        s8, sp, CTX_OFFSET(R_S8)

.endm

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷³öÕ»: ´ÓÖÐ¶Ï·µ»Ø
//-----------------------------------------------------------------------------

.macro RESTORE_CONTEXT_ISR

    RESTORE_REGISTERS

//  li.w        t8, CSR_CRMD_IE                 /* disable interrupt */
//  csrxchg     zero, t8, LA_CSR_CRMD

    ld.w        t8, sp, CTX_OFFSET(R_ERA)       /* CSR Registers */
    csrwr       t8, LA_CSR_ERA
    ld.w        t8, sp, CTX_OFFSET(R_PRMD)
    csrwr       t8, LA_CSR_PRMD

    ld.w        t8, sp, CTX_OFFSET(R_T8)        /* pop t8 from stack last */
    addi.w      sp, sp, CTX_SIZE                /* Grown-up Stack pointer */

.endm

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷³öÕ»: ÈÎÎñÇÐ»»
//-----------------------------------------------------------------------------

.macro RESTORE_CONTEXT_SWITCH

    RESTORE_REGISTERS

//  li.w        t8, CSR_CRMD_IE                 /* disable interrupt */
//  csrxchg     zero, t8, LA_CSR_CRMD

    ld.w        t8, sp, CTX_OFFSET(R_ERA)       /* CSR Registers */
    csrwr       t8, LA_CSR_ERA

    ld.w        t8, sp, CTX_OFFSET(R_T8)        /* pop t8 from stack last */
    addi.w      sp, sp, CTX_SIZE                /* Grown-up Stack pointer */

.endm

#endif // #ifdef __ASSEMBLER__

#endif /*__CONTEXT_H__*/

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */

