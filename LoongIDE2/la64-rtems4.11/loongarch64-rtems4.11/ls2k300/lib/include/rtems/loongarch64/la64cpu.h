
#ifndef _RTEMS_LOONGARCH_LA_CPU_H
#define _RTEMS_LOONGARCH_LA_CPU_H

/**************************************************************************************************
 * LoongArch Memory Address
 */
#define KUSEG_ADDR					0x0
#define CACHED_MEMORY_ADDR			0x9000000000000000
#define UNCACHED_MEMORY_ADDR		0x8000000000000000
#define MAX_MEM_ADDR				PHYS_TO_UNCACHED(0x1e000000)
#define	RESERVED_ADDR				PHYS_TO_UNCACHED(0x1fc80000)
#define IS_CACHED_ADDR(x)			(!!(((x) & 0xff00000000000000ULL) == CACHED_MEMORY_ADDR))

#define CACHED_TO_PHYS(x)			VA_TO_PHYS(x)
#define UNCACHED_TO_PHYS(x)			VA_TO_PHYS(x)

#define	PHYS_TO_CACHED(x)			((unsigned long)(x) | CACHED_MEMORY_ADDR)
#define	PHYS_TO_UNCACHED(x) 		((unsigned long)(x) | UNCACHED_MEMORY_ADDR)
#define	CACHED_TO_UNCACHED(x)		(PHYS_TO_UNCACHED(VA_TO_PHYS(x)))
#define UNCACHED_TO_CACHED(x)		(PHYS_TO_CACHED(VA_TO_PHYS(x)))
#define VA_TO_PHYS(x)				((unsigned long)(x) & 0xffffffffULL)

/**************************************************************************************************
 * LoongArch CSR Registers
 */

/**
 * basic csr register
 */
#define LA_CSR_CRMD                 0x000       /* current mode */
#define CSR_CRMD_WE                 (1<<9)
#define CSR_CRMD_PG                 (1<<4)
#define CSR_CRMD_DA                 (1<<3)
#define CSR_CRMD_IE                 (1<<2)
#define CSR_CRMD_PLV_MASK           0x03

#define LA_CSR_PRMD                 0x001       /* Prev-exception mode info */
#define CSR_PRMD_PWE                (1<<3)
#define CSR_PRMD_PIE                (1<<2)
#define CSR_PRMD_PPLV_MASK          0x03

#define LA_CSR_EUEN                 0x002       /* Extended unit enable */
#define CSR_EUEN_LBTEN              (1<<3)
#define CSR_EUEN_LASXEN             (1<<2)
#define CSR_EUEN_LSXEN              (1<<1)
#define CSR_EUEN_FPEN               (1<<0)

#define LA_CSR_MISC                 0x003       /* Misc config */

#define LA_CSR_ECFG                 0x004       /* Exception config */
#define CSR_ECFG_VS_SHIFT           16
#define CSR_ECFG_VS_MASK            0x07
#define CSR_ECFG_VS                 (0x07<<16)
#define CSR_ECFG_IM_MASK            0x1FFF

#define LA_CSR_ESTAT                0x005       /* Exception status */
#define CSR_ESTAT_ESUBCODE_SHIFT    22
#define CSR_ESTAT_ESUBCODE_MASK     (0x1FF<<22)
#define CSR_ESTAT_EXC_SHIFT         16
#define CSR_ESTAT_EXC_MASK          (0x3F<<16)
#define CSR_ESTAT_IS_MASK           0x1FFF

/*
 * CSR_ECFG_IM/CSR_ESTAT_IS位见下面定义: 658行
 */
#define LA_CSR_EPC                  0x006       /* EPC */
#define LA_CSR_BADV                 0x007       /* Bad virtual address */
#define LA_CSR_BADI                 0x008       /* Bad instruction */
#define LA_CSR_EBASE                0x00C       /* Exception entry base address */

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
#define LA_CSR_GTLBC                0x015       /* Guest TLB control */
#define LA_CSR_TRGP                 0x016       /* TLBR read guest info */
#define LA_CSR_ASID                 0x018       /* ASID */
#define LA_CSR_PGDL                 0x019       /* Page table base address when VA[47] = 0 */
#define LA_CSR_PGDH                 0x01A       /* Page table base address when VA[47] = 1 */
#define LA_CSR_PGD                  0x01B       /* Page table base */
#define LA_CSR_PWCTL0               0x01C       /* PWCtl0 */
#define LA_CSR_PWCTL1               0x01D       /* PWCtl1 */
#define LA_CSR_STLBPGSIZE           0x01E
#define LA_CSR_RVACFG               0x01F

