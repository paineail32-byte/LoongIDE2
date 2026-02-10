/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * stack.c
 *
 * created: 2024-7-16
 *  author: Bian
 */

#include <string.h>

#include <larchintrin.h>
#include "cpu.h"

#include "rtthread.h"
#include "rthw.h"

#include "context.h"

//-----------------------------------------------------------------------------

register unsigned long $GP __asm__ ("$r2");

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    static unsigned long crmd=0, ecfg=0, _gp=0;
    unsigned long *stk;

    if (crmd == 0)
    {
        crmd  = __csrrd_d(LA_CSR_CRMD);
        crmd |= CSR_CRMD_WE | CSR_CRMD_IE;
        ecfg  = __csrrd_d(LA_CSR_ECFG);
        _gp   = $GP;
    }

    /*
     * Get stack aligned
     */
#ifndef ARCH_CPU_STACK_GROWS_UPWARD
    stk = (unsigned long *)RT_ALIGN_DOWN((unsigned long)stack_addr, 8);
    stk -= CTX_SIZE / 8;
#else
#error "TODO stack in context.h"
#endif

    memset((void *)stk, 0, CTX_SIZE);       /* Clear the stack */

/*
    for (int i=0; i < 32; i++)
    {
        stk[i] = ((unsigned long)(i & 0xFF) << 56) |
                 ((unsigned long)(i & 0xFF) << 48) |
                 ((unsigned long)(i & 0xFF) << 40) |
                 ((unsigned long)(i & 0xFF) << 32) |
                 ((unsigned long)(i & 0xFF) << 24) |
                 ((unsigned long)(i & 0xFF) << 16) |
                 ((unsigned long)(i & 0xFF) << 8 ) |
                 ((unsigned long)(i & 0xFF) << 0 );
    }
 */

    *(stk + R_SP   ) = (unsigned long)stk;
    *(stk + R_A0   ) = (unsigned long)parameter;
    *(stk + R_RA   ) = (unsigned long)texit;

    *(stk + R_ESTAT) = 0;
    *(stk + R_CRMD)  = crmd;
    *(stk + R_GP  )  = _gp;
    *(stk + R_ECFG)  = ecfg;

    *(stk + R_PRMD)  = CSR_PRMD_PWE | CSR_PRMD_PIE;
    *(stk + R_EPC )  = (unsigned long)tentry;

    return (rt_uint8_t *)stk;
}

//-----------------------------------------------------------------------------

