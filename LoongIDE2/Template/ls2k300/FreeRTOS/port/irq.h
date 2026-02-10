/*
 * FreeRTOS Kernel V10.2.1
 */

#ifndef _INT_HANDLER_H
#define	_INT_HANDLER_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include "portmacro.h"

/* Initialise the ISR table */
void vPortInitISR( void );

/* Interrupt manipulation */
extern void pvPortInstallISR( uint32_t, void ( * )( int, void * ) );

/*
 * The software interrupt handler that performs the yield.
 */
/*
extern UBaseType_t vPortSetInterruptMaskFromISR( void );
extern void vPortClearInterruptMaskFromISR( UBaseType_t );
extern void vApplicationSetupSoftwareInterrupt( void );


#define portSET_INTERRUPT_MASK_FROM_ISR()   vPortSetInterruptMaskFromISR()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusRegister ) \
	vPortClearInterruptMaskFromISR( uxSavedStatusRegister )
 */

#ifdef	__cplusplus
}
#endif

#endif	/* INT_HANDLER_H */