/**
 * Config CSR registers
 */
#define LA_CSR_CPUNUM               0x020       /* CPU core number */
#define CSR_CPUNUM_CID_MASK         (0x1FF)

#define LA_CSR_PRCFG1               0x021       /* Config1 */
#define CSR_CONF1_VSMAX_SHIFT       12
#define CSR_CONF1_VSMAX_MASK        (0x07<<12)
#define CSR_CONF1_TMRBITS_SHIFT     4
#define CSR_CONF1_TMRBITS_MASK      (0xFF<<4)
#define CSR_CONF1_KSNUM_MASK        (0x0F)

#define LA_CSR_PRCFG2               0x022       /* Config2 */
#define CSR_CONF2_PGMASK_SUPP       0x3FFFF000

#define LA_CSR_PRCFG3               0x023       /* Config3 */
#define CSR_CONF3_STLBIDX_SHIFT     20
#define CSR_CONF3_STLBIDX_MASK      (0x3F<<20)
#define CSR_CONF3_STLBWAYS_SHIFT    12
#define CSR_CONF3_STLBWAYS_MASK     (0xFF<<12)
#define CSR_CONF3_MTLBSIZE_SHIFT    4
#define CSR_CONF3_MTLBSIZE_MASK     (0xFF<<4)
#define CSR_CONF3_TLBTYPE_MASK      (0x0F)

/**
 * Kscratch registers
 */
#define LA_CSR_KS0                  0x030
#define LA_CSR_KS1                  0x031
#define LA_CSR_KS2                  0x032
#define LA_CSR_KS3                  0x033

#if 0
#define LA_CSR_KS4                  0x034
#define LA_CSR_KS5                  0x035
#define LA_CSR_KS6                  0x036
#define LA_CSR_KS7                  0x037
#define LA_CSR_KS8                  0x038

/* TLB exception allocated KS0 and KS1 statically
 */
#define TLB_EXC_KS0                 LA_CSR_KS0
#define TLB_EXC_KS1                 LA_CSR_KS1
#define TLB_KSCRATCH_MASK           (1<<0 | 1<<1 | 1<<2)

/* Percpu allocated KS3
 */
#define PERCPU_BASE_KS              LA_CSR_KS3
#define PERCPU_BASE_MASK            (1<<3)

/* KVM allocated KS4 and KS5 statically
 */
#define KVM_VCPU_KS                 LA_CSR_KS4
#define KVM_TEMP_KS                 LA_CSR_KS5
#define KVM_KSCRATCH_MASK           (1<<4 | 1<<5)
#endif

/**
 * Timer registers
 */
#define LA_CSR_TMID                 0x040       /* Timer ID */
#define LA_CSR_TCFG                 0x041       /* Timer config */
#define CSR_TCFG_VAL_SHIFT          2
#define CSR_TCFG_VAL_WIDTH          48
#define CSR_TCFG_VAL_MASK           (0x3fffffffffff<<2)
#define CSR_TCFG_PERIOD             (1<<1)
#define CSR_TCFG_EN                 (1<<0)
#define LA_CSR_TVAL                 0x042       /* Timer value */
#define LA_CSR_CNTC                 0x043       /* Timer offset */
#define LA_CSR_TINTCLR              0x044       /* Timer interrupt clear */
#define CSR_TINTCLR_TI              (1<<0)

/**
 * Guest registers
 */
#define LA_CSR_GSTAT                0x050       /* Guest status */
#define CSR_GSTAT_GID_SHIFT         16
#define CSR_GSTAT_GID_MASK          (0xFF<<16)
#define CSR_GSTAT_GIDBIT_SHIFT      4
#define CSR_GSTAT_GIDBIT_MASK       (0x3F<<4)
#define CSR_GSTAT_PVM               (1<<1)
#define CSR_GSTAT_VM                (1<<0)

