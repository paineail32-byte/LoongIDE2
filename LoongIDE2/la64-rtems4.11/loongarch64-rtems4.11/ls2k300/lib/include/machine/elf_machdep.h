
#ifndef _LOONGARCH64_ELF_MACHDEP_H_
#define _LOONGARCH64_ELF_MACHDEP_H_

#ifdef _LP64
#define ARCH_ELFSIZE			64		/* MD native binary size */
#else
#define ARCH_ELFSIZE			32		/* MD native binary size */
#endif

#if ARCH_ELFSIZE == 32
#define	ELF32_MACHDEP_ID_CASES \
		case EM_MIPS:		   \
			break;

#define	ELF32_MACHDEP_ID		EM_LOONGARCH

#elif ARCH_ELFSIZE == 64
#define	ELF64_MACHDEP_ID_CASES \
		case EM_MIPS:		   \
			break;

#define	ELF64_MACHDEP_ID		EM_LOONGARCH

#endif

//-----------------------------------------------------------------------------
// loongarch relocate section
//-----------------------------------------------------------------------------

/*
 * Used by the dynamic linker.
 */
#define R_LARCH_NONE            0
#define R_LARCH_32              1
#define R_LARCH_64              2
#define R_LARCH_RELATIVE        3
#define R_LARCH_COPY            4
#define R_LARCH_JUMP_SLOT       5
#define R_LARCH_TLS_DTPMOD32    6
#define R_LARCH_TLS_DTPMOD64    7
#define R_LARCH_TLS_DTPREL32    8
#define R_LARCH_TLS_DTPREL64    9
#define R_LARCH_TLS_TPREL32     10
#define R_LARCH_TLS_TPREL64     11
#define R_LARCH_IRELATIVE       12

/*
 * Reserved for future relocs that the dynamic linker must understand.
 */

/*
 * Used by the static linker for relocating .text.
 */
#define R_LARCH_MARK_LA             20
#define R_LARCH_MARK_PCREL          21

#define R_LARCH_SOP_PUSH_PCREL      22

#define R_LARCH_SOP_PUSH_ABSOLUTE   23

#define R_LARCH_SOP_PUSH_DUP        24
#define R_LARCH_SOP_PUSH_GPREL      25
#define R_LARCH_SOP_PUSH_TLS_TPREL  26
#define R_LARCH_SOP_PUSH_TLS_GOT    27
#define R_LARCH_SOP_PUSH_TLS_GD     28
#define R_LARCH_SOP_PUSH_PLT_PCREL  29

#define R_LARCH_SOP_ASSERT          30
#define R_LARCH_SOP_NOT             31
#define R_LARCH_SOP_SUB             32
#define R_LARCH_SOP_SL              33
#define R_LARCH_SOP_SR              34
#define R_LARCH_SOP_ADD             35
#define R_LARCH_SOP_AND             36
#define R_LARCH_SOP_IF_ELSE         37
#define R_LARCH_SOP_POP_32_S_10_5           38
#define R_LARCH_SOP_POP_32_U_10_12          39
#define R_LARCH_SOP_POP_32_S_10_12          40
#define R_LARCH_SOP_POP_32_S_10_16          41
#define R_LARCH_SOP_POP_32_S_10_16_S2       42
#define R_LARCH_SOP_POP_32_S_5_20           43
#define R_LARCH_SOP_POP_32_S_0_5_10_16_S2   44
#define R_LARCH_SOP_POP_32_S_0_10_10_16_S2  45
#define R_LARCH_SOP_POP_32_U                46

/*
 * Used by the static linker for relocating non .text.
 */
#define R_LARCH_ADD8        47
#define R_LARCH_ADD16       48
#define R_LARCH_ADD24       49
#define R_LARCH_ADD32       50
#define R_LARCH_ADD64       51
#define R_LARCH_SUB8        52
#define R_LARCH_SUB16       53
#define R_LARCH_SUB24       54
#define R_LARCH_SUB32       55
#define R_LARCH_SUB64       56

/*
 * I don't know what it is.  Existing in almost all other arch.
 */
#define R_LARCH_GNU_VTINHERIT   57
#define R_LARCH_GNU_VTENTRY     58

//-----------------------------------------------------------------------------
// ELF Flags
//-----------------------------------------------------------------------------

/*
 * Processor specific flags for the ELF header e_flags field.
 */
#define EF_LOONGARCH_ABI_ILP32	    	0b01
#define EF_LOONGARCH_ABI_LP64	    	0b11
#define EF_LOONGARCH_ABI_MASK	    	0b11

#define EF_LOONGARCH_FLOAT_ABI_SOFT    	0b001100
#define EF_LOONGARCH_FLOAT_ABI_SINGLE  	0b001000
#define EF_LOONGARCH_FLOAT_ABI_DOUBLE  	0b000000
#define EF_LOONGARCH_FLOAT_ABI_MASK    	0b111100

#define EF_LOONGARCH_IS_LP64(abi) \
  ((abi & EF_LOONGARCH_ABI_MASK) == EF_LOONGARCH_ABI_LP64)
#define EF_LOONGARCH_IS_ILP32(abi) \
  ((abi & EF_LOONGARCH_ABI_MASK) == EF_LOONGARCH_ABI_ILP32)

#define EF_LOONGARCH_IS_SOFT_FLOAT(abi) \
  ((abi & EF_LOONGARCH_FLOAT_ABI_MASK) == EF_LOONGARCH_FLOAT_ABI_SOFT)
#define EF_LOONGARCH_IS_SINGLE_FLOAT(abi) \
  ((abi & EF_LOONGARCH_FLOAT_ABI_MASK) == EF_LOONGARCH_FLOAT_ABI_SINGLE)
#define EF_LOONGARCH_IS_DOUBLE_FLOAT(abi) \
  ((abi & EF_LOONGARCH_FLOAT_ABI_MASK) == EF_LOONGARCH_FLOAT_ABI_DOUBLE)

#define EF_LOONGARCH_ABI (EF_LOONGARCH_ABI_MASK | EF_LOONGARCH_FLOAT_ABI_MASK)

#define	ELF32_MACHDEP_ENDIANNESS	ELFDATA2LSB
#define	ELF64_MACHDEP_ENDIANNESS	ELFDATA2LSB

//-----------------------------------------------------------------------------

#ifdef _KERNEL
#ifdef _KERNEL_OPT
#include "opt_compat_netbsd.h"
#endif
#ifdef COMPAT_16
/*
 * Up to 1.6, the ELF dynamic loader (ld.elf_so) was not relocatable.
 * Tell the kernel ELF exec code not to try relocating the interpreter
 * for dynamically-linked ELF binaries.
 */
#define ELF_INTERP_NON_RELOCATABLE
#endif /* COMPAT_16 */

/*
 * We need to be able to include the ELF header so we can pick out the
 * ABI being used.
 */
#ifdef ELFSIZE
#define	ELF_MD_PROBE_FUNC		ELFNAME2(mips_netbsd,probe)
#define	ELF_MD_COREDUMP_SETUP	ELFNAME2(coredump,setup)
#endif

struct exec_package;

int loongarch_netbsd_elf32_probe(struct lwp *, struct exec_package *, void *, char *, vaddr_t *);
void coredump_elf32_setup(struct lwp *, void *);

int loongarch_netbsd_elf64_probe(struct lwp *, struct exec_package *, void *, char *, vaddr_t *);
void coredump_elf64_setup(struct lwp *, void *);

#endif /* _KERNEL */

#endif /* _LOONGARCH64_ELF_MACHDEP_H_ */

