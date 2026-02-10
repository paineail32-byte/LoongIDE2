/*
 * FreeRTOS Kernel V10.2.1
 */

/* Critical section management, and task utilities.*/
#ifndef _CRIT_TASK_H
#define	_CRIT_TASK_H

#include <larchintrin.h>
#include "cpu.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#define portDISABLE_INTERRUPTS( )   loongarch_interrupt_disable();
#define portENABLE_INTERRUPTS( ) 	loongarch_interrupt_enable();

#if (portTICK_TYPE_IS_ATOMIC == 0)
extern void vTaskEnterCritical( void );
extern void vTaskExitCritical( void );

#define portCRITICAL_NESTING_IN_TCB 1

#if portCRITICAL_NESTING_IN_TCB

#define portENTER_CRITICAL()        do { \
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) \
        vTaskEnterCritical(); \
    } while (0)
#define portEXIT_CRITICAL()         do { \
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) \
        vTaskExitCritical(); \
    } while (0)

#else

#define portENTER_CRITICAL()        do { } while (0)
#define portEXIT_CRITICAL()         do { } while (0)

#endif // #if portCRITICAL_NESTING_IN_TCB
    
#endif // #if (portTICK_TYPE_IS_ATOMIC == 0)

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
	#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* Check the configuration. */
	#if( configMAX_PRIORITIES > 32 )
		#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1  \
               when configMAX_PRIORITIES is less than or equal to 32.  \
               It is very rare that a system requires more than 10 to 15 \
               difference priorities as tasks that share a priority will time slice.
	#endif

	/* Store/clear the ready priorities in a bit map. */
	#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )      \
		( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )

	#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )       \
		( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

	/*-----------------------------------------------------------*/

	#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    \
		uxTopPriority = ( 31 - loongarch_clz( ( uxReadyPriorities ) ) )

#endif

/* Task utilities. */

extern void vPortYield(void);
extern void vPortYieldFromISR(BaseType_t xHigherPriorityTaskWoken);

#define portYIELD()             vPortYield()

#define portYIELD_FROM_ISR(x)   vPortYieldFromISR(x)

#define portASSERT_IF_IN_ISR()

/* Tick Hook */
extern void vApplicationTickHook(void);

/*-----------------------------------------------------------*/

#ifdef	__cplusplus
}
#endif

#endif	/* _CRIT_SECT_H */

