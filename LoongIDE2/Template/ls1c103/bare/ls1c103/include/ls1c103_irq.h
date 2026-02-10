/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef _LS1C103_IRQ_H
#define _LS1C103_IRQ_H

//-------------------------------------------------------------------------------------------------
// 中断句柄类型
//-------------------------------------------------------------------------------------------------

typedef void(*irq_handler_t) (int vector, void *arg);

//-------------------------------------------------------------------------------------------------
// 中断函数表索引, 50个
//-------------------------------------------------------------------------------------------------

/* CPU IP Interrupts */
#define LS1C103_IRQ_SW0                0       // IP0
#define LS1C103_IRQ_SW1                1       // IP1
#define LS1C103_IRQ_ATIM               2       // IP2
#define LS1C103_IRQ_GTIM               3       // IP3
#define LS1C103_IRQ_ADC                4       // IP4
#define LS1C103_IRQ_TICKER             5       // IP11

/* IP5 Interrupts from INTC */
#define LS1C103_IRQ_BTIM               6       // INTC:0
#define LS1C103_IRQ_I2C                7       // INTC:1
#define LS1C103_IRQ_UART1              8       // INTC:2
#define LS1C103_IRQ_UART0              9       // INTC:3
#define LS1C103_IRQ_FLASH              10      // INTC:4
#define LS1C103_IRQ_SPI                11      // INTC:5
#define LS1C103_IRQ_DMA0               12      // INTC:6
#define LS1C103_IRQ_DMA1               13      // INTC:7
#define LS1C103_IRQ_DMA2               14      // INTC:8
#define LS1C103_IRQ_DMA3               15      // INTC:9

/* IP6 Interrupts from PMU */
#define LS1C103_IRQ_WAKE               16
#define LS1C103_IRQ_BATFAIL            17
#define LS1C103_IRQ_32KFAIL            18
#define LS1C103_IRQ_8MFAIL             19
#define LS1C103_IRQ_RTC                20

/* IP7 Interrupts from ExtINT */
#define LS1C103_IRQ_GPIO_BASE          21
#define LS1C103_IRQ_GPIO0              21
#define LS1C103_IRQ_GPIO1              22
#define LS1C103_IRQ_GPIO2              23
#define LS1C103_IRQ_GPIO3              24
#define LS1C103_IRQ_GPIO4              25
#define LS1C103_IRQ_GPIO5              26
#define LS1C103_IRQ_GPIO6              27
#define LS1C103_IRQ_GPIO7              28
#define LS1C103_IRQ_GPIO8              29
#define LS1C103_IRQ_GPIO9              30
#define LS1C103_IRQ_GPIO10             31
#define LS1C103_IRQ_GPIO11             32
#define LS1C103_IRQ_GPIO12             33
#define LS1C103_IRQ_GPIO13             34
#define LS1C103_IRQ_GPIO14             35
#define LS1C103_IRQ_GPIO15             36
#define LS1C103_IRQ_GPIO16             37
#define LS1C103_IRQ_GPIO17             38
#define LS1C103_IRQ_GPIO18             39
#define LS1C103_IRQ_GPIO19             40
#define LS1C103_IRQ_GPIO20             41
#define LS1C103_IRQ_GPIO21             42
#define LS1C103_IRQ_GPIO22             43
#define LS1C103_IRQ_GPIO23             44
#define LS1C103_IRQ_GPIO24             45
#define LS1C103_IRQ_GPIO25             46
#define LS1C103_IRQ_GPIO26             47
#define LS1C103_IRQ_GPIO27             48
#define LS1C103_IRQ_GPIO28             49

#define LS1C103_IRQ_COUNT              50

//-------------------------------------------------------------------------------------------------
// 中断函数
//-------------------------------------------------------------------------------------------------

/**
 * 安装中断
 */
int ls1c103_install_isr(int vector, irq_handler_t isr, void *arg);

/**
 * 移除中断
 */
int ls1c103_remove_isr(int vector);


#endif // _LS1C103_IRQ_H