#define LA_CSR_GCFG                 0x051       /* Guest config */
#define CSR_GCFG_GPERF_SHIFT        24
#define CSR_GCFG_GPERF_MASK         (0x07<<24)
#define CSR_GCFG_GCI_SHIFT          20
#define CSR_GCFG_GCI_MASK           (0x03<<20)
#define CSR_GCFG_GCI_ALL            (0x00<<20)
#define CSR_GCFG_GCI_HIT            (0x01<<20)
#define CSR_GCFG_GCI_SECURE         (0x02<<20)
#define CSR_GCFG_GCIP_SHIFT         16
#define CSR_GCFG_GCIP_MASK          (0x0F<<16)
#define CSR_GCFG_GCIP_ALL           (1<<16)
#define CSR_GCFG_GCIP_HIT           (1<<17)
#define CSR_GCFG_GCIP_SECURE        (1<<18)

#define CSR_GCFG_TORU               (1<<15)
#define CSR_GCFG_TORUP              (1<<14)
#define CSR_GCFG_TOP                (1<<13)
#define CSR_GCFG_TOPP               (1<<12)
#define CSR_GCFG_TOE                (1<<11)
#define CSR_GCFG_TOEP               (1<<10)
#define CSR_GCFG_TIT                (1<<9)
#define CSR_GCFG_TITP               (1<<8)
#define CSR_GCFG_SIT                (1<<7)
#define CSR_GCFG_SITP               (1<<6)
#define CSR_GCFG_MATC_SHITF         4
#define CSR_GCFG_MATC_MASK          (0x03<<4)
#define CSR_GCFG_MATC_GUEST         (0x00<<4)
#define CSR_GCFG_MATC_ROOT          (0x01<<4)
#define CSR_GCFG_MATC_NEST          (0x02<<4)

#define LA_CSR_GINTC                0x052       /* Guest interrupt control */
#define CSR_GINTC_HC_SHIFT          16
#define CSR_GINTC_HC_MASK           (0xFF<<16)
#define CSR_GINTC_PIP_SHIFT         8
#define CSR_GINTC_PIP_MASK          (0xFF<<8)
#define CSR_GINTC_VIP_MASK          (0xFF<<0)

#define LA_CSR_GCNTC                0x053       /* Guest timer offset */

/**
 * LLBCTL register
 */
#define LA_CSR_LLBCTL               0x060       /* LLBit control */
#define CSR_LLBCTL_ROLLB            (1<<0)
#define CSR_LLBCTL_WCLLB            (1<<1)
#define CSR_LLBCTL_KLO              (1<<2)

/**
 * Implement dependent
 */
#define LA_CSR_IMPCTL1              0x080       /* Loongson config1 */
#define CSR_MISPEC_SHIFT            20
#define CSR_MISPEC_MASK             (0xFF<<20)
#define CSR_SSEN                    (1<<18)
#define CSR_SCRAND                  (1<<17)
#define CSR_LLEXCL                  (1<<16)
#define CSR_DISVC                   (1<<15)
#define CSR_VCLRU                   (1<<14)
#define CSR_DCLRU                   (1<<13)
#define CSR_FASTLDQ                 (1<<12)
#define CSR_USERCAC                 (1<<11)
#define CSR_ANTI_MISPEC             (1<<10)
#define CSR_ANTI_FLUSHSFB           (1<<9)
#define CSR_STFILL                  (1<<8)
#define CSR_LIFEP                   (1<<7)
#define CSR_LLSYNC                  (1<<6)
#define CSR_BRBTDIS                 (1<<5)
#define CSR_RASDIS                  (1<<4)
#define CSR_STPRE_SHIFT             2
#define CSR_STPRE_MASK              (0x03<<2)
#define CSR_INSTPRE                 (1<<1)
#define CSR_DATAPRE                 (1<<0)

#define LA_CSR_IMPCTL2              0x081       /* Loongson config2 */
#define CSR_FLUSH_MTLB_SHIFT        0
#define CSR_FLUSH_MTLB              (1<<0)
#define CSR_FLUSH_STLB              (1<<1)
#define CSR_FLUSH_DTLB              (1<<2)
#define CSR_FLUSH_ITLB              (1<<3)
#define CSR_FLUSH_BTAC              (1<<4)

#define LA_CSR_GNMI                 0x082

/**
 * TLB refill registers
 */
#define LA_CSR_TLBREBASE            0x088       /* TLB refill exception base address */
#define LA_CSR_TLBRBADV             0x089       /* TLB refill badvaddr */
#define LA_CSR_TLBREPC              0x08A       /* TLB refill EPC */
#define LA_CSR_TLBRSAVE             0x08B       /* KScratch for TLB refill exception */
#define LA_CSR_TLBRELO0             0x08C       /* TLB refill entrylo0 */
#define LA_CSR_TLBRELO1             0x08D       /* TLB refill entrylo1 */
#define LA_CSR_TLBREHI              0x08E       /* TLB refill entryhi */
#define LA_CSR_TLBRPRMD             0x08F       /* TLB refill mode info */

