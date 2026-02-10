/**
 *  @file
 *
 *  @brief Loongarch CPU Dependent Header File
 */

#ifndef _RTEMS_SCORE_CPU_H
#define _RTEMS_SCORE_CPU_H

/**
 *  @defgroup ScoreCPU CPU CPU
 *
 *  @ingroup Score
 *
 */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

#include <rtems/score/types.h>
#include <rtems/score/loongarch64.h>

/* conditional compilation parameters */

/*
 *  Should the calls to _Thread_Enable_dispatch be inlined?
 *
 *  If TRUE, then they are inlined.
 *  If FALSE, then a subroutine call is made.
 *
 *  Basically this is an example of the classic trade-off of size
 *  versus speed.  Inlining the call (TRUE) typically increases the
 *  size of RTEMS while speeding up the enabling of dispatching.
 *  [NOTE: In general, the _Thread_Dispatch_disable_level will
 *  only be 0 or 1 unless you are in an interrupt handler and that
 *  interrupt handler invokes the executive.]  When not inlined
 *  something calls _Thread_Enable_dispatch which in turns calls
 *  _Thread_Dispatch.  If the enable dispatch is inlined, then
 *  one subroutine call is avoided entirely.]
 */

#define CPU_INLINE_ENABLE_DISPATCH       	FALSE

/*
 *  Does RTEMS manage a dedicated interrupt stack in software?
 *
 *  If TRUE, then a stack is allocated in _Interrupt_Manager_initialization.
 *  If FALSE, nothing is done.
 *
 *  If the CPU supports a dedicated interrupt stack in hardware,
 *  then it is generally the responsibility of the BSP to allocate it
 *  and set it up.
 *
 *  If the CPU does not support a dedicated interrupt stack, then
 *  the porter has two options: (1) execute interrupts on the
 *  stack of the interrupted task, and (2) have RTEMS manage a dedicated
 *  interrupt stack.
 *
 *  If this is TRUE, CPU_ALLOCATE_INTERRUPT_STACK should also be TRUE.
 *
 *  Only one of CPU_HAS_SOFTWARE_INTERRUPT_STACK and
 *  CPU_HAS_HARDWARE_INTERRUPT_STACK should be set to TRUE.  It is
 *  possible that both are FALSE for a particular CPU.  Although it
 *  is unclear what that would imply about the interrupt processing
 *  procedure on that CPU.
 */

#define CPU_HAS_SOFTWARE_INTERRUPT_STACK 	FALSE

/*
 *  Does the CPU follow the simple vectored interrupt model?
 *
 *  If TRUE, then RTEMS allocates the vector table it internally manages.
 *  If FALSE, then the BSP is assumed to allocate and manage the vector
 *  table
 *
 *  LOONGARCH Specific Information:
 *
 *  Up to and including RTEMS 4.10, the LOONGARCH port used simple vectored
 *  interrupts. But this was changed to the PIC model after 4.10.
 */
#define CPU_SIMPLE_VECTORED_INTERRUPTS 		FALSE

/*
 *  Does this CPU have hardware support for a dedicated interrupt stack?
 *
 *  If TRUE, then it must be installed during initialization.
 *  If FALSE, then no installation is performed.
 *
 *  If this is TRUE, CPU_ALLOCATE_INTERRUPT_STACK should also be TRUE.
 *
 *  Only one of CPU_HAS_SOFTWARE_INTERRUPT_STACK and
 *  CPU_HAS_HARDWARE_INTERRUPT_STACK should be set to TRUE.  It is
 *  possible that both are FALSE for a particular CPU.  Although it
 *  is unclear what that would imply about the interrupt processing
 *  procedure on that CPU.
 */

#define CPU_HAS_HARDWARE_INTERRUPT_STACK 	FALSE

/*
 *  Does RTEMS allocate a dedicated interrupt stack in the Interrupt Manager?
 *
 *  If TRUE, then the memory is allocated during initialization.
 *  If FALSE, then the memory is allocated during initialization.
 *
 *  This should be TRUE is CPU_HAS_SOFTWARE_INTERRUPT_STACK is TRUE.
 */

#define CPU_ALLOCATE_INTERRUPT_STACK 		FALSE

/*
 *  Does the RTEMS invoke the user's ISR with the vector number and
 *  a pointer to the saved interrupt frame (1) or just the vector
 *  number (0)?
 *
 */

#define CPU_ISR_PASSES_FRAME_POINTER 		1

/*
 *  Does the CPU have hardware floating point?
 *
 *  If TRUE, then the RTEMS_FLOATING_POINT task attribute is supported.
 *  If FALSE, then the RTEMS_FLOATING_POINT task attribute is ignored.
 *
 *  If there is a FP coprocessor such as the i387 or mc68881, then
 *  the answer is TRUE.
 *
 *  The macro name "LOONGARCH_HAS_FPU" should be made CPU specific.
 *  It indicates whether or not this CPU model has FP support.  For
 *  example, it would be possible to have an i386_nofp CPU model
 *  which set this to false to indicate that you have an i386 without
 *  an i387 and wish to leave floating point support out of RTEMS.
 */

