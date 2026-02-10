/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * cache.S
 *
 * created: 2022-02-18
 *  author: Bian
 */

#include "cpu.h"
#include "asm.h"
#include "regdef.h"

/**
 * LOONGARCH64 cache functions
 */

#define OP_SCACHE               0               // not operate scache

/*
 * Cache Operations
 */
#if (!OP_SCACHE)
#define Index_Invalidate_I      0x00
#define Index_Store_Tag_I       0x08
#define Hit_Invalidate_I        0x10
#define Hit_Lock_I              0x1C

#define Index_Writeback_Inv_D   0x01
#define Index_Load_Tag_D        0x05
#define Index_Store_Tag_D       0x09
#define Hit_Invalidate_D        0x11
#define Hit_Writeback_Inv_D     0x15
#define Hit_Lock_D              0x1D

#else
#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Invalidate_SI     0x02
#define Index_Writeback_Inv_SD  0x03
#define Index_Writeback_Inv_S  	0x03
#define Index_Load_Tag_I		0x04
#define Index_Load_Tag_D		0x05
#define Index_Load_Tag_SI		0x06
#define Index_Load_Tag_SD		0x07
#define Index_Load_Tag_S		0x07
#define Index_Store_Tag_I		0x08

#define Index_Store_Tag_D		0x09
#define Index_Store_Tag_SI		0x0A
#define Index_Store_Tag_SD		0x0B
#define Index_Store_Tag_S		0x0B
#define Create_Dirty_Excl_D		0x0d
#define Create_Dirty_Excl_SD	0x0f
#define Hit_Invalidate_I		0x10
#define Hit_Invalidate_D		0x11
#define Hit_Invalidate_SI		0x12
#define Hit_Invalidate_SD		0x13
#define Hit_Invalidate_S		0x13
#define Fill					0x14
#define Hit_Writeback_Inv_D		0x15
		/* 0x16 is unused */
#define Hit_Writeback_Inv_SD	0x17
#define Hit_Writeback_Inv_S		0x17
#define Hit_Writeback_I			0x18
#define Hit_Writeback_D			0x19
		/* 0x1a is unused */
#define Hit_Writeback_SD		0x1b
		/* 0x1c is unused */
		/* 0x1e is unused */
#define Hit_Set_Virtual_SI		0x1e
#define Hit_Set_Virtual_SD		0x1f

#define	Index_Load_Data_D		0x19
#define	Index_Load_Data_S		0x1b

#define	Index_Store_Data_I		0x1c
#define	Index_Store_Data_D		0x1d
#define	Index_Store_Data_S		0x1f

#endif

//-------------------------------------------------------------------------------------------------
/*
 * 注意: 保存的是 32 位数据
 */
 
    .data
    .align 2
    
memory_size:                    /* memory */
    .word   0

icache_size:                    /* icache */
    .word   0
icache_linesize:
    .word   0
icache_nway:
    .word   0

dcache_size:                    /* icache */
    .word   0
dcache_linesize:
    .word   0
dcache_nway:
    .word   0

#if OP_SCACHE

scache_size:                    /* scache */
    .word   0
scache_linesize:
    .word   0
scache_nway:
    .word   0

#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

    .text

/**
 * static void _size_cache(void)
 *
 * Internal routine to determine cache sizes by looking at  config
 * register.  Sizes are returned in registers, as follows:
 *	s0	icache_size
 *	s1	icache_linesize
 *	s2	icache_nway
 *	s3	dcache_size
 *	s4	dcache_linesize
 *	s5	dcache_nway
 *	s6	scache_size
 *	s7	scache_linesize
 *	s8	scache_nway
 */
