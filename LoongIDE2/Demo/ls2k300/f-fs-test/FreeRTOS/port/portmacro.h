/*
 * FreeRTOS Kernel V10.2.1
 */

#ifndef _PORTMACRO_H
#define	_PORTMACRO_H

/* Include system headers */
//#include "cpu.h"

#ifdef	__cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR                char
#define portFLOAT               float
#define portDOUBLE              double
#define portLONG                long
#define portSHORT               short
#define portSTACK_TYPE          unsigned long
#define portBASE_TYPE           long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

/* We'll use 32-bit ticks for efficiency */
typedef uint32_t TickType_t;
#define portMAX_DELAY           (TickType_t) 0xffffffffUL
#define portTICK_PERIOD_MS     ((TickType_t) 1000 / configTICK_RATE_HZ)

/*-----------------------------------------------------------*/

#define portSTACK_GROWTH	    -1

/*
 * Hardware specifics.
 */
#define portBYTE_ALIGNMENT		8

#define portPOINTER_SIZE_TYPE   UBaseType_t     /* 64bit CPU */

/*-----------------------------------------------------------*/

/* Critical section management is done in the crit_sect.h file for the
   interrupt mechanism being used; similarly, interrupt enable and disable are
   also defined in that file. The directory for the interrupt mechanism MUST be
   on the include path, things will not compile. */
#include "crit_task.h"
//#include "irq.h"

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )  \
			void vFunction( void *pvParameters ) __attribute__((noreturn))
#define portTASK_FUNCTION( vFunction, pvParameters )        \
			void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Required by the kernel aware debugger. */
#ifdef __DEBUG
	#define portREMOVE_STATIC_QUALIFIER
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    do { } while(0)     /* we use the timer */
#define portALT_GET_RUN_TIME_COUNTER_VALUE(dest)    (dest = xTickCount)

#ifdef	__cplusplus
}
#endif

#endif	/* _PORTMACRO_H */

