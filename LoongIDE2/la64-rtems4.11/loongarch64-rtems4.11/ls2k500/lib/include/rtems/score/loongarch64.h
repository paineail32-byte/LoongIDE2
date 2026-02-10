
#ifndef _RTEMS_SCORE_LOONGARCH_H
#define _RTEMS_SCORE_LOONGARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ASM
#include <larchintrin.h>
#include <rtems/loongarch64/la64cpu.h>
#endif

/*
 * IE bits that enable/disable interrupts
 */

#ifdef ASM
#define LOONGARCH_IE_BITS	0x04
#else
#define LOONGARCH_IE_BITS	CSR_CRMD_IE
#endif

/*
 *  This file contains the information required to build
 *  RTEMS for a particular member of the "no cpu"
 *  family when executing in protected mode.  It does
 *  this by setting variables to indicate which implementation
 *  dependent features are present in a particular member
 *  of the family.
 */
#if __loongarch_hard_float
#define LOONGARCH_HAS_FPU 	1			// 先关闭FPU
#else
#define LOONGARCH_HAS_FPU 	0
#endif

#if __loongarch64
#define CPU_MODEL_NAME  "ISA Level 64"
#else
#define CPU_MODEL_NAME  "ISA Level 32"
#endif

/*
 * Define the name of the CPU family.
 */
#define CPU_NAME "LOONGARCH"

/*
 * RTEMS Vector numbers for exception conditions.
 * This is a direct map to the estat.
 */

#define LOONGARCH_EXCEPTION_BASE 			0

#define LOONGARCH_EXCEPTION_INT            	LOONGARCH_EXCEPTION_BASE+0      // 仅当CSR.ECFG.VS=0时, 表示是中断
#define LOONGARCH_EXCEPTION_PIL            	LOONGARCH_EXCEPTION_BASE+1      // load  操作页无效例外
#define LOONGARCH_EXCEPTION_PIS             LOONGARCH_EXCEPTION_BASE+2      // store 操作页无效例外
#define LOONGARCH_EXCEPTION_PIF             LOONGARCH_EXCEPTION_BASE+3      // 取指操作页无效例外
#define LOONGARCH_EXCEPTION_PME             LOONGARCH_EXCEPTION_BASE+4      // 页修改例外
#define LOONGARCH_EXCEPTION_PNR             LOONGARCH_EXCEPTION_BASE+5      // 页不可读例外
#define LOONGARCH_EXCEPTION_PNX             LOONGARCH_EXCEPTION_BASE+6      // 页不可执行例外
#define LOONGARCH_EXCEPTION_PPI             LOONGARCH_EXCEPTION_BASE+7      // 页特权等级不合规例外
#define LOONGARCH_EXCEPTION_ADEF_ADEM       LOONGARCH_EXCEPTION_BASE+8      // subcode=0: 取指地址错例外/subcode=1: 访存指令地址错例外
#define LOONGARCH_EXCEPTION_ALE             LOONGARCH_EXCEPTION_BASE+9      // 地址非对齐例外
#define LOONGARCH_EXCEPTION_BCE             LOONGARCH_EXCEPTION_BASE+10     // 边界约束检查错例外
#define LOONGARCH_EXCEPTION_SYSCALL         LOONGARCH_EXCEPTION_BASE+11     // 系统调用例外
#define LOONGARCH_EXCEPTION_BREAK           LOONGARCH_EXCEPTION_BASE+12     // 断点例外
#define LOONGARCH_EXCEPTION_INE             LOONGARCH_EXCEPTION_BASE+13     // 指令不存在例外
#define LOONGARCH_EXCEPTION_IPE             LOONGARCH_EXCEPTION_BASE+14     // 指令特权等级错例外
#define LOONGARCH_EXCEPTION_FPD             LOONGARCH_EXCEPTION_BASE+15     // 浮点指令未使能例外
#define LOONGARCH_EXCEPTION_SXD             LOONGARCH_EXCEPTION_BASE+16     // 128位向量扩展指令未使能例外
#define LOONGARCH_EXCEPTION_ASXD            LOONGARCH_EXCEPTION_BASE+17     // 256位向量扩展指令未使能例外
#define LOONGARCH_EXCEPTION_FPE_VFPE        LOONGARCH_EXCEPTION_BASE+18     // subcode=0: 基础浮点指令例外/subcode=1: 向量浮点指令例外
#define LOONGARCH_EXCEPTION_WPEF_WPEM       LOONGARCH_EXCEPTION_BASE+19     // subcode=0: 取指监测点例外/subcode=1: load/store操作监测点例外
#define LOONGARCH_EXCEPTION_BTD             LOONGARCH_EXCEPTION_BASE+20     // 二进制翻译扩展指令未使能例外
#define LOONGARCH_EXCEPTION_BTE             LOONGARCH_EXCEPTION_BASE+21     // 二进制翻译相关例外
#define LOONGARCH_EXCEPTION_GSPR            LOONGARCH_EXCEPTION_BASE+22     // 客户机敏感特权资源例外
#define LOONGARCH_EXCEPTION_HVC             LOONGARCH_EXCEPTION_BASE+23     // 虚拟机监控调用例外
#define LOONGARCH_EXCEPTION_GCSC_GCHC       LOONGARCH_EXCEPTION_BASE+24     // subcode=0: 客户机CSR软件修改例外/ subcode=1: 客户机CSR硬件修改例外