#if (LOONGARCH_HAS_FPU == 1)
#define CPU_HARDWARE_FP   	TRUE
#else
#define CPU_HARDWARE_FP     FALSE
#endif

/*
 *  Are all tasks RTEMS_FLOATING_POINT tasks implicitly?
 *
 *  If TRUE, then the RTEMS_FLOATING_POINT task attribute is assumed.
 *  If FALSE, then the RTEMS_FLOATING_POINT task attribute is followed.
 *
 *  So far, the only CPU in which this option has been used is the
 *  HP PA-RISC.  The HP C compiler and gcc both implicitly use the
 *  floating point registers to perform integer multiplies.  If
 *  a function which you would not think utilize the FP unit DOES,
 *  then one can not easily predict which tasks will use the FP hardware.
 *  In this case, this option should be TRUE.
 *
 *  If CPU_HARDWARE_FP is FALSE, then this should be FALSE as well.
 *
 *  Mips Note: It appears the GCC can implicitly generate FPU
 *  and Altivec instructions when you least expect them.  So make
 *  all tasks floating point.
 */

#define CPU_ALL_TASKS_ARE_FP 			CPU_HARDWARE_FP

/*
 *  Should the IDLE task have a floating point context?
 *
 *  If TRUE, then the IDLE task is created as a RTEMS_FLOATING_POINT task
 *  and it has a floating point context which is switched in and out.
 *  If FALSE, then the IDLE task does not have a floating point context.
 *
 *  Setting this to TRUE negatively impacts the time required to preempt
 *  the IDLE task from an interrupt because the floating point context
 *  must be saved as part of the preemption.
 */

#define CPU_IDLE_TASK_IS_FP      		FALSE

/*
 *  Should the saving of the floating point registers be deferred
 *  until a context switch is made to another different floating point
 *  task?
 *
 *  If TRUE, then the floating point context will not be stored until
 *  necessary.  It will remain in the floating point registers and not
 *  disturned until another floating point task is switched to.
 *
 *  If FALSE, then the floating point context is saved when a floating
 *  point task is switched out and restored when the next floating point
 *  task is restored.  The state of the floating point registers between
 *  those two operations is not specified.
 *
 *  If the floating point context does NOT have to be saved as part of
 *  interrupt dispatching, then it should be safe to set this to TRUE.
 *
 *  Setting this flag to TRUE results in using a different algorithm
 *  for deciding when to save and restore the floating point context.
 *  The deferred FP switch algorithm minimizes the number of times
 *  the FP context is saved and restored.  The FP context is not saved
 *  until a context switch is made to another, different FP task.
 *  Thus in a system with only one FP task, the FP context will never
 *  be saved or restored.
 */

#define CPU_USE_DEFERRED_FP_SWITCH      TRUE

/*
 *  Does this port provide a CPU dependent IDLE task implementation?
 *
 *  If TRUE, then the routine _CPU_Internal_threads_Idle_thread_body
 *  must be provided and is the default IDLE thread body instead of
 *  _Internal_threads_Idle_thread_body.
 *
 *  If FALSE, then use the generic IDLE thread body if the BSP does
 *  not provide one.
 *
 *  This is intended to allow for supporting processors which have
 *  a low power or idle mode.  When the IDLE thread is executed, then
 *  the CPU can be powered down.
 *
 *  The order of precedence for selecting the IDLE thread body is:
 *
 *    1.  BSP provided
 *    2.  CPU dependent (if provided)
 *    3.  generic (if no BSP and no CPU dependent)
 */

/* we can use the low power wait instruction for the IDLE thread */
#define CPU_PROVIDES_IDLE_THREAD_BODY   TRUE

/*
 *  Does the stack grow up (toward higher addresses) or down
 *  (toward lower addresses)?
 *
 *  If TRUE, then the grows upward.
 *  If FALSE, then the grows toward smaller addresses.
 */

/* our stack grows down */
#define CPU_STACK_GROWS_UP              FALSE

/*
 *  The following is the variable attribute used to force alignment of critical
 *  RTEMS structures. On some processors it may make sense to have these aligned
 *  on tighter boundaries than the minimum requirements of the compiler in order
 *  to have as much of the critical data area as possible in a cache line.
 *
 *  The placement of this macro in the declaration of the variables is based on
 *  the syntactically requirements of the GNU C "__attribute__" extension.
 *  For example with GNU C, use the following to force a structures to a 32 byte
 *  boundary.
 *
 *      __attribute__ ((aligned (32)))
 *
 *  NOTE:  Currently only the Priority Bit Map table uses this feature.
 *         To benefit from using this, the data must be heavily used so it will
 *         stay in the cache and used frequently enough in the executive to
 *         justify turning this on.
 */

/* our cache line size is 64 bytes */
#if __GNUC__
#define CPU_STRUCTURE_ALIGNMENT 		__attribute__ ((aligned(64)))
#else
#define CPU_STRUCTURE_ALIGNMENT
#endif

#define CPU_TIMESTAMP_USE_INT64_INLINE	TRUE

/*
 *  Define what is required to specify how the network to host conversion
 *  routines are handled.
 */

/* LOONGARCH is always Little-Endian */

#define CPU_BIG_ENDIAN               	FALSE
#define CPU_LITTLE_ENDIAN             	TRUE

/*
 *  The following defines the number of bits actually used in the
 *  interrupt field of the task mode.  How those bits map to the
 *  CPU interrupt levels is defined by the routine _CPU_ISR_Set_level().
 */

#define CPU_MODES_INTERRUPT_MASK   		0x000000ff

#if __loongarch64
#define CPU_SIZEOF_POINTER 				8
#else
#define CPU_SIZEOF_POINTER 				4
#endif

#define CPU_PER_CPU_CONTROL_SIZE 		0

/*
 *  Processor defined structures
 *
 *  Examples structures include the descriptor tables from the i386
 *  and the processor control structure on the i960ca.
 */

/* may need to put some structures here.  */

/*
 * Contexts
 *
 *  Generally there are 2 types of context to save.
 *     1. Interrupt registers to save
 *     2. Task level registers to save
 *
 *  This means we have the following 3 context items:
 *     1. task level context stuff::  Context_Control
 *     2. floating point task stuff:: Context_Control_fp
 *     3. special interrupt level context :: Context_Control_interrupt
 *
 *  On some processors, it is cost-effective to save only the callee
 *  preserved registers during a task context switch.  This means
 *  that the ISR code needs to save those registers which do not
 *  persist across function calls.  It is not mandatory to make this
 *  distinctions between the caller/callee saves registers for the
 *  purpose of minimizing context saved during task switch and on interrupts.
 *  If the cost of saving extra registers is minimal, simplicity is the
 *  choice.  Save the same context on interrupt entry as for tasks in
 *  this case.
 *
 *  Additionally, if gdb is to be made aware of RTEMS tasks for this CPU, then
 *  care should be used in designing the context area.
 *
 *  On some CPUs with hardware floating point support, the Context_Control_fp
 *  structure will not be used or it simply consist of an array of a
 *  fixed number of bytes.   This is done when the floating point context
 *  is dumped by a "FP save context" type instruction and the format
 *  is not really defined by the CPU.  In this case, there is no need
 *  to figure out the exact format -- only the size.  Of course, although
 *  this is enough information for RTEMS, it is probably not enough for
 *  a debugger such as gdb.  But that is another problem.
 */

#ifndef ASM

#include <larchintrin.h>

/*
 * WARNING: If this structure is modified, the constants in cpu.h must be updated.
 */
#if __loongarch64
#define __LOONGARCH_REGISTER_TYPE 		uint64_t
#define __LOONGARCH_FPU_REGISTER_TYPE 	uint64_t
#else
#define __LOONGARCH_REGISTER_TYPE     	uint32_t
#define __LOONGARCH_FPU_REGISTER_TYPE 	uint32_t
#endif

typedef struct
{
	__LOONGARCH_REGISTER_TYPE x;    	// callee register?
    __LOONGARCH_REGISTER_TYPE s0;
    __LOONGARCH_REGISTER_TYPE s1;
    __LOONGARCH_REGISTER_TYPE s2;
    __LOONGARCH_REGISTER_TYPE s3;
    __LOONGARCH_REGISTER_TYPE s4;
    __LOONGARCH_REGISTER_TYPE s5;
    __LOONGARCH_REGISTER_TYPE s6;
    __LOONGARCH_REGISTER_TYPE s7;
    __LOONGARCH_REGISTER_TYPE s8;
    __LOONGARCH_REGISTER_TYPE gp;		// need save?
    __LOONGARCH_REGISTER_TYPE sp;
    __LOONGARCH_REGISTER_TYPE fp;
    __LOONGARCH_REGISTER_TYPE ra;
    __LOONGARCH_REGISTER_TYPE euen;
    __LOONGARCH_REGISTER_TYPE ecfg;
    __LOONGARCH_REGISTER_TYPE crmd;
    __LOONGARCH_REGISTER_TYPE epc;
} Context_Control;

#define _CPU_Context_Get_SP(_context) \
	(uintptr_t)(_context)->sp

/* WARNING: If this structure is modified, the constants in cpu.h
 *          must also be updated.
 */

