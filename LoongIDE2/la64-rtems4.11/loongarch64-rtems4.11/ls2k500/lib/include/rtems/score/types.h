/**
 * @file rtems/score/types.h
 *
 * @brief Type Definitions Pertaining to the LOONGARCH Processor Family
 *
 *  This include file contains type definitions pertaining to the LOONGARCH
 *  processor family.
 */

/*
 *  COPYRIGHT (c) 1989-2001.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_TYPES_H
#define _RTEMS_SCORE_TYPES_H

/**
 *  @defgroup ScoreTypes LOONGARCH Processor Family Type Definitions
 *
 *  @ingroup Score
 *
 */
/**@{*/

#include <rtems/score/basedefs.h>

#ifndef ASM

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  This section defines the basic types for this processor.
 */

/** Type that can store a 32-bit integer or a pointer. */
typedef uintptr_t CPU_Uint32ptr;

typedef uint16_t Priority_bit_map_Word;
typedef void     loongarch_isr;
typedef void   (*loongarch_isr_entry)(void);

#ifdef __cplusplus
}
#endif

#endif  /* !ASM */

/**@}*/
#endif
