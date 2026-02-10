/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * FreeRTOS Kernel V10.2.1
 */

/*-----------------------------------------------------------------------------
 * Implementation of functions defined in portable.h for the MIPS32 port.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <larchintrin.h>

#include "regdef.h"
#include "cpu.h"

#include "FreeRTOS.h"
#include "task.h"

#include "portable.h"

#include "irq.h"
#include "context.h"

/* Let the user override the pre-loading of the initial RA with the address of
   prvTaskExitError() in case is messes up unwinding of the stack in the
   debugger - in which case configTASK_RETURN_ADDRESS can be defined as 0 (NULL). */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
   stack checking.  A problem in the ISR stack will trigger an assert, not call the
   stack overflow hook function (because the stack overflow hook is specific to a
   task stack, not the ISR stack). */
#if( configCHECK_FOR_STACK_OVERFLOW > 2 )

	/* Don't use 0xa5 as the stack fill bytes as that is used by the kernel for
	   the task stacks, and so will legitimately appear in many positions within
	   the ISR stack. */
	#define portISR_STACK_FILL_BYTE	0xee

	static const uint8_t ucExpectedStackBytes[] = {
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,   \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,   \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,	  \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,	  \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE }; \

	#define portCHECK_ISR_STACK() configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else
	/* Define the function away. */
	#define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

volatile UBaseType_t uxYieldFromISRFlag = pdFALSE;
volatile UBaseType_t uxInsideInterrupt = pdFALSE;

/*-----------------------------------------------------------*/

/*
 * use heap_5.c
 */

static HeapRegion_t xHeapRegions[2];

void xFreeRTOSInitialiseHeap_5(void *addr, size_t size)
{
    xHeapRegions[0].pucStartAddress = (uint8_t *)addr;
    xHeapRegions[0].xSizeInBytes = size;

    xHeapRegions[1].pucStartAddress = NULL;
    xHeapRegions[1].xSizeInBytes = 0;

    vPortDefineHeapRegions( xHeapRegions );
}

/*-----------------------------------------------------------*/

register StackType_t $GP __asm__ ("$r2");

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    static StackType_t crmd=0, ecfg=0, _gp=0;
    StackType_t *stk;

    if (crmd == 0)
    {
        crmd  = __csrrd_d(LA_CSR_CRMD);
        crmd |= CSR_CRMD_WE | CSR_CRMD_IE;
        ecfg  = __csrrd_d(LA_CSR_ECFG);
        _gp   = $GP;
    }

#if (portSTACK_GROWTH < 0)
    stk = (StackType_t *)((unsigned long)pxTopOfStack & ~0x7);
#else
#error "not support "
#endif

	*stk = (StackType_t) 0xDEADBEEF;
	stk--;

	*stk = (StackType_t) 0x12345678;        /* Word to which the stack pointer will be left pointing after context restore. */
	stk--;

#if (portSTACK_GROWTH < 0)
    stk -= CTX_SIZE / 8;                    /* sizeof(unsigned long) */
#else
#error "TODO stack need grown down."
#endif

    memset((void *)stk, 0, CTX_SIZE);       /* Clear the stack */

/*
    for (int i=0; i < 32; i++)
    {
        stk[i] = ((StackType_t)(i & 0xFF) << 56) |
                 ((StackType_t)(i & 0xFF) << 48) |
                 ((StackType_t)(i & 0xFF) << 40) |
                 ((StackType_t)(i & 0xFF) << 32) |
                 ((StackType_t)(i & 0xFF) << 24) |
                 ((StackType_t)(i & 0xFF) << 16) |
                 ((StackType_t)(i & 0xFF) << 8 ) |
                 ((StackType_t)(i & 0xFF) << 0 );
    }
 */
 
    *(stk + R_SP   ) = (StackType_t)stk;
    *(stk + R_A0   ) = (StackType_t)pvParameters;   /* Parameters to pass in. */
    *(stk + R_RA   ) = (StackType_t)portTASK_RETURN_ADDRESS;

    *(stk + R_ESTAT) = 0;
    *(stk + R_CRMD)  = crmd;
    *(stk + R_GP  )  = _gp;
    *(stk + R_ECFG)  = ecfg;

    *(stk + R_PRMD)  = CSR_PRMD_PWE | CSR_PRMD_PIE;
    *(stk + R_EPC )  = (StackType_t)pxCode;

	return stk;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).

	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
//	configASSERT( uxSavedTaskStackPointer == 0UL );

	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vPortEndScheduler(void)
{
	/* Not implemented in ports where there is nothing to return to.
	   Artificially force an assert. */
//	configASSERT( uxInterruptNesting == 1000UL );

    return;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    extern void vPortStartFirstTask( void );

	#if ( configCHECK_FOR_STACK_OVERFLOW > 2 )
	{
		/* Fill the ISR stack to make it easy to asses how much is being used. */
		memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
	}
	#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

	vPortStartFirstTask();

	/* Should never get here as the tasks will now be executing!  Call the task
 	   exit error function to prevent compiler warnings about a static function
	   not being called in the case that the application writer overrides this
	   functionality by defining configTASK_RETURN_ADDRESS. */
	prvTaskExitError();

	return pdFALSE;
}
/*-----------------------------------------------------------*/

//-----------------------------------------------------------------------------

void vPortIncrementTick(int vector, void *arg)
{
    UBaseType_t uxSavedStatus;

    vApplicationTickHook();

    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
    {
        return;
    }

	uxSavedStatus = vPortSetInterruptMaskFromISR();
	{
		if( xTaskIncrementTick() != pdFALSE )
		{
            portYIELD_FROM_ISR(pdTRUE);
		}
	}
	vPortClearInterruptMaskFromISR( uxSavedStatus );

	/* Look for the ISR stack getting near or past its limit. */
	portCHECK_ISR_STACK();

	/* Clear timer interrupt. */
//	configCLEAR_TICK_TIMER_INTERRUPT();
}

/*-----------------------------------------------------------*/

#if configCHECK_FOR_STACK_OVERFLOW > 0
/* Assert at a stack overflow. */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	configASSERT( xTask == NULL );
}
#endif

extern void printk(const char *fmt, ...);

void vAssertCalled( const char *pcFileName, unsigned long ulLine )
{
    printk("assert: filename %s; line %i\r\n", pcFileName, (int)ulLine);
}

//-------------------------------------------------------------------------------------------------
// 可以直接 return ?
//-------------------------------------------------------------------------------------------------

UBaseType_t vPortSetInterruptMaskFromISR( void )
{
    UBaseType_t crmd = __csrrd_d(LA_CSR_CRMD);
    __csrxchg_d(~CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD);
    return (UBaseType_t)crmd;
}

void vPortClearInterruptMaskFromISR( UBaseType_t level)
{
    __csrwr_d((unsigned long)level, LA_CSR_CRMD);
}

//-------------------------------------------------------------------------------------------------

BaseType_t xPortIsInsideInterrupt( void )
{
    return uxInsideInterrupt ? pdTRUE : pdFALSE;
}
    
//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */
 