/**
 * Machine error registers
 */
#define LA_CSR_MERRCTL              0x090       /* ERRCTL */
#define CSR_MERRCTL_ISMERR          (1<<0)
#define CSR_MERRCTL_REPAIRABLE      (1<<1)
#define LA_CSR_MERRINFO1            0x091       /* Error info1 */
#define LA_CSR_MERRINFO2            0x092       /* Error info2 */
#define LA_CSR_MERREBASE            0x093       /* Error exception base address */
#define LA_CSR_MERREPC              0x094       /* Error exception PC */
#define LA_CSR_MERRSAVE             0x095       /* KScratch for machine error exception */

#define LA_CSR_CTAG                 0x098       /* TagLo + TagHi */

#define LA_CSR_PRID                 0x0C0

/*
 * Shadow MCSR : 0xC0 ~ 0xFF
 */
#define LA_CSR_MCSR0                0x0C0       /* CPUCFG0 and CPUCFG1 */
#define MCSR0_INT_IMPL              (1<<58)     // 0
#define MCSR0_IOCSR_BRD             (1<<57)
#define MCSR0_HUGEPG                (1<<56)
#define MCSR0_RPLMTLB               (1<<55)
#define MCSR0_EXEPROT               (1<<54)
#define MCSR0_RI                    (1<<53)
#define MCSR0_UAL                   (1<<52)
#define MCSR0_VABIT_SHIFT           44
#define MCSR0_VABIT_MASK            (0xFF<<44)
#define MCSR0_PABIT_SHIFT           36
#define MCSR0_PABIT_MASK            (0xFF<<36)
#define MCSR0_IOCSR                 (1<<35)
#define MCSR0_PAGING                (1<<34)
#define MCSR0_GR64                  (1<<33)
#define MCSR0_GR32                  (1<<32)
#define MCSR0_PRID_WIDTH            32
#define MCSR0_PRID_MASK             0xFFFFFFFF
#define MCSR0_PRID                  0x14C010

#define LA_CSR_MCSR1                0x0C1       /* CPUCFG2 and CPUCFG3 */
#define MCSR1_HPFOLD                (1<<43)
#define MCSR1_SPW_LVL_SHIFT         40
#define MCSR1_SPW_LVL_MASK          (0x07<<40)
#define MCSR1_ICACHET               (1<<39)
#define MCSR1_ITLBT                 (1<<38)
#define MCSR1_LLDBAR                (1<<37)
#define MCSR1_SCDLY                 (1<<36)
#define MCSR1_LLEXC                 (1<<35)
#define MCSR1_UCACC                 (1<<34)
#define MCSR1_SFB                   (1<<33)
#define MCSR1_CCDMA                 (1<<32)
#define MCSR1_LAMO                  (1<<22)
#define MCSR1_LSPW                  (1<<21)
#define MCSR1_LOONGARCHBT           (1<<20)
#define MCSR1_ARMBT                 (1<<19)
#define MCSR1_X86BT                 (1<<18)
#define MCSR1_LLFTPVERS_SHIFT       15
#define MCSR1_LLFTPVERS_MASK        (0x07<<15)
#define MCSR1_LLFTP                 (1<<14)
#define MCSR1_VZVERS_SHIFT          11
#define MCSR1_VZVERS_MASK           (0x07<<11)
#define MCSR1_VZ                    (1<<10)
#define MCSR1_CRYPTO                (1<<9)
#define MCSR1_COMPLEX               (1<<8)
#define MCSR1_LASX                  (1<<7)
#define MCSR1_LSX                   (1<<6)
#define MCSR1_FPVERS_SHIFT          3
#define MCSR1_FPVERS_MASK           (0x07<<3)
#define MCSR1_FPDP                  (1<<2)
#define MCSR1_FPSP                  (1<<1)
#define MCSR1_FP                    (1<<0)