typedef struct
{
#if (CPU_HARDWARE_FP == TRUE)
    __LOONGARCH_FPU_REGISTER_TYPE fp0;
    __LOONGARCH_FPU_REGISTER_TYPE fp1;
    __LOONGARCH_FPU_REGISTER_TYPE fp2;
    __LOONGARCH_FPU_REGISTER_TYPE fp3;
    __LOONGARCH_FPU_REGISTER_TYPE fp4;
    __LOONGARCH_FPU_REGISTER_TYPE fp5;
    __LOONGARCH_FPU_REGISTER_TYPE fp6;
    __LOONGARCH_FPU_REGISTER_TYPE fp7;
    __LOONGARCH_FPU_REGISTER_TYPE fp8;
    __LOONGARCH_FPU_REGISTER_TYPE fp9;
    __LOONGARCH_FPU_REGISTER_TYPE fp10;
    __LOONGARCH_FPU_REGISTER_TYPE fp11;
    __LOONGARCH_FPU_REGISTER_TYPE fp12;
    __LOONGARCH_FPU_REGISTER_TYPE fp13;
    __LOONGARCH_FPU_REGISTER_TYPE fp14;
    __LOONGARCH_FPU_REGISTER_TYPE fp15;
    __LOONGARCH_FPU_REGISTER_TYPE fp16;
    __LOONGARCH_FPU_REGISTER_TYPE fp17;
    __LOONGARCH_FPU_REGISTER_TYPE fp18;
    __LOONGARCH_FPU_REGISTER_TYPE fp19;
    __LOONGARCH_FPU_REGISTER_TYPE fp20;
    __LOONGARCH_FPU_REGISTER_TYPE fp21;
    __LOONGARCH_FPU_REGISTER_TYPE fp22;
    __LOONGARCH_FPU_REGISTER_TYPE fp23;
    __LOONGARCH_FPU_REGISTER_TYPE fp24;
    __LOONGARCH_FPU_REGISTER_TYPE fp25;
    __LOONGARCH_FPU_REGISTER_TYPE fp26;
    __LOONGARCH_FPU_REGISTER_TYPE fp27;
    __LOONGARCH_FPU_REGISTER_TYPE fp28;
    __LOONGARCH_FPU_REGISTER_TYPE fp29;
    __LOONGARCH_FPU_REGISTER_TYPE fp30;
    __LOONGARCH_FPU_REGISTER_TYPE fp31;
	__LOONGARCH_FPU_REGISTER_TYPE fcc0;
	__LOONGARCH_FPU_REGISTER_TYPE fcc1;
	__LOONGARCH_FPU_REGISTER_TYPE fcc2;
	__LOONGARCH_FPU_REGISTER_TYPE fcc3;
	__LOONGARCH_FPU_REGISTER_TYPE fcc4;
	__LOONGARCH_FPU_REGISTER_TYPE fcc5;
	__LOONGARCH_FPU_REGISTER_TYPE fcc6;
	__LOONGARCH_FPU_REGISTER_TYPE fcc7;
    __LOONGARCH_FPU_REGISTER_TYPE fpcsr0;
#endif
} Context_Control_fp;

/*
 *  This struct reflects the stack frame employed in ISR_Handler.  Note
 *  that the ISR routine save some of the registers to this frame for
 *  all interrupts and exceptions.  Other registers are saved only on
 *  exceptions, while others are not touched at all.  The untouched
 *  registers are not normally disturbed by high-level language
 *  programs so they can be accessed when required.
 *
 *  The registers and their ordering in this struct must directly
 *  correspond to the layout and ordering of * shown in iregdef.h,
 *  as cpu_asm.S uses those definitions to fill the stack frame.
 *  This struct provides access to the stack frame for C code.
 *
 *  Similarly, this structure is used by debugger stubs and exception
 *  processing routines so be careful when changing the format.
 *
 *  NOTE: The comments with this structure and cpu_asm.S should be kept
 *        in sync.  When in doubt, look in the  code to see if the
 *        registers you're interested in are actually treated as expected.
 *        The order of the first portion of this structure follows the
 *        order of registers expected by gdb.
 */