LEAF(_size_cache)
    csrrd       t1, LA_CSR_MCSR8
    csrrd       t2, LA_CSR_MCSR9

    /******************************************************
     * icache nways
     */
    srli.d      s2, t1, MCSR8_L1I_WAY_SHIFT
    li.d        t0, 0xFFFF
    and         s2, s2, t0
    addi.d      s2, s2, 1                   /* +1 */
    
    /* icache linesize */
    srli.d      t0, t1, MCSR8_L1I_SIZE_SHIFT
    andi        t0, t0, 0x7F
    li.d        s1, 1                       /* 1<<linesize */
    sll.d       s1, s1, t0

    /* icache index: lines per way */
    srli.d      t0, t1, MCSR8_L1I_IDX_SHIFT
    andi        t0, t0, 0xFF
    li.d        s0, 1                       /* 1<<index */
    sll.d       s0, s0, t0
    
    /* icache size */
    mul.d       s0, s0, s1                  /* nways*index*linesize */
    mul.d       s0, s0, s2

    /******************************************************
     * dcache nways
     */
    li.d        t0, 0xFFFF
    and         s5, t2, t0
    addi.d      s5, s5, 1                   /* +1 */
    
    /* dcache linesize */
    srli.d      t0, t2, MCSR9_L1D_SIZE_SHIFT
    andi        t0, t0, 0x7F
    li.d        s4, 1                       /* 1<<linesize */
    sll.d       s4, s4, t0

    /* dcache index: lines per way */
    srli.d      t0, t2, MCSR9_L1D_IDX_SHIFT
    andi        t0, t0, 0xFF
    li.d        s3, 1                       /* 1<<index */
    sll.d       s3, s3, t0
    
    /* dcache size */
    mul.d       s3, s3, s4                  /* nways*index*linesize */
    mul.d       s3, s3, s5

#if OP_SCACHE
    /******************************************************
     * scache nways
     */
    srli.d      s8, t2, MCSR9_L2U_WAY_SHIFT
    li.d        t0, 0xFFFF
    and         s8, s8, t0
    addi.d      s8, s8, 1                   /* +1 */

    /* dcache linesize */
    srli.d      t0, t2, MCSR9_L2U_SIZE_SHIFT
    andi        t0, t0, 0x7F
    li.d        s7, 1                       /* 1<<linesize */
    sll.d       s7, s7, t0

    /* dcache index: lines per way */
    srli.d      t0, t2, MCSR9_L2U_IDX_SHIFT
    andi        t0, t0, 0xFF
    li.d        s6, 1                       /* 1<<index */
    sll.d       s6, s6, t0

    /* dcache size */
    mul.d       s6, s6, s7                  /* nways*index*linesize */
    mul.d       s6, s6, s8
#endif

    jirl        zero, ra, 0
END(_size_cache)

/**
 * void config_cache(void)
 *
 * Work out size of I, D & S caches, assuming they are already initialised.
 */
LEAF(config_cache)
    la          t0, icache_size
    ld.w        t0, t0, 0
    bgtz        t0, 8f

    move        t5, ra
    bl          _size_cache
    move        ra, t5

    la          t0, icache_size
    st.w	    s0, t0, 0x00            // icache_size
    st.w	    s1, t0, 0x04            // icache_linesize
    st.w	    s2, t0, 0x08            // icache_nway
    st.w	    s3, t0, 0x0c            // dcache_size
    st.w	    s4, t0, 0x10            // dcache_linesize
    st.w	    s5, t0, 0x14            // dcache_nway
#if OP_SCACHE
    st.w	    s6, t0, 0x18            // scache_size
    st.w	    s7, t0, 0x1c            // scache_linesize
    st.w	    s8, t0, 0x20            // scache_nway
#endif

8:
    jirl        zero, ra, 0
END(config_cache)

#if 0
/**
 * void _init_cache(void)
 *
 * LS2K 的PMON的打开CACHE的, 不能作废 CACHE
 */
LEAF(_init_cache)
    li.d        t0, CSR_CRMD_IE             /* disable interrupt */
    csrxchg     zero, t0, LA_CSR_CRMD

    move        t5, ra
    bl          _size_cache
    move        ra, t5

    /*
     * 返回参数在 s0~s8中
     */
     
    blez        s0, 2f
    li.d        a0, 0x9000000000000000
    move        a2, s0                      // icache_size
    move        a3, s1                      // icache_linesize
    move        a1, a2
    add.d       a1, a1, a0
1:
    cacop       Index_Store_Tag_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

2:
    blez        s3, 4f
    li.d        a0, 0x9000000000000000
    move        a2, s3                      // dcache_size
    move        a3, s4                      // dcache_linesize
    move        a1, a2
    add.d       a1, a1, a0
3:
    cacop       Index_Store_Tag_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b

4:
    blez        s6, 6f
    li.d        a0, 0x9000000000000000
    move        a2, s6                      // scache_size
    move        a3, s7                      // scache_linesize
    move        a1, a2
    add.d       a1, a1, a0
5:
    cacop       Index_Store_Tag_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 5b

6:
    li.d        t0, CSR_CRMD_IE             /* enable interrupt */
    csrxchg     t0, t0, LA_CSR_CRMD

    jirl        zero, ra, 0
END(_init_cache)
#endif

/**
 * void flush_cache(void)
 *
 * Flush and invalidate all caches
 */
LEAF(flush_cache)
#if OP_SCACHE                               /* scache */
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 2f
    ld.w        a3, t0, 4                   // scache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
1:
    cacop       Index_Writeback_Inv_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b
    b           6f                          // don't need op icache/dcache yet?
    
2:
#endif

    /* icache
     */
    la          t0, icache_size
    ld.w        a2, t0, 0                   // icache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // icache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
3:
    cacop       Index_Invalidate_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b

    /* dcache
     */
    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    ld.w        a3, t0, 4                   // dcache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
4:
    cacop       Index_Writeback_Inv_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 4b

6:
    jirl        zero, ra, 0
END(flush_cache)

/**
 * void flush_cache_nowrite(void)
 *
 * Invalidate all caches
 */
LEAF(flush_cache_nowrite)
    li.d        t6, CSR_CRMD_IE             /* disable interrupt */
    csrxchg     zero, t6, LA_CSR_CRMD

    /* icache
     */
    la          t0, icache_size
    ld.w        a2, t0, 0                   // icache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // icache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
1:
    cacop       Index_Invalidate_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

    /* dcache
     */
    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    ld.w        a3, t0, 4                   // dcache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
2:
    cacop       Index_Store_Tag_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 2b

#if OP_SCACHE                               /* scache */
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // scache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
3:
    cacop       Index_Store_Tag_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b
#endif

6:
    li.d        t6, CSR_CRMD_IE             /* enable interrupt */
    csrxchg     t6, t6, LA_CSR_CRMD
    jirl        zero, ra, 0
END(flush_cache_nowrite)

/*
 * void clean_cache(unsigned long kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(clean_cache)
#if OP_SCACHE                               /* scache */
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 2f
    ld.w        a3, t0, 4                   // scache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Hit_Writeback_Inv_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b
    b           6f
    
2:
#endif

    /* icache
     */
    la          t0, icache_size
    ld.w        a2, t0, 0                   // icache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // icache_linesize
    move        t7, a0                      // save kva
    move        t8, a1                      // save n
    add.d       a1, a1, a0
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
3:
    cacop       Hit_Invalidate_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b

    /* dcache
     */
    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    ld.w        a3, t0, 4                   // dcache_linesize
    move        a0, t7                      // load kva
    move        a1, t8                      // load kva
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
4:
    cacop       Hit_Writeback_Inv_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 4b

6:
    jirl        zero, ra, 0
END(clean_cache)

/**
 * void flush_dcache(void)
 *
 * Flush and invalidate data cache
 */
LEAF(flush_dcache)

    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    ld.w        a3, t0, 4                   // dcache_linesize
    li.d        a0, 0x9000000000000000
    move        a1, a2
    add.d       a1, a1, a0
1:
    cacop       Index_Writeback_Inv_D, a0, 0
    add.d       a0, a0, a3
    ble         a0, a1, 1b

    jirl        zero, ra, 0
END(flush_dcache)

/*
 * void clean_dcache(unsigned long kva, size_t n)
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(clean_dcache)
#if OP_SCACHE                               /* scache */
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 2f
    ld.w        a3, t0, 4                   // scache_linesize
    add.d       a1, a1, a0
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1
    addi.d      a1, a1, -1
    and         a1, a1, t1
1:
    cacop       Hit_Writeback_Inv_S, a0, 0
    add.d       a0, a0, a3
    ble         a0, a1, 1b
    b           6f

2:
#endif

    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // dcache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
3:
    cacop       Hit_Writeback_Inv_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b

6:
    jirl        zero, ra, 0
END(clean_dcache)

/*
 * void clean_dcache_indexed(unsigned long kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(clean_dcache_indexed)
    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // dcache_linesize
    move        a1, a2 
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Index_Writeback_Inv_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_dcache_indexed)

/*
 * void clean_dcache_nowrite(unsigned long kva, size_t n)
 *
 * Invalidate an address range in primary data cache
 */
LEAF(clean_dcache_nowrite)
#if OP_SCACHE                               /* scache */
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 2f
    ld.w        a3, t0, 4                   // scache_linesize
    add.d       a1, a1, a0
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1
    addi.d      a1, a1, -1
    and         a1, a1, t1
1:
    cacop       Hit_Invalidate_S, a0, 0
    add.d       a0, a0, a3
    ble         a0, a1, 1b
    b           6f

2:
#endif

    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // dcache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
3:
    cacop       Hit_Invalidate_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 3b

6:
    jirl        zero, ra, 0
END(clean_dcache_nowrite)

/*
 * void clean_dcache_nowrite_indexed(unsigned long kva, size_t n)
 *
 * Invalidate indexed range in primary data cache
 */
LEAF(clean_dcache_nowrite_indexed)
    li.d        t6, CSR_CRMD_IE             /* disable interrupt */
    csrxchg     zero, t6, LA_CSR_CRMD

    la          t0, dcache_size
    ld.w        a2, t0, 0                   // dcache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // dcache_linesize
    move        a1, a2 
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Index_Store_Tag_D, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    li.d        t6, CSR_CRMD_IE             /* enable interrupt */
    csrxchg     t6, t6, LA_CSR_CRMD
    jirl        zero, ra, 0
END(clean_dcache_nowrite_indexed)

/*
 * void clean_icache(unsigned long kva, size_t n)
 *
 * Invalidate address range in primary instruction cache
 */
LEAF(clean_icache)
    la          t0, icache_size
    ld.w        a2, t0, 0                   // icache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // icache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Hit_Invalidate_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_icache)

/*
 * void clean_icache_indexed(unsigned long kva, size_t n)
 *
 * Invalidate indexed range in primary instruction cache
 */
LEAF(clean_icache_indexed)
    la          t0, icache_size
    ld.w        a2, t0, 0                   // icache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // icache_linesize
    move        a1, a2 
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Index_Invalidate_I, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_icache_indexed)

#if OP_SCACHE
/*
 * void clean_scache(unsigned long kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(clean_scache)
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // scache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Hit_Writeback_Inv_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_scache)

/*
 * void clean_scache_indexed(unsigned long kva, size_t n)
 *
 * Writeback and invalidate indexed range in secondary cache
 */
LEAF(clean_scache_indexed)
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // scache_linesize
    move        a1, a2 
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Index_Writeback_Inv_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_scache_indexed)

/*
 * void clean_scache_nowrite(unsigned long kva, size_t n)
 *
 * Invalidate an address range in secondary cache
 */
LEAF(clean_scache_nowrite)
    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // scache_linesize
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Hit_Invalidate_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    jirl        zero, ra, 0
END(clean_scache_nowrite)

/*
 * void clean_scache_nowrite_indexed(unsigned long kva, size_t n)
 *
 * Invalidate indexed range in secondary cache
 */
LEAF(clean_scache_nowrite_indexed)
    li.d        t6, CSR_CRMD_IE             /* disable interrupt */
    csrxchg     zero, t6, LA_CSR_CRMD

    la          t0, scache_size
    ld.w        a2, t0, 0                   // scache_size
    blez        a2, 6f
    ld.w        a3, t0, 4                   // scache_linesize
    move        a1, a2 
    add.d       a1, a1, a0 
    addi.d      t1, a3, -1                  /* aligned */
    nor         t1, zero, t1
    and         a0, a0, t1 
    addi.d      a1, a1, -1 
    and         a1, a1, t1 
1:
    cacop       Index_Store_Tag_S, a0, 0
    add.d       a0, a0, a3 
    ble         a0, a1, 1b

6:
    li.d        t6, CSR_CRMD_IE             /* enable interrupt */
    csrxchg     t6, t6, LA_CSR_CRMD
    jirl        zero, ra, 0
END(clean_scache_nowrite_indexed)

#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * void set_memory_size(unsigned int memory_size)
 */
LEAF(set_memory_size)
    la          t0, memory_size
    st.w        a0, t0, 0
    jirl        zero, ra, 0
END(set_memory_size)

/**
 * function: unsigned int get_memory_size(void)
 * return:   memory size.
 */
LEAF(get_memory_size)
    la          v0, memory_size
    ld.w        v0, v0, 0
	jirl	    zero, ra, 0
END(get_memory_size)

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */
 
