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


extern UBaseType_t vPortSetInterruptMaskFromISR( void );
extern void vPortClearInterruptMaskFromISR( UBaseType_t );
extern void vApplicationSetupSoftwareInterrupt( void );



#if 0

extern void vPortYieldISR( int, void * );

//extern void vLevelTrigExternalNonEicInterrupt( uint32_t ext_int, uint32_t pol);
//extern void vRouteExternalNonEicInterrupt( uint32_t ext_int, uint32_t vpe, uint32_t vpe_int);

//#define portINITIAL_SR                     (SR_IMASK | SR_IE)


#define portCLEAR_PENDING_INTERRUPTS() \
    do { \
        unsigned int _cause;       \
        mips_get_cause(_cause);    \
        _cause &= ~(CAUSE_IPMASK); \
        mips_set_cause(_cause);    \
    } while (0)

/* Generate a software interrupt */
int assert_sw_irq(unsigned int irqnum);
/* Clear a software interrupt */
int negate_sw_irq(unsigned int irqnum);
#endif

#define portSET_INTERRUPT_MASK_FROM_ISR()   vPortSetInterruptMaskFromISR()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusRegister ) \
	vPortClearInterruptMaskFromISR( uxSavedStatusRegister )

#ifdef	__cplusplus
}
#endif

#endif	/* INT_HANDLER_H */

