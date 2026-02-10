/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * context.h
 *
 * created: 2024-6-18
 *  author: Bian
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "regdef.h"
#include "cpu.h"

//-----------------------------------------------------------------------------

#define CTX_OFFSET(n)   ((n)*8)

#if __loongarch_hard_float
#define CTX_SIZE        ((GR_NUMS+FR_NUMS)*8)
#else
#define CTX_SIZE        (GR_NUMS*8)
#endif

#ifdef __ASSEMBLER__

#if __loongarch_hard_float

#define FR_BASE         GR_NUMS

//-----------------------------------------------------------------------------
// ¸¡µã¼Ä´æÆ÷ÈëÕ»: f0~f31, fcc0~fcc7 & fcsr0
//-----------------------------------------------------------------------------

.macro SAVE_FPU

    fst.d       $f0,  sp, CTX_OFFSET(FR_BASE+R_F0)
    fst.d       $f1,  sp, CTX_OFFSET(FR_BASE+R_F1)
    fst.d       $f2,  sp, CTX_OFFSET(FR_BASE+R_F2)
    fst.d       $f3,  sp, CTX_OFFSET(FR_BASE+R_F3)
    fst.d       $f4,  sp, CTX_OFFSET(FR_BASE+R_F4)
    fst.d       $f5,  sp, CTX_OFFSET(FR_BASE+R_F5)
    fst.d       $f6,  sp, CTX_OFFSET(FR_BASE+R_F6)
    fst.d       $f7,  sp, CTX_OFFSET(FR_BASE+R_F7)
    fst.d       $f8,  sp, CTX_OFFSET(FR_BASE+R_F8)
    fst.d       $f9,  sp, CTX_OFFSET(FR_BASE+R_F9)
    fst.d       $f10, sp, CTX_OFFSET(FR_BASE+R_F10)
    fst.d       $f11, sp, CTX_OFFSET(FR_BASE+R_F11)
    fst.d       $f12, sp, CTX_OFFSET(FR_BASE+R_F12)
    fst.d       $f13, sp, CTX_OFFSET(FR_BASE+R_F13)
    fst.d       $f14, sp, CTX_OFFSET(FR_BASE+R_F14)
    fst.d       $f15, sp, CTX_OFFSET(FR_BASE+R_F15)
    fst.d       $f16, sp, CTX_OFFSET(FR_BASE+R_F16)
    fst.d       $f17, sp, CTX_OFFSET(FR_BASE+R_F17)
    fst.d       $f18, sp, CTX_OFFSET(FR_BASE+R_F18)
    fst.d       $f19, sp, CTX_OFFSET(FR_BASE+R_F19)
    fst.d       $f20, sp, CTX_OFFSET(FR_BASE+R_F20)
    fst.d       $f21, sp, CTX_OFFSET(FR_BASE+R_F21)
    fst.d       $f22, sp, CTX_OFFSET(FR_BASE+R_F22)
    fst.d       $f23, sp, CTX_OFFSET(FR_BASE+R_F23)
    fst.d       $f24, sp, CTX_OFFSET(FR_BASE+R_F24)
    fst.d       $f25, sp, CTX_OFFSET(FR_BASE+R_F25)
    fst.d       $f26, sp, CTX_OFFSET(FR_BASE+R_F26)
    fst.d       $f27, sp, CTX_OFFSET(FR_BASE+R_F27)
    fst.d       $f28, sp, CTX_OFFSET(FR_BASE+R_F28)
    fst.d       $f29, sp, CTX_OFFSET(FR_BASE+R_F29)
    fst.d       $f30, sp, CTX_OFFSET(FR_BASE+R_F30)
    fst.d       $f31, sp, CTX_OFFSET(FR_BASE+R_F31)
    movcf2gr    t8, $fcc0
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC0)
    movcf2gr    t8, $fcc1
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC1)
    movcf2gr    t8, $fcc2
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC2)
    movcf2gr    t8, $fcc3
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC3)
    movcf2gr    t8, $fcc4
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC4)
    movcf2gr    t8, $fcc5
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC5)
    movcf2gr    t8, $fcc6
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC6)
    movcf2gr    t8, $fcc7
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCC7)
    movfcsr2gr  t8, $r0
    st.d        t8, sp, CTX_OFFSET(FR_BASE+R_FCSR0)
    