#define LA_CSR_MCSR2                0x0C2       /* CPUCFG4 and CPUCFG5 */
#define MCSR2_CCDIV_SHIFT           48
#define MCSR2_CCDIV_MASK            (0xFFFF<<48)
#define MCSR2_CCMUL_SHIFT           32
#define MCSR2_CCMUL_MASK            (0xFFFF<<32)
#define MCSR2_CCFREQ_MASK           (0xFFFFFFFF)
#define CCFREQ_DEFAULT              0x5f5e100   /* 100MHz */

#define LA_CSR_MCSR3                0x0C3       /* CPUCFG6 */
#define MCSR3_UPM                   (1<<14)
#define MCSR3_PMBITS_SHIFT          8
#define MCSR3_PMBITS_MASK           (0x3F<<8)
#define MCSR3_PMNUM_SHIFT           4
#define MCSR3_PMNUM_MASK            (0x0F<<4)
#define MCSR3_PAMVER_SHIFT          1
#define MCSR3_PAMVER_MASK           (0x07<<1)
#define MCSR3_PMP                   (1<<0)

#define LA_CSR_MCSR8                0x0C8       /* CPUCFG16 and CPUCFG17 */
#define MCSR8_L1I_SIZE_SHIFT        56
#define MCSR8_L1I_SIZE_MASK         (0x7F<<56)
#define MCSR8_L1I_IDX_SHIFT         48
#define MCSR8_L1I_IDX_MASK          (0xFF<<48)
#define MCSR8_L1I_WAY_SHIFT         32
#define MCSR8_L1I_WAY_MASK          (0xFFFF<<32)
#define MCSR8_L3DINCL               (1<<16)
#define MCSR8_L3DPRIV               (1<<15)
#define MCSR8_L3DPRE                (1<<14)
#define MCSR8_L3IUINCL              (1<<13)
#define MCSR8_L3IUPRIV              (1<<12)
#define MCSR8_L3IUUNIFY             (1<<11)
#define MCSR8_L3IUPRE               (1<<10)
#define MCSR8_L2DINCL               (1<<9)
#define MCSR8_L2DPRIV               (1<<8)
#define MCSR8_L2DPRE                (1<<7)
#define MCSR8_L2IUINCL              (1<<6)
#define MCSR8_L2IUPRIV              (1<<5)
#define MCSR8_L2IUUNIFY             (1<<4)
#define MCSR8_L2IUPRE               (1<<3)
#define MCSR8_L1DPRE                (1<<2)
#define MCSR8_L1IUUNIFY             (1<<1)
#define MCSR8_L1IUPRE               (1<<0)

#define LA_CSR_MCSR9                0x0C9       /* CPUCFG18 and CPUCFG19 */
#define MCSR9_L2U_SIZE_SHIFT        56
#define MCSR9_L2U_SIZE_MASK         (0x7F<<56)
#define MCSR9_L2U_IDX_SHIFT         48
#define MCSR9_L2U_IDX_MASK          (0xFF<<48)
#define MCSR9_L2U_WAY_SHIFT         32
#define MCSR9_L2U_WAY_MASK          (0xFFFF<<32)
#define MCSR9_L1D_SIZE_SHIFT        24
#define MCSR9_L1D_SIZE_MASK         (0x7F<<24)
#define MCSR9_L1D_IDX_SHIFT         16
#define MCSR9_L1D_IDX_MASK          (0xFF<<16)
#define MCSR9_L1D_WAY_MASK          (0xFFFF)

#define LA_CSR_MCSR10               0x0CA       /* CPUCFG20 */
#define MCSR10_L3U_SIZE_SHIFT       24
#define MCSR10_L3U_SIZE_MASK        (0x7F<<24)
#define MCSR10_L3U_IDX_SHIFT        16
#define MCSR10_L3U_IDX_MASK         (0xFF<<16)
#define MCSR10_L3U_WAY_MASK         (0xFFFF)

#define LA_CSR_MCSR24               0x0F0       /* CPUCFG48 */
#define MCSR24_RAMCG                (1<<3)
#define MCSR24_VFPUCG               (1<<2)
#define MCSR24_NAPEN                (1<<1)
#define MCSR24_MCSRLOCK             (1<<0)

/**
 * Uncached accelerate windows registers
 */
#define LA_CSR_UCAWIN               0x100
#define LA_CSR_UCAWIN0_LO           0x102
#define LA_CSR_UCAWIN0_HI           0x103
#define LA_CSR_UCAWIN1_LO           0x104
#define LA_CSR_UCAWIN1_HI           0x105
#define LA_CSR_UCAWIN2_LO           0x106
#define LA_CSR_UCAWIN2_HI           0x107
#define LA_CSR_UCAWIN3_LO           0x108
#define LA_CSR_UCAWIN3_HI           0x109

