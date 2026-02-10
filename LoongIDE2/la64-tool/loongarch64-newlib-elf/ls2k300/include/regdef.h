/*
 * Copyright (C) 2020-2022 Suzhou Tiancheng Software Ltd.
 */
/*
 * regdef.h
 *
 * created: 2021-12-24
 *  author: Bian
 */

#ifndef _REGDEF_H
#define _REGDEF_H

/**
 * General registers
 */
#define zero    $r0     /* wired zero */
#define ra      $r1     /* return address */
#define tp      $r2     /* global pointer */
#define gp      $r2     /* == tp */
#define sp      $r3     /* stack pointer */
#define v0      $r4     /* return value - caller saved */
#define v1      $r5
#define a0      $r4     /* argument registers - SAME as v0/v1 */
#define a1      $r5
#define a2      $r6
#define a3      $r7
#define a4      $r8
#define a5      $r9
#define a6      $r10
#define a7      $r11
#define t0      $r12    /* caller saved */
#define t1      $r13
#define t2      $r14
#define t3      $r15
#define t4      $r16    /* callee saved */
#define t5      $r17
#define t6      $r18
#define t7      $r19
#define t8      $r20    /* caller saved */
#define x       $r21
#define fp      $r22    /* frame pointer */
#define s0      $r23    /* callee saved */
#define s1      $r24
#define s2      $r25
#define s3      $r26
#define s4      $r27
#define s5      $r28
#define s6      $r29
#define s7      $r30
#define s8      $r31    /* callee saved */

/**
 * Float registers
 */
#define fa0     $f0
#define fa1     $f1
#define fa2     $f2
#define fa3     $f3
#define fa4     $f4
#define fa5     $f5
#define fa6     $f6
#define fa7     $f7

#define fv0     fa0
#define fv1     fa1

#define ft0     $f8
#define ft1     $f9
#define ft2     $f10
#define ft3     $f11
#define ft4     $f12
#define ft5     $f13
#define ft6     $f14
#define ft7     $f15
#define ft8     $f16
#define ft9     $f17
#define ft10    $f18
#define ft11    $f19
#define ft12    $f20
#define ft13    $f21
#define ft14    $f22
#define ft15    $f23

#define fs0     $f24
#define fs1     $f25
#define fs2     $f26
#define fs3     $f27
#define fs4     $f28
#define fs5     $f29
#define fs6     $f30
#define fs7     $f31

#if 0
#define fcc0    $fcc0
#define fcc1    $fcc1
#define fcc2    $fcc2
#define fcc3    $fcc3
#define fcc4    $fcc4
#define fcc5    $fcc5
#define fcc6    $fcc6
#define fcc7    $fcc7
#endif

#define fcsr0   $r0
#define fcsr1   $r1
#define fcsr2   $r2
#define fcsr3   $r3

#define fcsr    fcsr0

/*
 * For those who like to think in terms of the compiler names for the regs
 */
/* General Registers
 */
#define R_ZERO  0
#define R_RA    1
#define R_TP    2
#define R_GP    2           /* == R_TP */
#define R_SP    3
#define R_A0    4
#define R_A1    5
#define R_A2    6
#define R_A3    7
#define R_A4    8
#define R_A5    9
#define R_A6    10
#define R_A7    11
#define R_T0    12
#define R_T1    13
#define R_T2    14
#define R_T3    15
#define R_T4    16
#define R_T5    17
#define R_T6    18
#define R_T7    19
#define R_T8    20
#define R_X     21
#define R_FP    22
#define R_S0    23
#define R_S1    24
#define R_S2    25
#define R_S3    26
#define R_S4    27
#define R_S5    28
#define R_S6    29
#define R_S7    30
#define R_S8    31

/* CSR Registers
 */
#define R_PRMD  32          /* save these */
#define R_EPC   33

#define R_CRMD  34          /* Thread Switch */
#define R_EUEN  35          /* FPU */
#define R_ECFG  36          /* IP Enable */
#define R_ESTAT 37          /* IP Status, ECODE */

/*
 * General + CSR
 */
#define GR_NUMS 38			/* Total General registers */

/* FPU Registers
 */
#define R_F0    0
#define R_F1    1
#define R_F2    2
#define R_F3    3
#define R_F4    4
#define R_F5    5
#define R_F6    6
#define R_F7    7
#define R_F8    8
#define R_F9    9
#define R_F10   10
#define R_F11   11
#define R_F12   12
#define R_F13   13
#define R_F14   14
#define R_F15   15
#define R_F16   16
#define R_F17   17
#define R_F18   18
#define R_F19   19
#define R_F20   20
#define R_F21   21
#define R_F22   22
#define R_F23   23
#define R_F24   24
#define R_F25   25
#define R_F26   26
#define R_F27   27
#define R_F28   28
#define R_F29   29
#define R_F30   30
#define R_F31   31

#define R_FCC0  32          /* save these */
#define R_FCC1  33
#define R_FCC2  34
#define R_FCC3  35
#define R_FCC4  36
#define R_FCC5  37
#define R_FCC6  38
#define R_FCC7  39

#define R_FCSR0 40
#define R_FCSR  R_FCSR0

#define FR_NUMS 41			/* Total FPU registers */

#endif // _REGDEF_H

