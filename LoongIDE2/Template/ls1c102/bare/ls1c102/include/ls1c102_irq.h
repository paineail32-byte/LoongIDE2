/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef _LS1C102_IRQ_H
#define _LS1C102_IRQ_H

//-------------------------------------------------------------------------------------------------
// 中断句柄类型
//-------------------------------------------------------------------------------------------------

typedef void(*irq_handler_t) (int vector, void *arg);

//-------------------------------------------------------------------------------------------------
// 中断函数表索引, 50个
//-------------------------------------------------------------------------------------------------

/**
 * CPU IP Interrupts
 */
#define LS1C102_IRQ_SW0                 0       // IP0
#define LS1C102_IRQ_SW1                 1       // IP1
#define LS1C102_IRQ_WAKEUP              2       // IP2
#define LS1C102_IRQ_TOUCH               3       // IP3
#define LS1C102_IRQ_UART2               4       // IP4
#define LS1C102_IRQ_TICKER              5       // IP11

/**
 * IP5 Interrupts from PMU
 */
#define LS1C102_IRQ_BATFAIL             6
#define LS1C102_IRQ_32KFAIL             7
#define LS1C102_IRQ_8MFAIL              8
#define LS1C102_IRQ_RTC                 9
#define LS1C102_IRQ_ADC                 10

/**
 * IP6 Interrupts from INTC
 */
#define LS1C102_IRQ_TIMER               11      // INTC:0
#define LS1C102_IRQ_I2C                 12      // INTC:1
#define LS1C102_IRQ_UART1               13      // INTC:2
#define LS1C102_IRQ_UART0               14      // INTC:3
#define LS1C102_IRQ_FLASH               15      // INTC:4
#define LS1C102_IRQ_SPI                 16      // INTC:5
#define LS1C102_IRQ_VPWM                17      // INTC:6
#define LS1C102_IRQ_DMA                 18      // INTC:7

/**
 * IP7 Interrupts from ExtINT
 */
#define LS1C102_IRQ_GPIO_BASE           19
#define LS1C102_IRQ_GPIO0               19      // GPIO[7:0]
#define LS1C102_IRQ_GPIO1               20
#define LS1C102_IRQ_GPIO2               21
#define LS1C102_IRQ_GPIO3               22
#define LS1C102_IRQ_GPIO4               23
#define LS1C102_IRQ_GPIO5               24
#define LS1C102_IRQ_GPIO6               25
#define LS1C102_IRQ_GPIO7               26
#define LS1C102_IRQ_GPIO16              27      // GPIO[23:16]
#define LS1C102_IRQ_GPIO17              28
#define LS1C102_IRQ_GPIO18              29
#define LS1C102_IRQ_GPIO19              30
#define LS1C102_IRQ_GPIO20              31
#define LS1C102_IRQ_GPIO21              32
#define LS1C102_IRQ_GPIO22              33
#define LS1C102_IRQ_GPIO23              34
#define LS1C102_IRQ_GPIO32              35      // GPIO[39:32]
#define LS1C102_IRQ_GPIO33              36
#define LS1C102_IRQ_GPIO34              37
#define LS1C102_IRQ_GPIO35              38
#define LS1C102_IRQ_GPIO36              39
#define LS1C102_IRQ_GPIO37              40
#define LS1C102_IRQ_GPIO38              41
#define LS1C102_IRQ_GPIO39              42
#define LS1C102_IRQ_GPIO48              43      // GPIO[55:48]
#define LS1C102_IRQ_GPIO49              44
#define LS1C102_IRQ_GPIO50              45
#define LS1C102_IRQ_GPIO51              46
#define LS1C102_IRQ_GPIO52              47
#define LS1C102_IRQ_GPIO53              48
#define LS1C102_IRQ_GPIO54              49
#define LS1C102_IRQ_GPIO55              50

#define LS1C102_IRQ_COUNT               51

//-------------------------------------------------------------------------------------------------
// 中断函数
//-------------------------------------------------------------------------------------------------

/**
 * 安装中断
 */
int ls1c102_install_isr(int vector, irq_handler_t isr, void *arg);

/**
 * 移除中断
 */
int ls1c102_remove_isr(int vector);


#endif // _LS1C102_IRQ_H

