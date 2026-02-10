/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * err_c.c
 *
 * created: 2022-04-21
 *  author: 
 */

#include <stdio.h>
#include <larchintrin.h>

#include "regdef.h"
#include "ls2k500.h"
#include "ls2k500_irq.h"

//-------------------------------------------------------------------------------------------------

extern void printk(const char *fmt, ...);

extern void dump_registers(void *stack);

//-----------------------------------------------------------------------------
// 机器错误处理, 安装到: MERRENTRY
//-----------------------------------------------------------------------------

void c_machine_error_handler(uint64_t *stack)
{
    uint64_t tmp;
    
    printk("machine error occurred!\r\n");

    /**
     * Machine error registers
     */
    tmp = __csrrd_d(LA_CSR_MERRCTL);
	printk("errctl  = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_MERRINFO1);
	printk("info1   = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_MERRINFO2);
	printk("info2   = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_MERREPC);
	printk("merrepc = 0x%016lx\r\n\r\n", tmp);

    dump_registers(stack);

    /*
     * dead loop
     */
    for (;;);

    /*
     * NEVER GO HERE!!!
     */
}

//-----------------------------------------------------------------------------
// TLB 重填错误处理, 安装到: TLBRENTRY
//-----------------------------------------------------------------------------

void c_tlbrefill_error_handler(uint64_t *stack)
{
    uint64_t tmp;

    printk("tlb-refill error occurred!\r\n");
    
    /**
     * TLB refill registers
     */
    tmp = __csrrd_d(LA_CSR_TLBRBADV);
	printk("tlbrbadv = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_TLBREPC);
	printk("tlbrepc  = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_TLBRELO0);
	printk("tlbrelo0 = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_TLBRELO1);
	printk("tlbrelo1 = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_TLBREHI);
	printk("tlbrehi  = 0x%016lx\r\n", tmp);
    tmp = __csrrd_d(LA_CSR_TLBRPRMD);
	printk("tlbrprmd = 0x%016lx\r\n\r\n", tmp);

    dump_registers(stack);

    /*
     * dead loop
     */
    for (;;);

    /*
     * NEVER GO HERE!!!
     */
}



