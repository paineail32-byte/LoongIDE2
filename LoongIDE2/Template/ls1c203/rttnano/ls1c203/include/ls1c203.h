/*
 * Copyright (C) 2020-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef LS1C203_H_
#define LS1C203_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define __WEAK      __attribute__((weak))

#define bit(x)      (1<<x)

/*
 *  寄存器 Read/Write 操作
 */
/*
 * 8 Bits
 */
#define READ_REG8(Addr)			(*(volatile unsigned char*)(Addr))
#define WRITE_REG8(Addr, Val)	(*(volatile unsigned char*)(Addr) = (Val))
#define OR_REG8(Addr, Val)		(*(volatile unsigned char*)(Addr) |= (Val))
#define AND_REG8(Addr, Val)	    (*(volatile unsigned char*)(Addr) &= (Val))

/*
 * 16 Bits
 */
#define READ_REG16(Addr) 		(*(volatile unsigned short*)(Addr))
#define WRITE_REG16(Addr, Val)	(*(volatile unsigned short*)(Addr) = (Val))
#define OR_REG16(Addr, Val)	    (*(volatile unsigned short*)(Addr) |= (Val))
#define AND_REG16(Addr, Val)	(*(volatile unsigned short*)(Addr) &= (Val))

/*
 * 32 Bits
 */
#define READ_REG32(Addr) 		(*(volatile unsigned int*)(Addr))
#define WRITE_REG32(Addr, Val)	(*(volatile unsigned int*)(Addr) = (Val))
#define OR_REG32(Addr, Val)	    (*(volatile unsigned int*)(Addr) |= (Val))
#define AND_REG32(Addr, Val)	(*(volatile unsigned int*)(Addr) &= (Val))

//-----------------------------------------------------------------------------
// 片上设备头文件
//-----------------------------------------------------------------------------

#include "ls1c203_irq.h"

#include "ls1c203_conf_hw.h"
#include "ls1c203_pmu_hw.h"
#include "ls1c203_io_hw.h"

extern HW_CONF_t *g_conf;
extern HW_PMU_t  *g_pmu;

/**
 * 工作频率
 */
extern unsigned int cpu_frequency;          // CPU  工作频率
extern unsigned int bus_frequency;          // BUS  片上总线时钟
extern unsigned int eapb_frequency;         // EAPB 常开域总线时钟

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LS1C203_H_ */


