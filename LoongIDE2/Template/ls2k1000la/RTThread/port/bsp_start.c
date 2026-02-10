/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp_start.c
 *
 * created: 2022-02-18
 *  author: Bian
 */

/*
 * 1.安装中断向量
 * 2.初始化 Tick
 * 3.初始化 Console
 */

#include <stdint.h>
#include <string.h>
#include <larchintrin.h>

#include "rtthread.h"
#include "rthw.h"

#include "cpu.h"
#include "ls2k1000.h"

extern void machine_error_entry(void);
extern void exception_common_entry(void);
extern void tlbrefill_error_entry(void);

extern void loongarch_init_isr_table(void);
extern void Clock_initialize(void);

extern void filesystem_initialize(void);
extern void register_all_devices(void);

extern void console_init(unsigned int baudrate);

//-------------------------------------------------------------------------------------------------
// LS2K1000 当前使用的CPU核
//-------------------------------------------------------------------------------------------------

unsigned int __cpunum;                  // read from LA_CSR_CPUNUM

//-------------------------------------------------------------------------------------------------
// PMON 配置的芯片频率参数
//-------------------------------------------------------------------------------------------------

unsigned int cpu_frequency;             // 芯片主频

unsigned int ddr_frequency;             // 400~700MHz
unsigned int gpu_frequency;             // 300~500MHz
unsigned int hda_frequency;             // FIXED 24MHz

unsigned int dc_frequency;              // 200MHz~
unsigned int gmac_frequency;            // FIXED 125MHz

unsigned int pix0_frequency;            // 100~250MHz
unsigned int pix1_frequency;            // 100~250MHz

unsigned int apb_frequency;             // APB 总线频率
unsigned int usb_frequency;
unsigned int sata_frequency;

/**
 * 晶振频率
 */
#define OSC_FREQ    100000000           // 100MHz

//-------------------------------------------------------------------------------------------------
// functions
//-------------------------------------------------------------------------------------------------

static void fetch_frequency_setting(void)
{
    unsigned long r0, r1;
    unsigned int divref, loopc, divout;

    /**************************************************************************
     * 软件配置模式
     **************************************************************************/

    r0 = READ_REG64(NODE_PLL0_BASE);
    r1 = READ_REG64(NODE_PLL1_BASE);
    divref = (unsigned int)((r0 & NODE_PLL0_L1_DIV_REF_MASK) >> NODE_PLL0_L1_DIV_REF_SHIFT);
    loopc  = (unsigned int)((r0 & NODE_PLL0_L1_DIV_LOOPC_MASK) >> NODE_PLL0_L1_DIV_LOOPC_SHIFT);
    /* cpu */
    divout = (unsigned int)(r1 & NODE_PLL1_L2_DIV_OUT_MASK);
    cpu_frequency = OSC_FREQ / divref * loopc / divout;

    r0 = READ_REG64(DDR_PLL0_BASE);
    r1 = READ_REG64(DDR_PLL1_BASE);
    divref = (unsigned int)((r0 & DDR_PLL0_L1_DIV_REF_MASK) >> DDR_PLL0_L1_DIV_REF_SHIFT);
    loopc  = (unsigned int)((r0 & DDR_PLL0_L1_DIV_LOOPC_MASK) >> DDR_PLL0_L1_DIV_LOOPC_SHIFT);
    /* ddr */
    divout = (unsigned int)(r1 & DDR_PLL1_L2_DIV_OUT_MASK);
    ddr_frequency = OSC_FREQ / divref * loopc / divout;
    /* hda */
    divout = (unsigned int)((r1 & DDR_PLL1_L2_DIV_OUT_HDA_MASK) >> DDR_PLL1_L2_DIV_OUT_HDA_SHIFT);
    hda_frequency = OSC_FREQ / divref * loopc / divout;
    /* gpu */
    divout = (unsigned int)((r1 & DDR_PLL1_L2_DIV_OUT_GPU_MASK) >> DDR_PLL1_L2_DIV_OUT_GPU_SHIFT);
    gpu_frequency = OSC_FREQ / divref * loopc / divout;

    r0 = READ_REG64(DC_PLL0_BASE);
    r1 = READ_REG64(DC_PLL1_BASE);
    divref = (unsigned int)((r0 & DC_PLL0_L1_DIV_REF_MASK) >> DC_PLL0_L1_DIV_REF_SHIFT);
    loopc  = (unsigned int)((r0 & DC_PLL0_L1_DIV_LOOPC_MASK) >> DC_PLL0_L1_DIV_LOOPC_SHIFT);
    /* dc */
    divout = (unsigned int)(r1 & DC_PLL1_L2_DIV_OUT_MASK);
    dc_frequency = OSC_FREQ / divref * loopc / divout;
    /* gmac */
    divout = (unsigned int)((r1 & DC_PLL1_L2_DIV_OUT_GMAC_MASK) >> DC_PLL1_L2_DIV_OUT_GMAC_SHIFT);
    gmac_frequency = OSC_FREQ / divref * loopc / divout;

    r0 = READ_REG64(PIX0_PLL0_BASE);
    r1 = READ_REG64(PIX0_PLL1_BASE);
    divref = (unsigned int)((r0 & PIX0_PLL0_L1_DIV_REF_MASK) >> PIX0_PLL0_L1_DIV_REF_SHIFT);
    loopc  = (unsigned int)((r0 & PIX0_PLL0_L1_DIV_LOOPC_MASK) >> PIX0_PLL0_L1_DIV_LOOPC_SHIFT);
    /* pix0 */
    divout = (unsigned int)(r1 & PIX0_PLL1_L2_DIV_OUT_MASK);
    pix0_frequency = OSC_FREQ / divref * loopc / divout;

    r0 = READ_REG64(PIX1_PLL0_BASE);
    r1 = READ_REG64(PIX1_PLL1_BASE);
    divref = (unsigned int)((r0 & PIX1_PLL0_L1_DIV_REF_MASK) >> PIX1_PLL0_L1_DIV_REF_SHIFT);
    loopc  = (unsigned int)((r0 & PIX1_PLL0_L1_DIV_LOOPC_MASK) >> PIX1_PLL0_L1_DIV_LOOPC_SHIFT);
    /* pix1 */
    divout = (unsigned int)(r1 & PIX1_PLL1_L2_DIV_OUT_MASK);
    pix1_frequency = OSC_FREQ / divref * loopc / divout;

    /*
     * 设备时钟分频配置寄存器, 分频计算公式为: fout=fin*(freqscale+1)/8.
     *
     * 基准频率 gmac_frequency
     */
    r0 = READ_REG64(FREQSCALE_BASE);

    apb_frequency  = gmac_frequency * (((r0 & FREQSCALE_APB_MASK)  >> FREQSCALE_APB_SHIFT)  + 1) / 8;
    usb_frequency  = gmac_frequency * (((r0 & FREQSCALE_USB_MASK)  >> FREQSCALE_USB_SHIFT)  + 1) / 8;
    sata_frequency = gmac_frequency * (((r0 & FREQSCALE_SATA_MASK) >> FREQSCALE_SATA_SHIFT) + 1) / 8;

    return;
}

//-----------------------------------------------------------------------------
// LS2K500 bsp start
//-----------------------------------------------------------------------------

#ifdef RT_KSERVICE_USING_STDLIB_MEMCPY
#error "Don't define RT_KSERVICE_USING_STDLIB_MEMCPY"
#endif

//-----------------------------------------------------------------------------

extern int bsp_start_hook1(void);
extern int bsp_start_hook2(void);

extern unsigned char _end;

void rt_hw_board_init(void)     //void bsp_start(void)
{
    uint64_t eentry;

    loongarch_interrupt_disable();

#ifndef RT_KSERVICE_USING_STDLIB_MEMCPY

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_MERREBASE));
    rt_memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)machine_error_entry, 32);
    
    eentry = __csrrd_d(LA_CSR_EBASE);
    rt_memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)exception_common_entry, 32);

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_TLBREBASE));
    rt_memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)tlbrefill_error_entry, 32);
    
#else

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_MERREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)machine_error_entry, 32);

    eentry = __csrrd_d(LA_CSR_EBASE);
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)exception_common_entry, 32);

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_TLBREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)tlbrefill_error_entry, 32);

#endif

    asm volatile( "dbar 0; " );
    asm volatile( "ibar 0; " );

    loongarch_init_isr_table();         /* initialize isr table */

    fetch_frequency_setting();

    /**
     * 初始化内存管理
     */
    #ifdef RT_USING_HEAP
    {
        extern unsigned char _end;

        size_t heap_start = ((size_t)&_end + 0x10) & ~0x0FULL;
        size_t heap_end   = PHYS_TO_CACHED(get_memory_size() - 8);
        heap_end -= 4*1024*1024;					/* 保留后 4M 内存 */

        if (heap_start + 1*1024*1024 < heap_end)	/* 至少有 1M 空间 */
        {
            rt_system_heap_init((void *)heap_start, (void *)heap_end);
        }
        else
        {
            for ( ; ; );
        }
    }
    #else
        #error "must define RT_USING_HEAP to manager heap"
    #endif

    bsp_start_hook1();					/* hook1: 实现文件系统初始化等 */

    console_init(115200);               /* initialize console */

#ifdef RT_USING_FPU
    /* init hardware fpu */
    rt_hw_fpu_init();
#endif

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

    Clock_initialize();                 /* initialize ticker */

    loongarch_interrupt_enable();		/* Enable CPU Interrept */

    /**
     * XXX 有创建线程的函数, RTT 不能在此处执行
     */
#if 0
    bsp_start_hook2();					/* hook2: 实现 EMMC, USB 初始化等 */
#endif

    return;
}

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */


