/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_SPI_MODE_H
#define _LS2K_SPI_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// SPI bus master communication mode
//-----------------------------------------------------------------------------

typedef struct
{
    unsigned int  baudrate;         /* maximum bits per second */
    unsigned char bits_per_char;    /* how many bits per byte/word/longword? */
    bool          lsb_first;        /* true: send LSB first */
    bool          clock_pha;        /* clock phase    - spi mode */
    bool          clock_pol;        /* clock polarity - spi mode */
    bool          clock_inv;        /* low active - true: inverted clock (high active) */
    bool          clock_phs;        /* true: clock starts toggling at start of data tfr - interface mode */
} SPI_mode_t;

#ifdef __cplusplus
}
#endif

#endif // _LS2K_SPI_MODE_H

