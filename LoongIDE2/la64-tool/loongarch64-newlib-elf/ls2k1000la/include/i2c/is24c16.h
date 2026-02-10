/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * is24c16.h
 *
 * created: 2022-10-12
 *  author: Bian
 */

#ifndef _IS24C16_H
#define _IS24C16_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// I2C0-IS24C16 driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *is24c16_drv_ops;

#define ls2k_is24c16_read(iic, buf, size, arg)  is24c16_drv_ops->read_entry(iic, buf, size, arg)
#define ls2k_is24c16_write(iic, buf, size, arg) is24c16_drv_ops->write_entry(iic, buf, size, arg)

#else

/*
 * IS24C16_read()
 *
 *   parameter:
 *     bus		busI2C0
 *     buf      is as unsigned char *
 *     arg      unsigned int, as EEPROM address to read
 *
 *  return 		readed bytes
 */
int IS24C16_read(const void *bus, void *buf, int size, void *arg);

/*
 * IS24C16_write()
 *
 *   parameter:
 *     bus		busI2C0
 *     buf      is as unsigned char *
 *     arg      unsigned int, as EEPROM address to write
 *
 *  return 		written bytes
 */
int IS24C16_write(const void *bus, void *buf, int size, void *arg);

#define ls2k_is24c16_read(iic, buf, size, arg)  IS24C16_read(iic, buf, size, arg)
#define ls2k_is24c16_write(iic, buf, size, arg) IS24C16_write(iic, buf, size, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _IS24C16_H

