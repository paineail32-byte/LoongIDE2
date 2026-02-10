/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp_start.c
 *
 * created: 2024-07-18
 *  author: Bian
 */

#include <stdint.h>
#include <string.h>
#include <larchintrin.h>

#include "bsp.h"

#include "cpu.h"
#include "ls2k300.h"

#include "os.h"

extern void machine_error_entry(void);
extern void exception_common_entry(void);
extern void tlbrefill_error_entry(void);

extern void loongarch_init_isr_table(void);
extern void Clock_initialize(void);

extern void console_init(unsigned int baudrate);
extern int main(void);

//-------------------------------------------------------------------------------------------------
// PMON 配置的芯片频率参数
//-------------------------------------------------------------------------------------------------

unsigned int cpu_frequency;             // 芯片主频
unsigned int ddr_frequency;

unsigned int net_frequency;
unsigned int gmac_frequency;
unsigned int i2s_frequency;
unsigned int usb_frequency;
unsigned int apb_frequency;             // APB 总线频率
unsigned int boot_frequency;
unsigned int sdio_frequency;
unsigned int pix_frequency;

unsigned int gmacbp_frequency;          // ?

/**
 * 晶振频率
 */
#define OSC_FREQ    120000000ULL

unsigned int osc_frequency = OSC_FREQ;

//-----------------------------------------------------------------------------

