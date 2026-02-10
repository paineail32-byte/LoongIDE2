/*
 * Copyright (C) 2020-2025 Suzhou Tiancheng Software Ltd.
 */
/*
 * cpu.h
 *
 * created: 2023-6-1
 *  author: Bian
 */

#ifndef _CPU_H
#define _CPU_H

/******************************************************************************
 * LoongArch32 CSR Registers
 */

/**
 * basic csr register
 */
#define LA_CSR_CRMD                 0x000       /* current mode */
#define CSR_CRMD_WE                 (1<<9)
#define CSR_CRMD_DATM_MASK			0x180		// bit[8:7]
#define CSR_CRMD_DATM_SHIFT			7
#define CSR_CRMD_DATF_MASK			0x60		// bit[6:5]
#define CSR_CRMD_DATF_SHIFT			5
#define CSR_CRMD_PG                 (1<<4)
#define CSR_CRMD_DA                 (1<<3)
#define CSR_CRMD_IE                 (1<<2)
#define CSR_CRMD_PLV_MASK           0x03

#define LA_CSR_PRMD                 0x001       /* Prev-exception mode info */
#define CSR_PRMD_PIE                (1<<2)
#define CSR_PRMD_PPLV_MASK          0x03

#define LA_CSR_EUEN                 0x002       /* Extended unit enable */
#define CSR_EUEN_FPEN               (1<<0)

#define LA_CSR_MISC                 0x003       /* Misc config */

#define LA_CSR_ECFG                 0x004       /* Exception config */
#define CSR_ECFG_IPI				(1<<12)		// 核间中断
#define CSR_ECFG_TI					(1<<11)		// 定时器中断
#define CSR_ECFG_HWI7				(1<<9)
#define CSR_ECFG_HWI6				(1<<8)
#define CSR_ECFG_HWI5				(1<<7)
#define CSR_ECFG_HWI4				(1<<6)
#define CSR_ECFG_HWI3				(1<<5)
#define CSR_ECFG_HWI2				(1<<4)
#define CSR_ECFG_HWI1				(1<<3)
#define CSR_ECFG_HWI0				(1<<2)
#define CSR_ECFG_SWI1				(1<<1)
#define CSR_ECFG_SWI0				(1<<0)
#define CSR_ECFG_IM_MASK            0x1FFF		// 0x1BFF

#define LA_CSR_ESTAT                0x005       /* Exception status */
#define CSR_ESTAT_ESUBCODE_SHIFT    22
#define CSR_ESTAT_ESUBCODE_MASK     (0x1FF<<22)
#define CSR_ESTAT_ECODE_SHIFT       16
#define CSR_ESTAT_ECODE_MASK        (0x3F<<16)
#define CSR_ESTAT_ECODE_INT			0x00000		// 中断
#define CSR_ESTAT_ECODE_PIL			0x10000		// load 操作页无效例外
#define CSR_ESTAT_ECODE_PIS			0x20000		// store 操作页无效例外
#define CSR_ESTAT_ECODE_PIF			0x30000		// 取指操作页无效例外
#define CSR_ESTAT_ECODE_PME			0x40000		// 页修改例外
#define CSR_ESTAT_ECODE_PPI			0x70000		// 页特权等级不合规例外
#define CSR_ESTAT_ECODE_ADEF		0x80000		// 取指地址错例外
#define CSR_ESTAT_ECODE_ADEM		0x480000	// 访存指令地址错例外
#define CSR_ESTAT_ECODE_ALE			0x90000		// 地址非对齐例外
#define CSR_ESTAT_ECODE_SYS			0xB0000		// 系统调用例外
#define CSR_ESTAT_ECODE_BREAK		0xC0000		// 断点例外
#define CSR_ESTAT_ECODE_INE			0xD0000		// 指令不存在例外
#define CSR_ESTAT_ECODE_IPE			0xE0000		// 指令特权等级错例外
#define CSR_ESTAT_ECODE_FPD			0xF0000		// 浮点指令未使能例外
#define CSR_ESTAT_ECODE_FPE			0x120000	// 基础浮点指令例外
#define CSR_ESTAT_ECODE_TLBR		0x3F0000	// TLB 重填例外
#define CSR_ESTAT_IPI				(1<<12)		// 核间中断
#define CSR_ESTAT_TI				(1<<11)		// 定时器中断
#define CSR_ESTAT_HWI7				(1<<9)
#define CSR_ESTAT_HWI6				(1<<8)
#define CSR_ESTAT_HWI5				(1<<7)
#define CSR_ESTAT_HWI4				(1<<6)
#define CSR_ESTAT_HWI3				(1<<5)
#define CSR_ESTAT_HWI2				(1<<4)
#define CSR_ESTAT_HWI1				(1<<3)
#define CSR_ESTAT_HWI0				(1<<2)
#define CSR_ESTAT_SWI1				(1<<1)
#define CSR_ESTAT_SWI0				(1<<0)
#define CSR_ESTAT_IS_MASK           0x1FFF

/*
 * CSR_ECFG_IM/CSR_ESTAT_IS位见下面定义.
 */

#define LA_CSR_ERA                  0x006       /* EPC */

#define LA_CSR_BADV                 0x007       /* Bad virtual address */

