/*  bsp.h
 *
 *  This include file contains some definitions specific to the RBTX4925.
 *
 *  COPYRIGHT (c) 1989-2000.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  bsp.h,v 1.7.6.1 2003/09/04 18:44:49 joel Exp
 */

#ifndef _BSP_H
#define _BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bspopts.h>
#include <rtems.h>

/**
 * Define Network Support, Only has one interface.
 */
extern int rtems_ls2k_gmac_attach(struct rtems_bsdnet_ifconfig *config, int attaching);

#define LS2K_NETWORK_DRIVER_NAME		"gmac0"
#define LS2K_NETWORK_DRIVER_ATTACH		rtems_ls2k_gmac_attach

/**
 * APB Frequency
 */
extern unsigned int apb_frequency;

/**
 * delay functions
 */
extern void bsp_delay_us(int us);
extern void bsp_delay_ms(int ms);

/**
 * 根据中断向量来使能/禁止中断
 */
extern int ls2k_interrupt_enable(int vector);
extern int ls2k_interrupt_disable(int vector);

/**
 * 设置中断 route
 */
extern void ls2k_set_irq_routeip(int vector, int route_ip);

/*
 * 设置中断触发方式
 */
extern void ls2k_set_irq_triggermode(int vector, int mode);

/**
 * Board physical memory size
 */
extern unsigned get_memory_size(void);

#ifdef __cplusplus
}
#endif

#endif
