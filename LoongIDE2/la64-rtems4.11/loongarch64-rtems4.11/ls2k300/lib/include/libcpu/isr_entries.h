/**
 *  @file
 *  
 */

/*
 *  COPYRIGHT (c) 1989-2012.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#ifndef _ISR_ENTRIES_H
#define _ISR_ENTRIES_H 1

extern void loongarch_install_isr_entries(void);
extern void loongarch_vector_isr_handlers(CPU_Interrupt_frame *frame);

extern void machine_error_entry(void);
extern void exception_common_entry(void);
extern void tlbrefill_error_entry(void);

#endif
