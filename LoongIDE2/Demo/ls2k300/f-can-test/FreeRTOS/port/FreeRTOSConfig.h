/*
 * FreeRTOS Kernel V10.2.1
 */

#ifndef _FREERTOS_CONFIG_H
#define _FREERTOS_CONFIG_H

/*-----------------------------------------------------------*/

#define CONFIG_TIMER_CLOCK_HZ   1000
#define CONFIG_TICK_RATE_HZ     1000

#define CONFIG_MAX_PRIORITIES   31
#define CONFIG_STACK_SIZE       (1*1024)

#define CONFIG_HEAP_MODULE      5                   /* using heap_5.c */

#define CONFIG_HEAP_SIZE        (64*1024*1024)      /* heap size: 64M */

/*-----------------------------------------------------------*/

#define configUSE_PREEMPTION                        1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION     0
#define configTIMER_RATE_HZ                         ( ( TickType_t ) CONFIG_TIMER_CLOCK_HZ )
#define configTICK_RATE_HZ                          ( CONFIG_TICK_RATE_HZ )
#define configUSE_16_BIT_TICKS                      0
#define configMAX_PRIORITIES                        ( CONFIG_MAX_PRIORITIES )
#define configMINIMAL_STACK_SIZE                    ( CONFIG_STACK_SIZE )
#define configISR_STACK_SIZE                        ( CONFIG_STACK_SIZE )
#define configTOTAL_HEAP_SIZE						( ( size_t ) CONFIG_HEAP_SIZE )
#define configMAX_TASK_NAME_LEN                     ( 16 )
#define configCHECK_FOR_STACK_OVERFLOW              0           // remove when release

#define configUSE_MUTEXES                           1           // mutex: 设备保护与线程安全
#define configUSE_TIMERS                            1           // event: 中断与任务同步
#if configUSE_TIMERS
#define configTIMER_TASK_PRIORITY                   16          // 配置软件定时器任务的优先级: big is high
#define configTIMER_QUEUE_LENGTH                    8           // 配置软件定时器命令队列的长度
#define configTIMER_TASK_STACK_DEPTH                (1*1024)    // 配置软件定时器任务的栈空间大小
#endif

/* Hook functions */
#define configUSE_IDLE_HOOK                         0
#define configUSE_TICK_HOOK                         1           //

/* Co routines */
#define configUSE_CO_ROUTINES                       0

/* The interrupt priority of the RTOS kernel */
#define configKERNEL_INTERRUPT_PRIORITY             0x01

/* The maximum priority from which API functions can be called */
#define configMAX_API_CALL_INTERRUPT_PRIORITY       0x03

/*
 * TCB_t 内部使用, XXX don't update It.
 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS     8

//-----------------------------------------------------------------------------

#define configUSE_COUNTING_SEMAPHORES               1       /* cherry usb */

#define configUSE_TRACE_FACILITY                    1

/*
 * 查看运行时刻统计数
 */
#define configGENERATE_RUN_TIME_STATS               1
#if configGENERATE_RUN_TIME_STATS
#define configUSE_STATS_FORMATTING_FUNCTIONS        1
#define configSUPPORT_DYNAMIC_ALLOCATION            1
#endif

//-----------------------------------------------------------------------------

/* Prevent assert code from being used in assembly files */
#ifndef __ASSEMBLER__
	void vAssertCalled( const char *pcFileName, unsigned long ulLine );
	#define configASSERT( x )						\
		do {										\
			if( ( x ) == 0 )						\
				vAssertCalled( __FILE__, __LINE__ );\
		} while (0)
#endif

/*-----------------------------------------------------------*/

/* Optional functions */
#define INCLUDE_vTaskPrioritySet					1
#define INCLUDE_uxTaskPriorityGet					1
#define INCLUDE_vTaskDelayUntil						1
#define INCLUDE_vTaskDelay							1
#define INCLUDE_vTaskDelete							1
#define INCLUDE_vTaskSuspend						1
#define INCLUDE_xTimerPendFunctionCall              1       // event group

#define INCLUDE_xTaskGetHandle                      1       // get pcb handle by name

/*-----------------------------------------------------------*/

#if defined(ENABLE_TRACE)
#include "trace.h"
#endif

#define traceTASK_INCREMENT_TICK(xTickCount)    xTickCount++

/*-----------------------------------------------------------*/

#endif	/* FREERTOSCONFIG_H */

