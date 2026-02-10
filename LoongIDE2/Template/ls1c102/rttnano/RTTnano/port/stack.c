/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

/*
 * stack.c
 *
 * created: 2022/06/21
 *  author: Bian
 */

#include <string.h>
#include <larchintrin.h>

#include "cpu.h"

#include "rtthread.h"
#include "rthw.h"

#include "context.h"

//-----------------------------------------------------------------------------

register unsigned int $GP __asm__ ("$r2");

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    static unsigned int crmd = 0, ecfg, __gp;
    unsigned int *stk;

    if (crmd == 0)
    {
        crmd  = __csrrd_w(LA_CSR_CRMD);
        crmd |= CSR_CRMD_IE; // CSR_CRMD_WE
        ecfg  = __csrrd_w(LA_CSR_ECFG);
        __gp = $GP;
    }

    /*
     * Get stack aligned
     */
#ifndef ARCH_CPU_STACK_GROWS_UPWARD
    stack_addr -= 0x10;	// PAD is needed
    stk = (unsigned int *)RT_ALIGN_DOWN((unsigned int)stack_addr, 4);
    stk -= CTX_SIZE / 4;
#else
#error "TODO stack in context.h"
#endif

#ifndef RT_USING_OVERFLOW_CHECK
    for (int i=0; i < 32; i++)
    {
        stk[i] = ((unsigned int)(i & 0xFF) << 24) |
                 ((unsigned int)(i & 0xFF) << 16) |
                 ((unsigned int)(i & 0xFF) << 8 ) |
                 ((unsigned int)(i & 0xFF) << 0 );
    }
#endif
 
    *(stk + R_SP   ) = (unsigned int)stk;
    *(stk + R_A0   ) = (unsigned int)parameter;
    *(stk + R_RA   ) = (unsigned int)texit;

    *(stk + R_ESTAT) = 0;
    *(stk + R_CRMD ) = crmd;
    *(stk + R_GP   ) = __gp;
    *(stk + R_ECFG ) = ecfg;

    *(stk + R_PRMD ) = CSR_PRMD_PIE;
    *(stk + R_ERA  ) = (unsigned int)tentry;

    return (rt_uint8_t *)stk;
}

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