#define LA_CSR_EBASE                0x00C       /* Exception entry base address */
#define CSR_EBASE_MASK              (~0x3F)     /* bit[31:6]*/

/**
 * TLB related CSR registers
 */
#define LA_CSR_TLBIDX               0x010       /* TLB Index, EHINV, PageSize, NP */
#define CSR_TLBIDX_EHINV            (1<<31)
#define CSR_TLBIDX_PS_SHIFT         24
#define CSR_TLBIDX_PS_MASK          (0x3F<<24)
#define CSR_TLBIDX_IDX_MASK         (0x0FFF)

#define LA_CSR_TLBEHI               0x011       /* TLB EntryHi */
#define LA_CSR_TLBELO0              0x012       /* TLB EntryLo0 */
#define LA_CSR_TLBELO1              0x013       /* TLB EntryLo1 */
#define LA_CSR_ASID                 0x018       /* ASID */
#define LA_CSR_PGDL                 0x019       /* Page table base address when VA[47] = 0 */
#define LA_CSR_PGDH                 0x01A       /* Page table base address when VA[47] = 1 */
#define LA_CSR_PGD                  0x01B       /* Page table base */

/**
 * Config CSR registers
 */
#define LA_CSR_CPUNUM               0x020       /* CPU core number */
#define CSR_CPUNUM_CID_MASK         (0x1FF)

/**
 * Kscratch registers
 */
#define LA_CSR_SAVE0                0x030
#define LA_CSR_SAVE1                0x031

/**
 * Timer registers
 */
#define LA_CSR_TMID                 0x040       /* Timer ID */
#define LA_CSR_TCFG                 0x041       /* Timer config */
#define CSR_TCFG_VAL_SHIFT          2
#define CSR_TCFG_PERIOD             (1<<1)
#define CSR_TCFG_EN                 (1<<0)

#define LA_CSR_TVAL                 0x042       /* Timer value */

#define LA_CSR_CNTC                 0x043       /* Timer offset */

#define LA_CSR_TINTCLR              0x044       /* Timer interrupt clear */
#define CSR_TINTCLR_TI              (1<<0)

/**
 * LLBCTL register
 */
#define LA_CSR_LLBCTL               0x060       /* LLBit control */
#define CSR_LLBCTL_ROLLB            (1<<0)
#define CSR_LLBCTL_WCLLB            (1<<1)
#define CSR_LLBCTL_KLO              (1<<2)

/**
 * TLB refill registers
 */
#define LA_CSR_TLBREBASE            0x088       /* TLB refill exception base address */
#define CSR_TLBREBASE_MASK          (~0x3F)     /* bit[31:6]*/

#define LA_CSR_CTAG                 0x098       /* TagLo + TagHi */

/**
 * Direct map windows registers
 */
#define LA_CSR_DMW0                 0x180
#define LA_CSR_DMW1                 0x181

/**
 * Direct map window 0/1
 */
#define CSR_DMW_PLV0                (1<<0)
#define CSR_DMW_PLV3                (1<<3)
#define CSR_DMW_MAT_MASK            (0x03<<4) 	// 虚地址落在该映射窗口下访存操作的存储访问类型
#define CSR_DMW_MAT_SHIFT			4
#define CSR_DMW_PSEG_MASK			(0x07<<25)	// 直接映射窗口的物理地址的 [31:29] 位
#define CSR_DMW_PSEG_SHIFT			25
#define CSR_DMW_VSEG_MASK			(0x07<<29)	// 直接映射窗口的虚地址的 [31:29] 位
#define CSR_DMW_VSEG_SHIFT			29

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * Debug registers
 */
#define LA_CSR_MWPC                 0x300       /* data breakpoint config */
#define LA_CSR_MWPS                 0x301       /* data breakpoint status */

#define LA_CSR_FWPC                 0x380       /* instruction breakpoint config */
#define LA_CSR_FWPS                 0x381       /* instruction breakpoint status */

#define LA_CSR_DEBUG                0x500       /* debug config */
#define LA_CSR_DEPC                 0x501       /* debug epc */
#define LA_CSR_DSAVE                0x502       /* debug save */

//-----------------------------------------------------------------------------
// larchintrin.h
//-----------------------------------------------------------------------------

#ifndef __ASSEMBLER__

#define set_csr_estat(val)   \
	__csrxchg_w(val, val, LA_CSR_ESTAT)
#define clear_csr_estat(val) \
	__csrxchg_w(~(val), val, LA_CSR_ESTAT)
 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * 开中断
 */
#define loongarch_interrupt_enable() \
    __csrxchg_w(CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD)

/**
 * 关中断
 */
#define loongarch_interrupt_disable() \
    __csrxchg_w(~CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD)

//-----------------------------------------------------------------------------
// Loongarch Critial
//-----------------------------------------------------------------------------

#define loongarch_critical_enter() \
    unsigned int rv_CRMD = __csrrd_w(LA_CSR_CRMD); \
    __csrxchg_w(~CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD);

#define loongarch_critical_exit() \
    if (rv_CRMD & CSR_CRMD_IE)    \
        __csrxchg_w(CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD);

//-----------------------------------------------------------------------------
// ELF Macro
//-----------------------------------------------------------------------------

#endif // #ifndef __ASSEMBLER__

#endif // _CPU_H

/*
 * @@ END
 */

