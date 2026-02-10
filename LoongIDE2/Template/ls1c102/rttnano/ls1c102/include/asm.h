/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd.
 */
/*
 * asm.h
 *
 * created: 2021-12-24
 *  author: Bian
 */

#ifndef _ASM_H
#define _ASM_H

/*
 * Indicate we are in an assembly file and get the basic CPU definitions.
 */

/*
 * Recent versions of GNU cpp define variables which indicate the
 * need for underscores and percents.  If not using GNU cpp or
 * the version does not support this, then you will obviously
 * have to define these as appropriate.
 */
#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif

#ifndef __REGISTER_PREFIX__
#define __REGISTER_PREFIX__
#endif

// #include <concat.h>

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1(__USER_LABEL_PREFIX__, x)

/* Use the right prefix for registers.  */

#define REG(x) CONCAT1(__REGISTER_PREFIX__, x)

/*
 * define macros for all of the registers on this CPU
 *
 * EXAMPLE:     #define d0 REG (d0)
 */

/*
 * Define macros to handle section beginning and ends.
 */
#define BEGIN_CODE_DCL .text
#define END_CODE_DCL
#define BEGIN_DATA_DCL .data
#define END_DATA_DCL
#define BEGIN_CODE     .text
#define END_CODE
#define BEGIN_DATA
#define END_DATA
#define BEGIN_BSS
#define END_BSS
//#define END

/*
 * Following must be tailor for a particular flavor of the C compiler.
 * They may need to put underscores in front of the symbols.
 */
#define PUBLIC(sym) .globl SYM(sym)
#define EXTERN(sym) .globl SYM(sym)

//-------------------------------------------------------------------

#define LEAF(label) \
    .align 3;       \
    .globl label;   \
label:

#if 0
#define END(name) \
        .end  name
#else
#define END(name)
#endif

#endif // _ASM_H