/* 0x1A-0x3E 保留  */

#define LOONGARCH_INTERRUPT_BASE   			LOONGARCH_EXCEPTION_BASE+32		// +32

/**************************************************************************************************
 *  Some macros to access registers
 */

/*
 * CRMD寄存器: 全局IE位bit[2]
 */
#define loongarch_get_crmd(_x) 		do { _x = __csrrd_d(LA_CSR_CRMD); } while (0)
#define loongarch_set_crmd(_x)  	do { __csrwr_d(_x, LA_CSR_CRMD); } while (0)

/*
 * ECFG寄存器: 中断屏蔽掩码 0x1FFF
 */
#define loongarch_get_ecfg(_x)  	do { _x = __csrrd_d(LA_CSR_ECFG); } while (0)
#define loongarch_set_ecfg(_x) 		do { __csrwr_d(_x, LA_CSR_ECFG); } while (0)

/*
 * ESTAT寄存器: 中断状态掩码 0x1FFF
 */
#define loongarch_get_estat(_x) 	do { _x = __csrrd_d(LA_CSR_ESTAT); } while (0)
#define loongarch_set_estat(_x) 	do { __csrwr_d(_x, LA_CSR_ESTAT); } while (0)

/**************************************************************************************************
 *  Access FCSR0
 */

#if (LOONGARCH_HAS_FPU == 1)
#define loongarch_get_fcsr0(_x) 	do { _x = __movfcsr2gr($r0); } while(0)
#define loongarch_set_fcsr0(_x) 	do { __movgr2fcsr($r0, _x); } while(0)
#else
#define loongarch_get_fcsr0(_x)
#define loongarch_set_fcsr0(_x)
#endif

/*
 *  Manipulate interrupt mask
 *
 *  loongarch_unmask_interrupt(_mask)
 *    enables interrupts - mask is positioned so it only needs to be or'ed
 *    into the ECFG reg. This also does some other things !!!! Caution
 *    should be used if invoking this while in the middle of a debugging
 *    session where the client may have nested interrupts.
 *
 *  loongarch_mask_interrupt(_mask)
 *    disable the interrupt - mask is the complement of the bits to be
 *    cleared.
 *
 *
 *  NOTE: loongarch_mask_interrupt() used to be disable_int().
 *        loongarch_unmask_interrupt() used to be enable_int().
 *
 */
#define loongarch_unmask_interrupt(_mask)   do { __csrxchg_d(~0ul, _mask, LA_CSR_ECFG); } while (0)
#define loongarch_mask_interrupt(_mask)  	do { __csrxchg_d(0ul, _mask, LA_CSR_ECFG); } while (0)

#ifdef __cplusplus
}
#endif

#endif // #ifndef _RTEMS_SCORE_LOONGARCH_H

