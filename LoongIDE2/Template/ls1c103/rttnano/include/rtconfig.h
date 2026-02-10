/* RT-Thread config file */

#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

#define RT_THREAD_PRIORITY_MAX  8

#define RT_TICK_PER_SECOND  1000

#define RT_ALIGN_SIZE   4

#define RT_NAME_MAX    8

#define RT_USING_COMPONENTS_INIT

#define IDLE_THREAD_STACK_SIZE        512

#define RT_USING_USER_MAIN

#define RT_MAIN_THREAD_STACK_SIZE     512

#define RT_DEBUG_INIT 0

#define RT_USING_OVERFLOW_CHECK

//#define RT_USING_HOOK

//#define RT_USING_IDLE_HOOK

#define RT_USING_TIMER_SOFT         0

#if RT_USING_TIMER_SOFT == 0
#undef RT_USING_TIMER_SOFT
#endif

#define RT_TIMER_THREAD_PRIO        3

#define RT_TIMER_THREAD_STACK_SIZE  512

#define RT_USING_SEMAPHORE

//#define RT_USING_MUTEX

#define RT_USING_MAILBOX

//#define RT_USING_MESSAGEQUEUE

//#define RT_USING_HEAP

#define RT_USING_SMALL_MEM

#define RT_USING_TINY_SIZE

#define RT_USING_CONSOLE            /* rt_kprintf() */

#define RT_CONSOLEBUF_SIZE          64

#if defined(RTE_USING_FINSH)
#define RT_USING_FINSH

#define FINSH_USING_MSH

#define FINSH_USING_MSH_ONLY

#define __FINSH_THREAD_PRIORITY     5

#define FINSH_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX / 8 * __FINSH_THREAD_PRIORITY + 1)

#define FINSH_THREAD_STACK_SIZE     512

#define FINSH_HISTORY_LINES         1

#define FINSH_USING_SYMTAB
#endif

#endif
