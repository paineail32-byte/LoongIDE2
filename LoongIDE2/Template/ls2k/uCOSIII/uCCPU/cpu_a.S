/*
*********************************************************************************************************
*                                               uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                    Copyright 2004-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            CPU PORT FILE
*
*                                           LOONGSON 2K300
*
* Filename : cpu_a.s
* Version  : V1.32.01
*********************************************************************************************************
*/

#include "asm.h"
#include "regdef.h"
#include "cpu.h"

/*
*********************************************************************************************************
*                                           PUBLIC FUNCTIONS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      CODE GENERATION DIRECTIVES
*********************************************************************************************************
*/

/*
**********************************************************************************************************
*                                      CRITICAL SECTION FUNCTIONS
*
* Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking, the
*               state of the interrupt disable flag is stored in the local variable 'cpu_sr' & interrupts
*               are then disabled ('cpu_sr' is allocated in all functions that need to disable interrupts).
*               The previous interrupt state is restored by copying 'cpu_sr' into the CPU's status register.
*
* Prototypes  : CPU_SR  CPU_SR_Save(void);
*               void    CPU_SR_Restore(CPU_SR cpu_sr);
**********************************************************************************************************
*/

    .text

LEAF(CPU_SR_Save)
    csrrd       v0, LA_CSR_CRMD
    li.d        t8, CSR_CRMD_IE                 /* disable interrupt */
    csrxchg     zero, t8, LA_CSR_CRMD
    jr          ra
    nop
    nop
END(CPU_SR_Save)

LEAF(CPU_SR_Restore)
    csrwr       a0, LA_CSR_CRMD
    jr          ra
    nop
    nop
END(CPU_SR_Restore)

/*
*********************************************************************************************************
*                                         CPU_CntLeadZeros()
*
* Description : Count the number of contiguous, most-significant, leading zero bits in a data value.
*
* Argument(s) : val         Data value to count leading zero bits.
*
* Return(s)   : Number of contiguous, most-significant, leading zero bits in 'val', if NO error(s).
*               0, otherwise.
*
* Note(s)     : (1) (a) Supports the following data value sizes :
*
*                       (1)  8-bits - N/A
*                       (2) 16-bits - N/A
*                       (3) 32-bits - N/A 
*                       (4) 64-bits - LA64 core (see LOONGARCH clz.d documentation)
*
*                       See also 'cpu_def.h  CPU WORD CONFIGURATION  Note #1'.
*
*
*                       (3) For 32-bit values :
*
*                             b31  b30  b29  ...  b04  b03  b02  b01  b00    # Leading Zeros
*                             ---  ---  ---       ---  ---  ---  ---  ---    ---------------
*                              1    x    x         x    x    x    x    x            0
*                              0    1    x         x    x    x    x    x            1
*                              0    0    1         x    x    x    x    x            2
*                              :    :    :         :    :    :    :    :            :
*                              :    :    :         :    :    :    :    :            :
*                              0    0    0         1    x    x    x    x           27
*                              0    0    0         0    1    x    x    x           28
*                              0    0    0         0    0    1    x    x           29
*                              0    0    0         0    0    0    1    x           30
*                              0    0    0         0    0    0    0    1           31
*                              0    0    0         0    0    0    0    0           32
*
*
*
* Note(s) : (1) This functions is used when CPU_CFG_LEAD_ZEROS_ASM_PRESENT is defined in cpu.h/cpu_cfg.h.
*               This function has been implemented to use the Count Leading Zeros (CLZ) instruction
*               present in the LA64 core.
*           (2) This function was located here because it is hardware dependant, and is part of the port.
*               The function prototype exists in cpu.h.  It is needed because this assembler routine is
*               called from C.
*           (3) See MIPS documentation for explanation of why the same GP register is used for source and
*               desitination of the clz instruction.
*********************************************************************************************************
*/

LEAF(CPU_CntLeadZeros)
    clz.d       v0, a0                          /* Count leading zeros in a0 passed into v0 */
    jr          ra
    nop
    nop
END(CPU_CntLeadZeros)

/*
*********************************************************************************************************
*                                         CPU_CntTrailZeros()
*
* Description : Count the number of contiguous, least-significant, trailing zero bits in a data value.
*
* Argument(s) : val         Data value to count trailing zero bits.
*
* Return(s)   : Number of contiguous, least-significant, trailing zero bits in 'val'.
*
* Note(s)     : (1) (a) Supports the following data value sizes :
*
*                       (1)  8-bits
*                       (2) 16-bits
*                       (3) 32-bits
*                       (4) 64-bits
*
*                       See also 'cpu_def.h  CPU WORD CONFIGURATION  Note #1'.
*
*                       For 64-bit values :
*
*                             b63  b62  b61  b60  b59  ...  b02  b01  b00    # Trailing Zeros
*                             ---  ---  ---  ---  ---       ---  ---  ---    ----------------
*                              x    x    x    x    x         x    x    1            0
*                              x    x    x    x    x         x    1    0            1
*                              x    x    x    x    x         1    0    0            2
*                              :    :    :    :    :         :    :    :            :
*                              :    :    :    :    :         :    :    :            :
*                              x    x    x    x    1         0    0    0           59
*                              x    x    x    1    0         0    0    0           60
*                              x    x    1    0    0         0    0    0           61
*                              x    1    0    0    0         0    0    0           62
*                              1    0    0    0    0         0    0    0           63
*                              0    0    0    0    0         0    0    0           64
*
*               (2) For non-zero values, the returned number of contiguous, least-significant, trailing
*                   zero bits is also equivalent to the bit position of the least-significant set bit.
*
*               (3) 'val' SHOULD be validated for non-'0' PRIOR to all other counting zero calculations :
*
*                   (a) CPU_CntTrailZeros()'s final conditional statement calculates 'val's number of
*                       trailing zeros based on its return data size, 'CPU_CFG_DATA_SIZE', & 'val's
*                       calculated number of lead zeros ONLY if the initial 'val' is non-'0' :
*
*                           if (val != 0u) {
*                               nbr_trail_zeros = ((CPU_CFG_DATA_SIZE * DEF_OCTET_NBR_BITS) - 1u) - nbr_lead_zeros;
*                           } else {
*                               nbr_trail_zeros = nbr_lead_zeros;
*                           }
*
*                       Therefore, initially validating all non-'0' values avoids having to conditionally
*                       execute the final 'if' statement.
*********************************************************************************************************
*/
LEAF(CPU_CntTrailZeros)
    ctz.d       v0, a0                          /* Count Trailing zeros in a0 passed into v0 */
    jr          ra
    nop
    nop
END(CPU_CntTrailZeros)

/*
 * @@ END
 */