/**
 * Direct map windows registers
 */
#define LA_CSR_DMWIN0               0x180       /* 64 direct map win0: MEM & IF */
#define LA_CSR_DMWIN1               0x181       /* 64 direct map win1: MEM & IF */
#define LA_CSR_DMWIN2               0x182       /* 64 direct map win2: MEM */
#define LA_CSR_DMWIN3               0x183       /* 64 direct map win3: MEM */

/**
 * Direct map window 0/1
 */
#define CSR_DMW0_PLV0               (1ull << 0)
#define CSR_DMW0_VSEG               (0x8000ull)
#define CSR_DMW0_BASE               (CSR_DMW0_VSEG << DMW_PABITS)
#define CSR_DMW0_INIT               (CSR_DMW0_BASE | CSR_DMW0_PLV0)

#define CSR_DMW1_PLV0               (1ull << 0)
#define CSR_DMW1_MAT                (1ull << 4)
#define CSR_DMW1_VSEG               (0x9000ull)
#define CSR_DMW1_BASE               (CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT               (CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

/**
 * Performance counter registers
 */
#define LA_CSR_PERFCTRL0            0x200       /* 32 perf event 0 config */
#define LA_CSR_PERFCNTR0            0x201       /* 64 perf event 0 count value */
#define LA_CSR_PERFCTRL1            0x202       /* 32 perf event 1 config */
#define LA_CSR_PERFCNTR1            0x203       /* 64 perf event 1 count value */
#define LA_CSR_PERFCTRL2            0x204       /* 32 perf event 2 config */
#define LA_CSR_PERFCNTR2            0x205       /* 64 perf event 2 count value */
#define LA_CSR_PERFCTRL3            0x206       /* 32 perf event 3 config */
#define LA_CSR_PERFCNTR3            0x207       /* 64 perf event 3 count value */
#define CSR_PERFCTRL_PLV0           (1<<16)
#define CSR_PERFCTRL_PLV1           (1<<17)
#define CSR_PERFCTRL_PLV2           (1<<18)
#define CSR_PERFCTRL_PLV3           (1<<19)
#define CSR_PERFCTRL_IE             (1<<20)
#define CSR_PERFCTRL_EVENT          0x3ff

#if 0
/**
 * IOCSR
 */
#define LA_IOCSR_FEATURES           0x8
#define IOCSRF_TEMP                 (1<<0)
#define IOCSRF_NODECNT              (1<<1)
#define IOCSRF_MSI                  (1<<2)
#define IOCSRF_EXTIOI               (1<<3)
#define IOCSRF_CSRIPI               (1<<4)
#define IOCSRF_FREQCSR              (1<<5)
#define IOCSRF_FREQSCALE            (1<<6)
#define IOCSRF_DVFSV1               (1<<7)
#define IOCSRF_GMOD                 (1<<9)
#define IOCSRF_VM                   (1<<11)

#define LA_IOCSR_VENDOR             0x10

#define LA_IOCSR_CPUNAME            0x20

#define LA_IOCSR_NODECNT            0x408

#define LA_IOCSR_MISC_FUNC          0x420
#define IOCSR_MISC_FUNC_TIMER_RESET (1<<21)
#define IOCSR_MISC_FUNC_EXT_IOI_EN  (1<<48)

#define LA_IOCSR_CPUTEMP            0x428

/**
 * PerCore CSR, only accessable by local cores
 */
#define LA_IOCSR_IPI_STATUS         0x1000
#define LA_IOCSR_IPI_EN             0x1004
#define LA_IOCSR_IPI_SET            0x1008
#define LA_IOCSR_IPI_CLEAR          0x100c
#if 0
#define LA_IOCSR_MBUF0              0x1020
#define LA_IOCSR_MBUF1              0x1028
#define LA_IOCSR_MBUF2              0x1030
#define LA_IOCSR_MBUF3              0x1038

#define LA_IOCSR_IPI_SEND           0x1040
#define IOCSR_IPI_SEND_IP_SHIFT     0
#define IOCSR_IPI_SEND_CPU_SHIFT    16
#define IOCSR_IPI_SEND_BLOCKING     (1<<31)