typedef struct
{
	/**
	 * General registers
	 */
	__LOONGARCH_REGISTER_TYPE  r0;       	/*  0 -- NOT FILLED IN, zero */
	__LOONGARCH_REGISTER_TYPE  ra;       	/*  1 -- saved always */
	__LOONGARCH_REGISTER_TYPE  gp;       	/*  2 -- saved always */
	__LOONGARCH_REGISTER_TYPE  sp;       	/*  3 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a0;      	/*  4 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a1;       	/*  5 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a2;       	/*  6 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a3;       	/*  7 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a4;       	/*  8 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a5;       	/*  9 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a6;       	/* 10 -- saved always */
	__LOONGARCH_REGISTER_TYPE  a7;       	/* 11 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t0;       	/* 12 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t1;       	/* 13 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t2;       	/* 14 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t3;       	/* 15 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t4;       	/* 16 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t5;       	/* 17 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t6;       	/* 18 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t7;       	/* 19 -- saved always */
	__LOONGARCH_REGISTER_TYPE  t8;       	/* 20 -- saved always */
	__LOONGARCH_REGISTER_TYPE  x;        	/* 21 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  fp;       	/* 22 -- saved always */
	__LOONGARCH_REGISTER_TYPE  s0;       	/* 23 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s1;       	/* 24 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s2;       	/* 25 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s3;       	/* 26 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s4;       	/* 27 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s5;       	/* 28 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s6;       	/* 29 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s7;       	/* 30 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  s8;       	/* 31 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  crmd;     	/* 32 -- saved always */
	__LOONGARCH_REGISTER_TYPE  prmd;     	/* 33 -- saved always */
	__LOONGARCH_REGISTER_TYPE  ecfg;     	/* 34 -- saved always, some bits are manipulated per-thread */
	__LOONGARCH_REGISTER_TYPE  estat;    	/* 35 -- saved on exceptions, read-only */
	__LOONGARCH_REGISTER_TYPE  euen;     	/* 36 -- saved always, some bits are manipulated per-thread */
	__LOONGARCH_REGISTER_TYPE  badv;     	/* 37 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  badi;     	/* 38 -- saved on exceptions */
	__LOONGARCH_REGISTER_TYPE  epc;      	/* 39 -- saved always, read-only register but logically restored */

	/**
	 * Float registers
	 */
	__LOONGARCH_FPU_REGISTER_TYPE f0;    	/* 40 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f1;    	/* 41 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f2;    	/* 42 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f3;    	/* 43 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f4;    	/* 44 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f5;    	/* 45 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f6;    	/* 46 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f7;    	/* 47 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f8;    	/* 48 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f9;    	/* 49 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f10;   	/* 50 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f11;   	/* 51 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f12;   	/* 52 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f13;   	/* 53 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f14;   	/* 54 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f15;   	/* 55 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f16;   	/* 56 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f17;   	/* 57 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f18;   	/* 58 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f19;   	/* 59 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f20;   	/* 60 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f21;   	/* 61 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f22;   	/* 62 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f23;   	/* 63 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f24;   	/* 64 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f25;   	/* 65 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f26;   	/* 66 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f27;   	/* 67 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f28;   	/* 68 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f29;   	/* 69 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f30;   	/* 70 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE f31;   	/* 71 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc0;  	/* 72 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc1;  	/* 73 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc2;  	/* 74 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc3;  	/* 75 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc4; 	/* 76 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc5;  	/* 77 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc6;  	/* 78 -- saved if FP enabled */
	__LOONGARCH_FPU_REGISTER_TYPE fcc7;  	/* 79 -- saved if FP enabled */
	__LOONGARCH_REGISTER_TYPE     fcsr0; 	/* 80 -- saved if FP enabled */

	/**
	 * NREGS == 81
	 */

} CPU_Interrupt_frame;

typedef CPU_Interrupt_frame 		CPU_Exception_frame;

/*
 *  This variable is optional.  It is used on CPUs on which it is difficult
 *  to generate an "uninitialized" FP context.  It is filled in by
 *  _CPU_Initialize and copied into the task's FP context area during
 *  _CPU_Context_Initialize.
 */

SCORE_EXTERN Context_Control_fp 	_CPU_Null_fp_context;

/*
 *  Nothing prevents the porter from declaring more CPU specific variables.
 */

/* XXX: if needed, put more variables here */

/*
 *  The size of the floating point context area.  On some CPUs this
 *  will not be a "sizeof" because the format of the floating point
 *  area is not defined -- only the size is.  This is usually on
 *  CPUs with a "floating point save context" instruction.
 */

#define CPU_CONTEXT_FP_SIZE 		sizeof(Context_Control_fp)

/*
 *  Amount of extra stack (above minimum stack size) required by
 *  system initialization thread.  Remember that in a multiprocessor
 *  system the system intialization thread becomes the MP server thread.
 */

#define CPU_MPCI_RECEIVE_SERVER_EXTRA_STACK 0

/*
 *  Should be large enough to run all RTEMS tests.  This ensures
 *  that a "reasonable" small application should not have any problems.
 */
#define CPU_STACK_MINIMUM_SIZE     (8*1024)

/*
 *  CPU's worst alignment requirement for data types on a byte boundary.  This
 *  alignment does not take into account the requirements for the stack.
 */
#if __loongarch64
#define CPU_ALIGNMENT              16
#else
#define CPU_ALIGNMENT              8
#endif

/*
 *  This number corresponds to the byte alignment requirement for the
 *  heap handler.  This alignment requirement may be stricter than that
 *  for the data types alignment specified by CPU_ALIGNMENT.  It is
 *  common for the heap to follow the same alignment requirement as
 *  CPU_ALIGNMENT.  If the CPU_ALIGNMENT is strict enough for the heap,
 *  then this should be set to CPU_ALIGNMENT.
 *
 *  NOTE:  This does not have to be a power of 2.  It does have to
 *         be greater or equal to than CPU_ALIGNMENT.
 */

#define CPU_HEAP_ALIGNMENT         CPU_ALIGNMENT

/*
 *  This number corresponds to the byte alignment requirement for memory
 *  buffers allocated by the partition manager.  This alignment requirement
 *  may be stricter than that for the data types alignment specified by
 *  CPU_ALIGNMENT.  It is common for the partition to follow the same
 *  alignment requirement as CPU_ALIGNMENT.  If the CPU_ALIGNMENT is strict
 *  enough for the partition, then this should be set to CPU_ALIGNMENT.
 *
 *  NOTE:  This does not have to be a power of 2.  It does have to
 *         be greater or equal to than CPU_ALIGNMENT.
 */

#define CPU_PARTITION_ALIGNMENT    CPU_ALIGNMENT

/*
 *  This number corresponds to the byte alignment requirement for the
 *  stack.  This alignment requirement may be stricter than that for the
 *  data types alignment specified by CPU_ALIGNMENT.  If the CPU_ALIGNMENT
 *  is strict enough for the stack, then this should be set to 0.
 *
 *  NOTE:  This must be a power of 2 either 0 or greater than CPU_ALIGNMENT.
 */

#define CPU_STACK_ALIGNMENT        CPU_ALIGNMENT

void loongarch_vector_exceptions(CPU_Interrupt_frame *frame);

/*
 *  ISR handler macros
 */

/*
 *  Declare the function that is present in the shared libcpu directory,
 *  that returns the processor dependent interrupt mask.
 */

uint32_t loongarch_interrupt_mask(void);

/*
 *  Disable all interrupts for an RTEMS critical section.  The previous
 *  level is returned in _level.
 */

#define _CPU_ISR_Disable(_level) \
	do { \
		unsigned int _scratch; \
		loongarch_get_crmd(_scratch); \
		loongarch_set_crmd(_scratch & ~LOONGARCH_IE_BITS); \
		_level = _scratch & LOONGARCH_IE_BITS; \
	} while(0)

/*
 *  Enable interrupts to the previous level (returned by _CPU_ISR_Disable).
 *  This indicates the end of an RTEMS critical section.  The parameter
 *  _level is not modified.
 */

#define _CPU_ISR_Enable(_level)  \
	do { \
		unsigned int _scratch; \
		loongarch_get_crmd(_scratch); \
		loongarch_set_crmd((_scratch & ~LOONGARCH_IE_BITS) | \
				           (_level & LOONGARCH_IE_BITS)); \
	} while(0)

/*
 *  This temporarily restores the interrupt to _level before immediately
 *  disabling them again.  This is used to divide long RTEMS critical
 *  sections into two or more parts.  The parameter _level is not
 *  modified.
 */

#define _CPU_ISR_Flash(_xlevel) \
	do { \
		unsigned int _scratch2 = _xlevel; \
		_CPU_ISR_Enable(_scratch2); \
		_CPU_ISR_Disable(_scratch2); \
		_xlevel = _scratch2; \
	} while(0)

/*
 *  Map interrupt level in task mode onto the hardware that the CPU
 *  actually provides.  Currently, interrupt levels which do not
 *  map onto the CPU in a generic fashion are undefined.  Someday,
 *  it would be nice if these were "mapped" by the application
 *  via a callout.  For example, m68k has 8 levels 0 - 7, levels
 *  8 - 255 would be available for bsp/application specific meaning.
 *  This could be used to manage a programmable interrupt controller
 *  via the rtems_task_mode directive.
 *
 *  On the LOONGARCH, 0 is all on.  Non-zero is all off.  This only
 *  manipulates the IEC.
 */

uint32_t _CPU_ISR_Get_level(void);

void _CPU_ISR_Set_level(uint32_t);

/* end of ISR handler macros */

/* Context handler macros */

/*
 *  Initialize the context to a state suitable for starting a
 *  task after a context restore operation.  Generally, this
 *  involves:
 *
 *     - setting a starting address
 *     - preparing the stack
 *     - preparing the stack and frame pointers
 *     - setting the proper interrupt level in the context
 *     - initializing the floating point context
 *
 *  This routine generally does not set any unnecessary register
 *  in the context.  The state of the "general data" registers is
 *  undefined at task start time.
 *
 *  NOTE: This is_fp parameter is TRUE if the thread is to be a floating
 *        point thread.  This is typically only used on CPUs where the
 *        FPU may be easily disabled by software such as on the SPARC
 *        where the PSR contains an enable FPU bit.
 *
 *  The per-thread status register holds the interrupt enable, FP enable
 *  and global interrupt enable for that thread.  It means each thread can
 *  enable its own set of interrupts.  If interrupts are disabled, RTEMS
 *  can still dispatch via blocking calls.  This is the function of the
 *  "Interrupt Level", and on the LOONGARCH, it controls the IEC bit and all
 *  the hardware interrupts as defined in the SR.  Software ints
 *  are automatically enabled for all threads, as they will only occur under
 *  program control anyhow.  Besides, the interrupt level parm is only 8 bits,
 *  and controlling the software ints plus the others would require 9.
 *
 *  If the Interrupt Level is 0, all ints are on.  Otherwise, the
 *  Interrupt Level should supply a bit pattern to impose on the SR
 *  interrupt bits; bit 0 applies to the mips1 IEC bit/mips3 EXL&IE, bits 1 thru 6
 *  apply to the SR register Intr bits from bit 10 thru bit 15.  Bit 7 of
 *  the Interrupt Level parameter is unused at this time.
 *
 *  These are the only per-thread SR bits, the others are maintained
 *  globally & explicitly preserved by the Context Switch code in cpu_asm.s
 */

void _CPU_Context_Initialize(
	Context_Control  *the_context,
	uintptr_t        *stack_base,
	uint32_t          size,
	uint32_t          new_level,
	void             *entry_point,
	bool              is_fp,
	void             *tls_area
);

