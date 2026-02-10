
#ifndef _RTEMS_LOONGARCH_LA_REGDEF_H
#define _RTEMS_LOONGARCH_LA_REGDEF_H

/******************************************************************************
 * Loongarch General registers
 */
#define zero    	$r0     /* wired zero */
#define ra      	$r1     /* return address */
#define tp      	$r2     /* global pointer */
#define gp      	$r2     /* == tp */
#define sp      	$r3     /* stack pointer */
#define v0      	$r4     /* return value - caller saved */
#define v1      	$r5
#define a0      	$r4     /* argument registers - SAME as v0/v1 */
#define a1      	$r5
#define a2      	$r6
#define a3      	$r7
#define a4      	$r8
#define a5      	$r9
#define a6      	$r10
#define a7      	$r11
#define t0      	$r12    /* caller saved */
#define t1      	$r13
#define t2      	$r14
#define t3      	$r15
#define t4      	$r16    /* callee saved */
#define t5      	$r17
#define t6      	$r18
#define t7      	$r19
#define t8      	$r20    /* caller saved */
#define x       	$r21
#define fp      	$r22    /* frame pointer */
#define s0      	$r23    /* callee saved */
#define s1      	$r24
#define s2      	$r25
#define s3      	$r26
#define s4      	$r27
#define s5      	$r28
#define s6      	$r29
#define s7      	$r30
#define s8      	$r31    /* callee saved */

/*******************************************************************************
 * Loongarch Float registers
 */
#define F0      	$f0
#define F1      	$f1
#define F2      	$f2
#define F3      	$f3
#define F4      	$f4
#define F5      	$f5
#define F6      	$f6
#define F7      	$f7
#define F8      	$f8
#define F9      	$f9
#define F10     	$f10
#define F11     	$f11
#define F12     	$f12
#define F13     	$f13
#define F14     	$f14
#define F15     	$f15
#define F16     	$f16
#define F17     	$f17
#define F18     	$f18
#define F19     	$f19
#define F20     	$f20
#define F21     	$f21
#define F22     	$f22
#define F23     	$f23
#define F24     	$f24
#define F25     	$f25
#define F26     	$f26
#define F27     	$f27
#define F28     	$f28
#define F29     	$f29
#define F30     	$f30
#define F31     	$f31

#define FCC0    	$fcc0
#define FCC1    	$fcc1
#define FCC2    	$fcc2
#define FCC3    	$fcc3
#define FCC4    	$fcc4
#define FCC5    	$fcc5
#define FCC6    	$fcc6
#define FCC7    	$fcc7
#define FCSR0   	$r0
#define FCSR1   	$r1
#define FCSR2   	$r2
#define FCSR3   	$r3

/******************************************************************************
 * For those who like to think in terms of the compiler names for the regs
 */

/******************************************************************************
 * Loongarch General Registers
 */
#define R_ZERO  	0
#define R_RA    	1
#define R_TP    	2
#define R_GP    	2           /* == R_TP */
#define R_SP    	3
#define R_A0    	4
#define R_A1    	5
#define R_A2    	6
#define R_A3    	7
#define R_A4    	8
#define R_A5    	9
#define R_A6    	10
#define R_A7    	11
#define R_T0    	12
#define R_T1    	13
#define R_T2    	14
#define R_T3    	15
#define R_T4    	16
#define R_T5    	17
#define R_T6    	18
#define R_T7    	19
#define R_T8    	20
#define R_X     	21
#define R_FP    	22
#define R_S0    	23
#define R_S1    	24
#define R_S2    	25
#define R_S3    	26
#define R_S4    	27
#define R_S5    	28
#define R_S6    	29
#define R_S7    	30
#define R_S8    	31

/* CSR Registers
 */
#define R_CRMD  	32          /* Thread Switch */
#define R_PRMD  	33          /* save these */
#define R_ECFG  	34          /* IP Enable */
#define R_ESTAT 	35          /* IP Status, ECODE */
#define R_EUEN  	36          /* FPU */
#define R_BADV	 	37			/* bad virtual address */
#define R_BADI 		38			/* bad instruction */
#define R_EPC   	39

/*
 * General + CSR
 */
#define GR_NUMS 	40

/******************************************************************************
 * Loongarch FPU Registers
 */
#define R_F0    	40
#define R_F1    	41
#define R_F2    	42
#define R_F3    	43
#define R_F4    	44
#define R_F5    	45
#define R_F6    	46
#define R_F7    	47
#define R_F8    	48
#define R_F9    	49
#define R_F10   	50
#define R_F11   	51
#define R_F12   	52
#define R_F13   	53
#define R_F14   	54
#define R_F15   	55
#define R_F16   	56
#define R_F17   	57
#define R_F18   	58
#define R_F19   	59
#define R_F20   	60
#define R_F21   	61
#define R_F22   	62
#define R_F23   	63
#define R_F24   	64
#define R_F25   	65
#define R_F26   	66
#define R_F27   	67
#define R_F28   	68
#define R_F29   	69
#define R_F30   	70
#define R_F31   	71
#define R_FCC0  	72          /* save these */
#define R_FCC1  	73
#define R_FCC2  	74
#define R_FCC3  	75
#define R_FCC4  	76
#define R_FCC5  	77
#define R_FCC6  	78
#define R_FCC7  	79
#define R_FCSR0 	80

#define FR_NUMS 	41

/*
 * Total Registers
 */
#define NREGS		81

#if 0
/**
 * CSR registers
 */
#define R_TLBIDX	81
#define R_TLBHI		82
#define R_TLBLO0	83
#define R_TLBLO1	84
#define R_ASID		85
#define R_PGDL		86
#define R_PGDH		87
#define R_PGD		88
#define R_PWCTL0	89
#define R_PWCTL1	90
#define R_STLBPS	91
#define R_TLDRBADV	92
#define R_TLBREPC	93
#define R_TLBTLO0	94
#define R_TLBRLO1	95
#define R_TLBRHI	96
#define R_TLBRPRMD	97
#define R_MERRINFO1	98
#define R_MERRINFO2	99
#define R_MERREPC	100
#define R_CTAG		101
#define R_PRID		102

#define CSR_REGS	22

/*
 * Total Registers
 */
#undef  NREGS
#define NREGS		103

#endif

#endif // #ifndef _RTEMS_LOONGARCH_LA_REGDEF_H