.endm

//-----------------------------------------------------------------------------
// ¸¡µã¼Ä´æÆ÷³öÕ»: f0~f31, fcc0~fcc7 & fcsr0
//-----------------------------------------------------------------------------

.macro RESTORE_FPU

    fld.d       $f0,  sp, CTX_OFFSET(FR_BASE+R_F0)
    fld.d       $f1,  sp, CTX_OFFSET(FR_BASE+R_F1)
    fld.d       $f2,  sp, CTX_OFFSET(FR_BASE+R_F2)
    fld.d       $f3,  sp, CTX_OFFSET(FR_BASE+R_F3)
    fld.d       $f4,  sp, CTX_OFFSET(FR_BASE+R_F4)
    fld.d       $f5,  sp, CTX_OFFSET(FR_BASE+R_F5)
    fld.d       $f6,  sp, CTX_OFFSET(FR_BASE+R_F6)
    fld.d       $f7,  sp, CTX_OFFSET(FR_BASE+R_F7)
    fld.d       $f8,  sp, CTX_OFFSET(FR_BASE+R_F8)
    fld.d       $f9,  sp, CTX_OFFSET(FR_BASE+R_F9)
    fld.d       $f10, sp, CTX_OFFSET(FR_BASE+R_F10)
    fld.d       $f11, sp, CTX_OFFSET(FR_BASE+R_F11)
    fld.d       $f12, sp, CTX_OFFSET(FR_BASE+R_F12)
    fld.d       $f13, sp, CTX_OFFSET(FR_BASE+R_F13)
    fld.d       $f14, sp, CTX_OFFSET(FR_BASE+R_F14)
    fld.d       $f15, sp, CTX_OFFSET(FR_BASE+R_F15)
    fld.d       $f16, sp, CTX_OFFSET(FR_BASE+R_F16)
    fld.d       $f17, sp, CTX_OFFSET(FR_BASE+R_F17)
    fld.d       $f18, sp, CTX_OFFSET(FR_BASE+R_F18)
    fld.d       $f19, sp, CTX_OFFSET(FR_BASE+R_F19)
    fld.d       $f20, sp, CTX_OFFSET(FR_BASE+R_F20)
    fld.d       $f21, sp, CTX_OFFSET(FR_BASE+R_F21)
    fld.d       $f22, sp, CTX_OFFSET(FR_BASE+R_F22)
    fld.d       $f23, sp, CTX_OFFSET(FR_BASE+R_F23)
    fld.d       $f24, sp, CTX_OFFSET(FR_BASE+R_F24)
    fld.d       $f25, sp, CTX_OFFSET(FR_BASE+R_F25)
    fld.d       $f26, sp, CTX_OFFSET(FR_BASE+R_F26)
    fld.d       $f27, sp, CTX_OFFSET(FR_BASE+R_F27)
    fld.d       $f28, sp, CTX_OFFSET(FR_BASE+R_F28)
    fld.d       $f29, sp, CTX_OFFSET(FR_BASE+R_F29)
    fld.d       $f30, sp, CTX_OFFSET(FR_BASE+R_F30)
    fld.d       $f31, sp, CTX_OFFSET(FR_BASE+R_F31)
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC0)
    movgr2cf    $fcc0, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC1)
    movgr2cf    $fcc1, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC2)
    movgr2cf    $fcc2, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC3)
    movgr2cf    $fcc3, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC4)
    movgr2cf    $fcc4, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC5)
    movgr2cf    $fcc5, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC6)
    movgr2cf    $fcc6, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCC7)
    movgr2cf    $fcc7, x
    ld.d        x, sp, CTX_OFFSET(FR_BASE+R_FCSR0)
    movgr2fcsr  $r0, x

.endm

#endif // #if __loongarch_hard_float

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷ÈëÕ»: ra, tp, sp, a0~a7, t0~t8, x, fp, s0~s8; some CSRs
//-----------------------------------------------------------------------------

.macro SAVE_CONTEXT_ALL

    addi.d      sp, sp, -CTX_SIZE               /* Grown-down Stack pointer*/
    
    st.d        zero, sp, CTX_OFFSET(R_ZERO)    /* push it maybe some usage? */
    st.d        ra, sp, CTX_OFFSET(R_RA)
    st.d        tp, sp, CTX_OFFSET(R_TP)
//  st.d        sp, sp, CTX_OFFSET(R_SP)        /* need push it? */
    st.d        a0, sp, CTX_OFFSET(R_A0)
    st.d        a1, sp, CTX_OFFSET(R_A1)
    st.d        a2, sp, CTX_OFFSET(R_A2)
    st.d        a3, sp, CTX_OFFSET(R_A3)
    st.d        a4, sp, CTX_OFFSET(R_A4)
    st.d        a5, sp, CTX_OFFSET(R_A5)
    st.d        a6, sp, CTX_OFFSET(R_A6)
    st.d        a7, sp, CTX_OFFSET(R_A7)
    st.d        t0, sp, CTX_OFFSET(R_T0)
    st.d        t1, sp, CTX_OFFSET(R_T1)
    st.d        t2, sp, CTX_OFFSET(R_T2)
    st.d        t3, sp, CTX_OFFSET(R_T3)
    st.d        t4, sp, CTX_OFFSET(R_T4)
    st.d        t5, sp, CTX_OFFSET(R_T5)
    st.d        t6, sp, CTX_OFFSET(R_T6)
    st.d        t7, sp, CTX_OFFSET(R_T7)
    st.d        t8, sp, CTX_OFFSET(R_T8)
    st.d        x,  sp, CTX_OFFSET(R_X)         /* Reserved Register ? */
    st.d        fp, sp, CTX_OFFSET(R_FP)
    st.d        s0, sp, CTX_OFFSET(R_S0)
    st.d        s1, sp, CTX_OFFSET(R_S1)
    st.d        s2, sp, CTX_OFFSET(R_S2)
    st.d        s3, sp, CTX_OFFSET(R_S3)
    st.d        s4, sp, CTX_OFFSET(R_S4)
    st.d        s5, sp, CTX_OFFSET(R_S5)
    st.d        s6, sp, CTX_OFFSET(R_S6)
    st.d        s7, sp, CTX_OFFSET(R_S7)
    st.d        s8, sp, CTX_OFFSET(R_S8)

    csrrd       t8, LA_CSR_CRMD                 /* CSR Registers */
    st.d        t8, sp, CTX_OFFSET(R_CRMD)
    csrrd       t8, LA_CSR_ECFG
    st.d        t8, sp, CTX_OFFSET(R_ECFG)
    csrrd       t8, LA_CSR_ESTAT
    st.d        t8, sp, CTX_OFFSET(R_ESTAT)
    csrrd       t8, LA_CSR_EUEN
    st.d        t8, sp, CTX_OFFSET(R_EUEN)      /* FPU is using? */

#if __loongarch_hard_float
    andi        t8, t8, CSR_EUEN_FPEN
    beqz        t8, 1f
    SAVE_FPU                                    /* FPU Registers */
1:
#endif

    csrrd       t8, LA_CSR_PRMD
    st.d        t8, sp, CTX_OFFSET(R_PRMD)
    csrrd       t8, LA_CSR_EPC
    st.d        t8, sp, CTX_OFFSET(R_EPC)

.endm

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷³öÕ»: ra, tp, sp, a0~a7, t0~t8, x, fp, s0~s8;
//-----------------------------------------------------------------------------

.macro RESTORE_REGISTERS

//  ld.d        zero, sp, CTX_OFFSET(R_ZERO)    /* needn't pop */
    ld.d        ra, sp, CTX_OFFSET(R_RA)
    ld.d        tp, sp, CTX_OFFSET(R_TP)
//  ld.d        sp, sp, CTX_OFFSET(R_SP)        /* needn't pop */
    ld.d        a0, sp, CTX_OFFSET(R_A0)
    ld.d        a1, sp, CTX_OFFSET(R_A1)
    ld.d        a2, sp, CTX_OFFSET(R_A2)
    ld.d        a3, sp, CTX_OFFSET(R_A3)
    ld.d        a4, sp, CTX_OFFSET(R_A4)
    ld.d        a5, sp, CTX_OFFSET(R_A5)
    ld.d        a6, sp, CTX_OFFSET(R_A6)
    ld.d        a7, sp, CTX_OFFSET(R_A7)
    ld.d        t0, sp, CTX_OFFSET(R_T0)
    ld.d        t1, sp, CTX_OFFSET(R_T1)
    ld.d        t2, sp, CTX_OFFSET(R_T2)
    ld.d        t3, sp, CTX_OFFSET(R_T3)
    ld.d        t4, sp, CTX_OFFSET(R_T4)
    ld.d        t5, sp, CTX_OFFSET(R_T5)
    ld.d        t6, sp, CTX_OFFSET(R_T6)
    ld.d        t7, sp, CTX_OFFSET(R_T7)
//  ld.d        t8, sp, CTX_OFFSET(R_T8)        /* pop from stack last */
    ld.d        x,  sp, CTX_OFFSET(R_X)         /* Reserved Register */
    ld.d        fp, sp, CTX_OFFSET(R_FP)
    ld.d        s0, sp, CTX_OFFSET(R_S0)
    ld.d        s1, sp, CTX_OFFSET(R_S1)
    ld.d        s2, sp, CTX_OFFSET(R_S2)
    ld.d        s3, sp, CTX_OFFSET(R_S3)
    ld.d        s4, sp, CTX_OFFSET(R_S4)
    ld.d        s5, sp, CTX_OFFSET(R_S5)
    ld.d        s6, sp, CTX_OFFSET(R_S6)
    ld.d        s7, sp, CTX_OFFSET(R_S7)
    ld.d        s8, sp, CTX_OFFSET(R_S8)

#if __loongarch_hard_float
    ld.d        t8, sp, CTX_OFFSET(R_EUEN)      /* FPU is using? */
    andi        t8, t8, CSR_EUEN_FPEN
    beqz        t8, 1f
    RESTORE_FPU                                 /* FPU Registers */
1:
    nop
#endif

.endm

//-----------------------------------------------------------------------------
// ¼Ä´æÆ÷³öÕ»: ´ÓÖÐ¶Ï·µ»Ø
//-----------------------------------------------------------------------------

.macro RESTORE_CONTEXT_ISR

    RESTORE_REGISTERS

//	li.d        t8, CSR_CRMD_IE                 /* disable interrupt */
//	csrxchg     zero, t8, LA_CSR_CRMD

    ld.d        t8, sp, CTX_OFFSET(R_EPC)       /* CSR Registers */
    csrwr       t8, LA_CSR_EPC
    ld.d        t8, sp, CTX_OFFSET(R_PRMD)
//	ori         t8, t8, CSR_PRMD_PWE | CSR_PRMD_PIE
    csrwr       t8, LA_CSR_PRMD

    ld.d        t8, sp, CTX_OFFSET(R_T8)        /* pop t8 from stack last */
    addi.d      sp, sp, CTX_SIZE                /* Grown-up Stack pointer */

.endm

#endif // #ifdef __ASSEMBLER__

#endif /*__CONTEXT_H__*/

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */

