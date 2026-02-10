/*
 * Copyright (C) 2020-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef _LS1C203_IRQ_H
#define _LS1C203_IRQ_H

//-----------------------------------------------------------------------------
// 中断函数表索引, 52个
//-----------------------------------------------------------------------------

/*
 * CPU IPx Interrupts
 */
#define LS1C203_IRQ_SW0                0       // IP0
#define LS1C203_IRQ_SW1                1       // IP1
#define LS1C203_IRQ_ATIM               2       // IP2
#define LS1C203_IRQ_GTIM               3       // IP3
#define LS1C203_IRQ_ADC                4       // IP4
#define LS1C203_IRQ_TICKER             5       // IP11

/*
 * IP5 Interrupts from INTC
 */
#define LS1C203_IRQ_BTIM               6       // INTC:0
#define LS1C203_IRQ_I2C                7       // INTC:1
#define LS1C203_IRQ_UART1              8       // INTC:2
#define LS1C203_IRQ_UART0              9       // INTC:3
#define LS1C203_IRQ_FLASH              10      // INTC:4
#define LS1C203_IRQ_SPI                11      // INTC:5
#define LS1C203_IRQ_DMA0               12      // INTC:6
#define LS1C203_IRQ_DMA1               13      // INTC:7
#define LS1C203_IRQ_DMA2               14      // INTC:8
#define LS1C203_IRQ_DMA3               15      // INTC:9
#define LS1C203_IRQ_CAN                16      // INTC:10
#define LS1C203_IRQ_ACOMP              17      // INTC:11

/*
 * IP6 Interrupts from PMU
 */
#define LS1C203_IRQ_WAKE               18
#define LS1C203_IRQ_BATFAIL            19
#define LS1C203_IRQ_32KFAIL            20
#define LS1C203_IRQ_8MFAIL             21
#define LS1C203_IRQ_RTC                22

/*
 * IP7 Interrupts from ExtI
 */
#define LS1C203_IRQ_GPIO_BASE          23

#define LS1C203_IRQ_GPIO0              23
#define LS1C203_IRQ_GPIO1              24
#define LS1C203_IRQ_GPIO2              25
#define LS1C203_IRQ_GPIO3              26
#define LS1C203_IRQ_GPIO4              27
#define LS1C203_IRQ_GPIO5              28
#define LS1C203_IRQ_GPIO6              29
#define LS1C203_IRQ_GPIO7              30
#define LS1C203_IRQ_GPIO8              31
#define LS1C203_IRQ_GPIO9              32
#define LS1C203_IRQ_GPIO10             33
#define LS1C203_IRQ_GPIO11             34
#define LS1C203_IRQ_GPIO12             35
#define LS1C203_IRQ_GPIO13             36
#define LS1C203_IRQ_GPIO14             37
#define LS1C203_IRQ_GPIO15             38
#define LS1C203_IRQ_GPIO16             39
#define LS1C203_IRQ_GPIO17             40
#define LS1C203_IRQ_GPIO18             41
#define LS1C203_IRQ_GPIO19             42
#define LS1C203_IRQ_GPIO20             43
#define LS1C203_IRQ_GPIO21             44
#define LS1C203_IRQ_GPIO22             45
#define LS1C203_IRQ_GPIO23             46
#define LS1C203_IRQ_GPIO24             47
#define LS1C203_IRQ_GPIO25             48
#define LS1C203_IRQ_GPIO26             49
#define LS1C203_IRQ_GPIO27             50
#define LS1C203_IRQ_GPIO28             51

#define LS1C203_IRQ_COUNT              52

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifndef __ASSEMBLER__

//-----------------------------------------------------------------------------
// 中断句柄类型
//-----------------------------------------------------------------------------

typedef void(*irq_handler_t) (int vector, void *arg);

//-----------------------------------------------------------------------------
// 中断函数
//-----------------------------------------------------------------------------

/**
 * 安装中断
 */
int ls1c203_install_isr(int vector, irq_handler_t isr, void *arg);

/**
 * 移除中断
 */
int ls1c203_remove_isr(int vector);

#endif // #ifndef __ASSEMBLER__

#endif // _LS1C203_IRQ_H