static void fetch_frequency_setting(void)
{
    unsigned int clk_sel, r0, r1;
    unsigned int odiv, loopc, refc;

    unsigned long mcsr2 = __csrrd_d(LA_CSR_MCSR2);
    mcsr2 &= MCSR2_CCFREQ_MASK;
    osc_frequency = (unsigned int)mcsr2;

    clk_sel = READ_REG32(CHIP_SAMP0_BASE);
    clk_sel &= SAMP0_CLK_SEL_MASK;

    switch (clk_sel)
    {
        case SAMP0_CLK_SEL_HW_LO:       // 00: 硬件低频时钟配置模式, PLL按照低频配置参数输出时钟;
            cpu_frequency  = 750*1000*1000;
            ddr_frequency  = 800*1000*1000;
            net_frequency  = 266*1000*1000;
            gmac_frequency = 125*1000*1000;
            i2s_frequency  = 750*1000*1000;
            usb_frequency  = 100*1000*1000;
            apb_frequency  = 100*1000*1000;
            boot_frequency = 100*1000*1000;
            sdio_frequency = 100*1000*1000;
            pix_frequency  = 100*1000*1000;
            break;

        case SAMP0_CLK_SEL_HW_HI:       // 01: 硬件高频时钟配置模式, PLL按照高频配置参数输出时钟;
            cpu_frequency  = 1000*1000*1000;
            ddr_frequency  = 1200*1000*1000;
            net_frequency  = 400*1000*1000;
            gmac_frequency = 125*1000*1000;
            i2s_frequency  = 1000*1000*1000;
            usb_frequency  = 150*1000*1000;
            apb_frequency  = 150*1000*1000;
            boot_frequency = 150*1000*1000;
            sdio_frequency = 150*1000*1000;
            pix_frequency  = 200*1000*1000;
            break;

        case SAMP0_CLK_SEL_SW:          // 10: 软件配置模式, PLL按照软件配置选择输出时钟;
        {
            r0 = READ_REG32(NODE_PLL0_BASE);
            r1 = READ_REG32(NODE_PLL1_BASE);

            loopc = (r0 & NODE_PLL0_LOOPC_MASK) >> NODE_PLL0_LOOPC_SHIFT;
            refc  = (r0 & NODE_PLL0_REFC_MASK) >> NODE_PLL0_REFC_SHIFT;

            /*
             * CPU 频率
             */
            if (r0 & NODE_PLL0_SEL_NODE)
            {
                odiv = (r0 & NODE_PLL0_ODIV_MASK) >> NODE_PLL0_ODIV_SHIFT;
                cpu_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            /*
             * I2S 频率
             */
            if (r0 & NODE_PLL0_SEL_I2S)
            {
                odiv = (r1 & NODE_PLL1_ODIV_I2S_MASK) >> NODE_PLL1_ODIV_I2S_SHIFT;
                i2s_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            /*
             * GMAC 频率
             */
            if (r0 & NODE_PLL0_SEL_GMAC)
            {
                odiv = r1 & NODE_PLL1_ODIV_GMAC_MASK;
                gmac_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            //-------------------------------------------------------------------------

            r0 = READ_REG32(DDR_PLL0_BASE);
            r1 = READ_REG32(DDR_PLL1_BASE);

            loopc = (r0 & DDR_PLL0_LOOPC_MASK) >> DDR_PLL0_LOOPC_SHIFT;
            refc  = (r0 & DDR_PLL0_REFC_MASK) >> DDR_PLL0_REFC_SHIFT;

            /*
             * DDR 频率
             */
            if (r0 & DDR_PLL0_SEL_DDR)
            {
                odiv = (r0 & DDR_PLL0_ODIV_MASK) >> DDR_PLL0_ODIV_SHIFT;
                ddr_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            /*
             * APB 频率
             */
            if (r0 & DDR_PLL0_SEL_DEV)
            {
                odiv = (r1 & DDR_PLL1_ODIV_DEV_MASK) >> DDR_PLL1_ODIV_DEV_SHIFT;
                apb_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            /*
             * NETWORK 频率
             */
            if (r0 & DDR_PLL0_SEL_NETWORK)
            {
                odiv = r1 & DDR_PLL1_ODIV_NETWORK_MASK;
                net_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            //-------------------------------------------------------------------------

            r0 = READ_REG32(PIX_PLL0_BASE);
            r1 = READ_REG32(PIX_PLL1_BASE);

            loopc = (r0 & PIX_PLL0_LOOPC_MASK) >> PIX_PLL0_LOOPC_SHIFT;
            refc  = (r0 & PIX_PLL0_REFC_MASK) >> PIX_PLL0_REFC_SHIFT;

            /*
             * PIX 频率
             */
            if (r0 & PIX_PLL0_SEL_PIX)
            {
                odiv = (r0 & PIX_PLL0_ODIV_MASK) >> PIX_PLL0_ODIV_SHIFT;
                pix_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            if (r0 & PIX_PLL0_SEL_GMACBP)
            {
                odiv = r1 & PIX_PLL1_ODIV_GMACBP_MASK;;
                gmacbp_frequency = OSC_FREQ / odiv * loopc / refc;
            }

            break;
        }

        case SAMP0_CLK_SEL_BYPASS:      // 11: 硬件bypass模式, PLL输出时钟全部使用外部输入系统时钟.
        default:
            cpu_frequency  = 120*1000*1000;
            ddr_frequency  = 120*1000*1000;
            net_frequency  = 120*1000*1000;
            gmac_frequency = 120*1000*1000;
            i2s_frequency  = 120*1000*1000;
            usb_frequency  = 120*1000*1000;
            apb_frequency  = 120*1000*1000;
            boot_frequency = 120*1000*1000;
            sdio_frequency = 120*1000*1000;
            pix_frequency  = 120*1000*1000;
            break;
    }

    return;
}

//-----------------------------------------------------------------------------
// stack 参数
//-----------------------------------------------------------------------------

#define STACK_PARAM     0

#if STACK_PARAM
extern unsigned char __stack_start;
extern unsigned char __stack_end;

static unsigned long g_stack_start;
static unsigned long g_stack_end;

unsigned int bsp_stack_free_size(unsigned int *totalsize)
{
    volatile register unsigned long sp_val;

    asm volatile( "move %0, $r3 ; " : "=r"(sp_val) );

    if (totalsize)
    {
        *totalsize = (unsigned int)(g_stack_end - g_stack_start);
        *totalsize -= 0x2000;
    }

    /*
     * grown down
     */
    return (unsigned int)(sp_val - g_stack_start);
}
#endif

//-----------------------------------------------------------------------------
// LS2K300 bsp start
//-----------------------------------------------------------------------------

extern int bsp_start_hook1(void);
extern int bsp_start_hook2(void);

void bsp_start(void)
{
	OS_ERR   uErr;
    uint64_t eentry;

    loongarch_interrupt_disable();

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_MERREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)machine_error_entry, 32);

    eentry = __csrrd_d(LA_CSR_EBASE);
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)exception_common_entry, 32);

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_TLBREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)tlbrefill_error_entry, 32);

    asm volatile( "dbar 0; " );
    asm volatile( "ibar 0; " );

    loongarch_init_isr_table();         /* initialize isr table */

    fetch_frequency_setting();

#if STACK_PARAM
    /*
     * 堆栈参数
     */
    g_stack_start = (unsigned long)&__stack_start;
    g_stack_end   = (unsigned long)&__stack_end;
#endif

    /**
     * Initialize uC/OS-III
     */
    OSInit(&uErr);

    /**
     * 初始化内存管理
     */
    #if 1
    {
    	extern unsigned char _end;
        extern int heap_add_region(void *addr, size_t size);
        extern void malloc_create_oslock(void);

        size_t heap_start = ((size_t)&_end + 0x10) & ~0x0FULL;
        size_t heap_end   = PHYS_TO_CACHED(get_memory_size() - 8);
        heap_end -= 4*1024*1024;					/* 保留后 4M 内存 */

        if (heap_start + 1*1024*1024 < heap_end)	/* 至少有 1M 空间 */
        {
            heap_add_region((void *)heap_start, heap_end - heap_start);
            malloc_create_oslock();
        }
        else
        {
            for ( ; ; );
        }
    }
    #endif

    bsp_start_hook1();					/* hook1: 实现文件系统初始化等 */

    console_init(115200);               /* initialize console */

    Clock_initialize();                 /* initialize ticker */

    loongarch_interrupt_enable();		/* Enable CPU Interrept */

    bsp_start_hook2();					/* hook2: 实现 EMMC, USB 初始化等 */

    //-------------------------------------------------------------------------
    // goto main function
    //-------------------------------------------------------------------------

    /*
     * 使用汇编跳转, 不再使用 main();
     */
    volatile register unsigned long mainAddr = (unsigned long)main;

    asm volatile( "move $r20, %0 ; " : "=r"(mainAddr) );
    asm volatile( "jirl $r1, $r20, 0 ;" );

    /*
     * NEVER go here!
     */
    while (1)
        ;
}

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */


