/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * start.S
 *
 * created: 2024-07-18
 *  author: Bian
 */

#include "asm.h"
#include "cpu.h"
#include "regdef.h"

//-------------------------------------------------------------------------------------------------

	.extern __bss_start, 8                  /* The name in ld.script */
	.extern __bss_end, 8                    /* The name in ld.script */
	.extern __stack_start, 8                /* The name in ld.script */
	.extern __stack_end, 8                  /* The name in ld.script */

//-------------------------------------------------------------------------------------------------

    .section ".start", "ax"                 // .text

//-------------------------------------------------------------------------------------------------

    .align 3
    .globl start
    .globl _start
start:
_start:
 
    li.d        t0, CSR_CRMD_IE             /* disable interrupt */
    csrxchg     zero, t0, LA_CSR_CRMD       /* clear IE bit */

    li.w        t1, CSR_ECFG_IM_MASK        /* set interrupt mask 0x1fff */
    csrwr       t1, LA_CSR_ECFG

    csrwr       zero, LA_CSR_MERREBASE      /* set machine error entry */

    li.d        t2, 0x9000000000001000      /* set common-exception entry */
    csrwr       t2, LA_CSR_EBASE

    li.d        t3, 0x0000000000002000      /* set tlb-refill error entry */
    csrwr       t3, LA_CSR_TLBREBASE

    li.d        t4, CSR_EUEN_FPEN           /* 使能浮点协处理器 */
    csrwr       t4, LA_CSR_EUEN

    la.abs      gp, _gp                     /* initialize gp */

    /*
     * clear bss
     */
    la.abs      t0, __bss_start
    la.abs      t1, __bss_end
//  addi.d      t1, t1, -8
1:
    st.d        zero, t0, 0
    addi.d      t0, t0, 8
    bne         t0, t1, 1b

    /*
     * clear stack
     */
    la.abs      t0, __stack_start
    la.abs      t1, __stack_end
    
    move        sp, t1                      /* initialize sp */
    li.d        t2, 0x2000
    sub.d       sp, sp, t2                  /* Left 8K isr stack */

    addi.d      t1, t1, -8
2:
    st.d        zero, t0, 0
    addi.d      t0, t0, 8
    bne         t0, t1, 2b

    /*
     * Set memory size global
     */
    la.abs      a0, _RamSize
    la.abs      t8, set_memory_size
    jirl        ra, t8, 0

    /*
     * Initialize cahce
     */
    la.abs      t8, config_cache
    jirl        ra, t8, 0

    /******************************************************
     * jump to bsp_start
     */
    la.abs      t8, bsp_start
    jirl        ra, t8, 0

    /*
     * dead loop, NEVER go here!
     */
99:
    beq         zero, zero, 99b
    nop

    .end start

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */
 