/*
 *  This routine is responsible for somehow restarting the currently
 *  executing task.  If you are lucky, then all that is necessary
 *  is restoring the context.  Otherwise, there will need to be
 *  a special assembly routine which does something special in this
 *  case.  Context_Restore should work most of the time.  It will
 *  not work if restarting self conflicts with the stack frame
 *  assumptions of restoring a context.
 */

#define _CPU_Context_Restart_self(_the_context) \
		_CPU_Context_restore((_the_context));

/*
 *  The purpose of this macro is to allow the initial pointer into
 *  A floating point context area (used to save the floating point
 *  context) to be at an arbitrary place in the floating point
 *  context area.
 *
 *  This is necessary because some FP units are designed to have
 *  their context saved as a stack which grows into lower addresses.
 *  Other FP units can be saved by simply moving registers into offsets
 *  from the base of the context area.  Finally some FP units provide
 *  a "dump context" instruction which could fill in from high to low
 *  or low to high based on the whim of the CPU designers.
 */

#define _CPU_Context_Fp_start(_base, _offset) \
	((void *) _Addresses_Add_offset((_base), (_offset)))

/*
 *  This routine initializes the FP context area passed to it to.
 *  There are a few standard ways in which to initialize the
 *  floating point context.  The code included for this macro assumes
 *  that this is a CPU in which a "initial" FP context was saved into
 *  _CPU_Null_fp_context and it simply copies it to the destination
 *  context passed to it.
 *
 *  Other models include (1) not doing anything, and (2) putting
 *  a "null FP status word" in the correct place in the FP context.
 */

#if (CPU_HARDWARE_FP == TRUE)
#define _CPU_Context_Initialize_fp(_destination) \
	{ \
		*(*(_destination)) = _CPU_Null_fp_context; \
	}
#endif

/* end of Context handler macros */

/* Fatal Error manager macros */

/*
 *  This routine copies _error into a known place -- typically a stack
 *  location or a register, optionally disables interrupts, and
 *  halts/stops the CPU.
 */

#define _CPU_Fatal_halt(_source, _error) \
	do { \
		unsigned int _level; \
		_CPU_ISR_Disable(_level); \
		(void)_level; \
		loop: goto loop; \
	} while (0)

//-----------------------------------------------------------------------------

// extern void loongarch_break(int error);

/* Bitfield handler macros */

/*
 *  This routine sets _output to the bit number of the first bit
 *  set in _value.  _value is of CPU dependent type Priority_bit_map_Word.
 *  This type may be either 16 or 32 bits wide although only the 16
 *  least significant bits will be used.
 *
 *  There are a number of variables in using a "find first bit" type
 *  instruction.
 *
 *    (1) What happens when run on a value of zero?
 *    (2) Bits may be numbered from MSB to LSB or vice-versa.
 *    (3) The numbering may be zero or one based.
 *    (4) The "find first bit" instruction may search from MSB or LSB.
 *
 *  RTEMS guarantees that (1) will never happen so it is not a concern.
 *  (2),(3), (4) are handled by the macros _CPU_Priority_mask() and
 *  _CPU_Priority_bits_index().  These three form a set of routines
 *  which must logically operate together.  Bits in the _value are
 *  set and cleared based on masks built by _CPU_Priority_mask().
 *  The basic major and minor values calculated by _Priority_Major()
 *  and _Priority_Minor() are "massaged" by _CPU_Priority_bits_index()
 *  to properly range between the values returned by the "find first bit"
 *  instruction.  This makes it possible for _Priority_Get_highest() to
 *  calculate the major and directly index into the minor table.
 *  This mapping is necessary to ensure that 0 (a high priority major/minor)
 *  is the first bit found.
 *
 *  This entire "find first bit" and mapping process depends heavily
 *  on the manner in which a priority is broken into a major and minor
 *  components with the major being the 4 MSB of a priority and minor
 *  the 4 LSB.  Thus (0 << 4) + 0 corresponds to priority 0 -- the highest
 *  priority.  And (15 << 4) + 14 corresponds to priority 254 -- the next
 *  to the lowest priority.
 *
 *  If your CPU does not have a "find first bit" instruction, then
 *  there are ways to make do without it.  Here are a handful of ways
 *  to implement this in software:
 *
 *    - a series of 16 bit test instructions
 *    - a "binary search using if's"
 *    - _number = 0
 *      if _value > 0x00ff
 *        _value >>=8
 *        _number = 8;
 *
 *      if _value > 0x0000f
 *        _value >=8
 *        _number += 4
 *
 *      _number += bit_set_table[ _value ]
 *
 *    where bit_set_table[ 16 ] has values which indicate the first
 *      bit set
 */

#define CPU_USE_GENERIC_BITFIELD_CODE 	TRUE
#define CPU_USE_GENERIC_BITFIELD_DATA 	TRUE

#if (CPU_USE_GENERIC_BITFIELD_CODE == FALSE)
#define _CPU_Bitfield_Find_first_bit(_value, _output) \
	{ \
		(_output) = 0;   /* do something to prevent warnings */ \
	}
#endif

/* end of Bitfield handler macros */

/*
 *  This routine builds the mask which corresponds to the bit fields
 *  as searched by _CPU_Bitfield_Find_first_bit().  See the discussion
 *  for that routine.
 */

#if (CPU_USE_GENERIC_BITFIELD_CODE == FALSE)
#define _CPU_Priority_Mask(_bit_number) 		(1 << (_bit_number))
#endif

/*
 *  This routine translates the bit numbers returned by
 *  _CPU_Bitfield_Find_first_bit() into something suitable for use as
 *  a major or minor component of a priority.  See the discussion
 *  for that routine.
 */

#if (CPU_USE_GENERIC_BITFIELD_CODE == FALSE)
#define _CPU_Priority_bits_index(_priority)  	(_priority)
#endif

/* end of Priority handler macros */

/* functions */

/*
 *  _CPU_Initialize
 *
 *  This routine performs CPU dependent initialization.
 */

void _CPU_Initialize(void);

/*
 *  _CPU_ISR_install_raw_handler
 *
 *  This routine installs a "raw" interrupt handler directly into the
 *  processor's vector table.
 */

void _CPU_ISR_install_raw_handler(
	uint32_t    vector,
	proc_ptr    new_handler,
	proc_ptr   *old_handler
);

/*
 *  _CPU_ISR_install_vector
 *
 *  This routine installs an interrupt vector.
 */

void _CPU_ISR_install_vector(
	uint32_t    vector,
	proc_ptr    new_handler,
	proc_ptr   *old_handler
);

/*
 *  _CPU_Install_interrupt_stack
 *
 *  This routine installs the hardware interrupt stack pointer.
 *
 *  NOTE:  It need only be provided if CPU_HAS_HARDWARE_INTERRUPT_STACK
 *         is TRUE.
 */

void _CPU_Install_interrupt_stack(void);

/*
 *  _CPU_Internal_threads_Idle_thread_body
 *
 *  This routine is the CPU dependent IDLE thread body.
 *
 *  NOTE:  It need only be provided if CPU_PROVIDES_IDLE_THREAD_BODY
 *         is TRUE.
 */

void *_CPU_Thread_Idle_body(uintptr_t ignored);

/*
 *  _CPU_Context_switch
 *
 *  This routine switches from the run context to the heir context.
 */

void _CPU_Context_switch(
	Context_Control  *run,
	Context_Control  *heir
);

/*
 *  _CPU_Context_restore
 *
 *  This routine is generally used only to restart self in an
 *  efficient manner.  It may simply be a label in _CPU_Context_switch.
 *
 *  NOTE: May be unnecessary to reload some registers.
 */

void _CPU_Context_restore(
	Context_Control *new_context
) RTEMS_COMPILER_NO_RETURN_ATTRIBUTE;

/*
 *  _CPU_Context_save_fp
 *
 *  This routine saves the floating point context passed to it.
 */

void _CPU_Context_save_fp(Context_Control_fp **fp_context_ptr);

/*
 *  _CPU_Context_restore_fp
 *
 *  This routine restores the floating point context passed to it.
 */

void _CPU_Context_restore_fp(Context_Control_fp **fp_context_ptr);

static inline void _CPU_Context_volatile_clobber(uintptr_t pattern)
{
	/* TODO */
}

static inline void _CPU_Context_validate(uintptr_t pattern)
{
	while (1)
	{
		/* TODO */
	}
}

void _CPU_Exception_frame_print(const CPU_Exception_frame *frame);

/*  The following routine swaps the endian format of an unsigned int.
 *  It must be static because it is referenced indirectly.
 *
 *  This version will work on any processor, but if there is a better
 *  way for your CPU PLEASE use it.  The most common way to do this is to:
 *
 *     swap least significant two bytes with 16-bit rotate
 *     swap upper and lower 16-bits
 *     swap most significant two bytes with 16-bit rotate
 *
 *  Some CPUs have special instructions which swap a 32-bit quantity in
 *  a single instruction (e.g. i486).  It is probably best to avoid
 *  an "endian swapping control bit" in the CPU.  One good reason is
 *  that interrupts would probably have to be disabled to ensure that
 *  an interrupt does not try to access the same "chunk" with the wrong
 *  endian.  Another good reason is that on some CPUs, the endian bit
 *  endianness for ALL fetches -- both code and data -- so the code
 *  will be fetched incorrectly.
 */

static inline uint32_t CPU_swap_u32(uint32_t value)
{
	uint32_t byte1, byte2, byte3, byte4, swapped;

	byte4 = (value >> 24) & 0xff;
	byte3 = (value >> 16) & 0xff;
	byte2 = (value >> 8)  & 0xff;
	byte1 =  value        & 0xff;

	swapped = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
	return(swapped);
}

#define CPU_swap_u16(value) 	(((value&0xff) << 8) | ((value >> 8)&0xff))

typedef uint32_t CPU_Counter_ticks;

CPU_Counter_ticks _CPU_Counter_read(void);

static inline CPU_Counter_ticks _CPU_Counter_difference(
	CPU_Counter_ticks second,
	CPU_Counter_ticks first)
{
	return second - first;
}

#endif // #ifndef ASM

#ifdef __cplusplus
}
#endif

#endif

/*
 * @@ END
 */