#define LA_IOCSR_MBUF_SEND          0x1048
#define IOCSR_MBUF_SEND_BLOCKING    (1<<31)
#define IOCSR_MBUF_SEND_BOX_SHIFT   2
#define IOCSR_MBUF_SEND_BOX_LO(box) (box<<1)
#define IOCSR_MBUF_SEND_BOX_HI(box) ((box<<1)+1)
#define IOCSR_MBUF_SEND_CPU_SHIFT   16
#define IOCSR_MBUF_SEND_BUF_SHIFT   32
#define IOCSR_MBUF_SEND_H32_MASK    0xFFFFFFFF00000000ULL

#define LA_IOCSR_ANY_SEND           0x1158
#define IOCSR_ANY_SEND_BLOCKING     (1<<31)
#define IOCSR_ANY_SEND_NODE_SHIFT   18
#define IOCSR_ANY_SEND_MASK_SHIFT   27
#define IOCSR_ANY_SEND_BUF_SHIFT    32
#define IOCSR_ANY_SEND_H32_MASK     0xFFFFFFFF00000000ULL
#endif
/**
 * Register offset and bit definition for CSR access
 */
#define LA_IOCSR_TIMER_CFG           0x1060
#define LA_IOCSR_TIMER_TICK          0x1070
#define IOCSR_TIMER_CFG_RESERVED     (1<<63)
#define IOCSR_TIMER_CFG_PERIODIC     (1<<62)
#define IOCSR_TIMER_CFG_EN           (1<<61)
#define IOCSR_TIMER_MASK             0x0ffffffffffffULL
#define IOCSR_TIMER_INITVAL_RST      (0xffff<<48)

#define LA_IOCSR_EXTIOI_NODEMAP_BASE 0x14a0
#define LA_IOCSR_EXTIOI_IPMAP_BASE   0x14c0
#define LA_IOCSR_EXTIOI_EN_BASE      0x1600
#define LA_IOCSR_EXTIOI_BOUNCE_BASE  0x1680
#define LA_IOCSR_EXTIOI_ISR_BASE     0x1800
#define LA_IOCSR_EXTIOI_ROUTE_BASE   0x1c00
#define IOCSR_EXTIOI_VECTOR_NUM      256

#endif

/******************************************************************************
 * CSR_ECFG IM
 */
#define ECFG0_IM                    0x00001fff
#define ECFGB_SIP0                  0
#define ECFGF_SIP0                  (1<<0)
#define ECFGB_SIP1                  1
#define ECFGF_SIP1                  (1<<1)
#define ECFGB_IP0                   2
#define ECFGF_IP0                   (1<<2)
#define ECFGB_IP1                   3
#define ECFGF_IP1                   (1<<3)
#define ECFGB_IP2                   4
#define ECFGF_IP2                   (1<<4)
#define ECFGB_IP3                   5
#define ECFGF_IP3                   (1<<5)
#define ECFGB_IP4                   6
#define ECFGF_IP4                   (1<<6)
#define ECFGB_IP5                   7
#define ECFGF_IP5                   (1<<7)
#define ECFGB_IP6                   8
#define ECFGF_IP6                   (1<<8)
#define ECFGB_IP7                   9
#define ECFGF_IP7                   (1<<9)
#define ECFGB_PC                    10
#define ECFGF_PC                    (1<<10)
#define ECFGB_TIMER                 11
#define ECFGF_TIMER                 (1<<11)
#define ECFGB_IPI                   12
#define ECFGF_IPI                   (1<<12)
#define ECFGF(hwirq)                (1<< hwirq)

#define ESTATF_IP                   0x00001fff

/******************************************************************************
 * ExStatus.ExcCode
 */
#define EXCCODE_RSV			0		/* Reserved */
#define EXCCODE_TLBL		1		/* TLB miss on a load */
#define EXCCODE_TLBS		2		/* TLB miss on a store */
#define EXCCODE_TLBI		3		/* TLB miss on a ifetch */
#define EXCCODE_TLBM		4		/* TLB modified fault */
#define EXCCODE_TLBRI		5		/* TLB Read-Inhibit exception */
#define EXCCODE_TLBXI		6		/* TLB Execution-Inhibit exception */
#define EXCCODE_TLBPE		7		/* TLB Privilege Error */

#define EXCCODE_ADE			8		/* Address Error */
#define EXSUBCODE_ADEF		0		/* Fetch Instruction */
#define EXSUBCODE_ADEM		1		/* Access Memory*/

