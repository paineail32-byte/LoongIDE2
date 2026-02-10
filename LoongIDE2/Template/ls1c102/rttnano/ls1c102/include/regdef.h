/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
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
#define R_ERA   33

#define R_CRMD  34          /* Thread Switch */
#define R_EUEN  35          /* FPU */
#define R_ECFG  36          /* IP Enable */
#define R_ESTAT 37          /* IP Status, ECODE */

/*
 * General + CSR
 */
#define GR_NUMS 38

#endif // _REGDEF_H