#define EXCCODE_ALE			9		/* Unalign Access */
#define EXCCODE_OOB			10  	/* Out of bounds */
#define EXCCODE_SYS			11		/* System call */
#define EXCCODE_BP			12		/* Breakpoint */
#define EXCCODE_INE			13		/* Inst. Not Exist */
#define EXCCODE_IPE			14		/* Inst. Privileged Error */
#define EXCCODE_FPDIS		15		/* FPU Disabled */
#define EXCCODE_LSXDIS		16  	/* LSX Disabled */
#define EXCCODE_LASXDIS		17		/* LASX Disabled */

#define EXCCODE_FPE			18		/* Floating Point Exception */
#define EXCSUBCODE_FPE		0		/* Floating Point Exception */
#define EXCSUBCODE_VFPE		1		/* Vector Exception */

#define EXCCODE_WATCH		19		/* Watch address reference */
#define EXCCODE_BTDIS		20		/* Binary Trans. Disabled */
#define EXCCODE_BTE			21		/* Binary Trans. Exception */
#define EXCCODE_PSI			22		/* Guest Privileged Error */
#define EXCCODE_HYP			23		/* Hypercall */

#define EXCCODE_GCM			24		/* Guest CSR modified */
#define EXCSUBCODE_GCSC		0		/* Software caused */
#define EXCSUBCODE_GCHC		1		/* Hardware caused */

#define EXCCODE_SE			25		/* Security */

#define EXCCODE_INT_START   64
#define EXCCODE_SIP0        64
#define EXCCODE_SIP1        65
#define EXCCODE_IP0         66
#define EXCCODE_IP1         67
#define EXCCODE_IP2         68
#define EXCCODE_IP3         69
#define EXCCODE_IP4         70
#define EXCCODE_IP5         71
#define EXCCODE_IP6         72
#define EXCCODE_IP7         73
#define EXCCODE_PC          74 		/* Performance Counter */
#define EXCCODE_TIMER       75
#define EXCCODE_IPI         76
#define EXCCODE_NMI         77
#define EXCCODE_INT_END     78
#define EXCCODE_INT_NUM		(EXCCODE_INT_END - EXCCODE_INT_START)

/******************************************************************************
 * FPU Registers
 */
#define LA_FCSR0			$r0
#define LA_FCSR1			$r1
#define LA_FCSR2			$r2
#define LA_FCSR3			$r3

/*
 * FPU Status Register Values ?
 */
#define FPU_CSR_RSVD		0xe0e0fce0

/*
 * X the exception cause indicator
 * E the exception enable
 * S the sticky/flag bit
 */
#define FPU_CSR_ALL_X		0x1f000000
#define FPU_CSR_INV_X		0x10000000
#define FPU_CSR_DIV_X		0x08000000
#define FPU_CSR_OVF_X		0x04000000
#define FPU_CSR_UDF_X		0x02000000
#define FPU_CSR_INE_X		0x01000000

#define FPU_CSR_ALL_S		0x001f0000
#define FPU_CSR_INV_S		0x00100000
#define FPU_CSR_DIV_S		0x00080000
#define FPU_CSR_OVF_S		0x00040000
#define FPU_CSR_UDF_S		0x00020000
#define FPU_CSR_INE_S		0x00010000

#define FPU_CSR_ALL_E		0x0000001f
#define FPU_CSR_INV_E		0x00000010		/* V: Invalid Operation */
#define FPU_CSR_DIV_E		0x00000008		/* Z: Division by Zero */
#define FPU_CSR_OVF_E		0x00000004		/* O: Overflow */
#define FPU_CSR_UDF_E		0x00000002		/* U: Underflow */
#define FPU_CSR_INE_E		0x00000001		/* I: Inexact */

/* Bits 8 and 9 of FPU Status Register specify the rounding mode */
#define FPU_CSR_RM			0x300
#define FPU_CSR_RN			0x000		/* nearest */
#define FPU_CSR_RZ			0x100		/* towards zero */
#define FPU_CSR_RU			0x200		/* towards +Infinity */
#define FPU_CSR_RD			0x300		/* towards -Infinity */

/* LBT extension */
#define FPU_CSR_TM_SHIFT	0x6
#define FPU_CSR_TM			0x40		/* float register in stack mode */

#endif // #ifndef _RTEMS_LOONGARCH_LA_CPU_H
