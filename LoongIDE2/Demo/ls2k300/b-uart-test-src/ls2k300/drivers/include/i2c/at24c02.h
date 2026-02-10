/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * at24c02.h
 *
 * created: 2024-7-13
 *  author: 
 */

#ifndef _AT24C02_H
#define _AT24C02_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Device name
//-----------------------------------------------------------------------------

#define AT24C02_DEV_NAME        "i2c0.eeprom"

//-----------------------------------------------------------------------------
// AT24016 driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *at24c02_drv_ops;

#define ls2k_at24c02_read(bus, buf, size, arg)  at24c02_drv_ops->read_entry(bus, buf, size, arg)
#define ls2k_at24c02_write(bus, buf, size, arg) at24c02_drv_ops->write_entry(bus, buf, size, arg)

#else

/**
 * bus:  busI2C0
 * buf:  unsigned char *
 * size: length of buf
 * arg:  integer, address of eeprom
 */
int AT24C02_read(const void *bus, void *buf, int size, void *arg);

/**
 * bus:  busI2C0
 * buf:  unsigned char *
 * size: length of buf
 * arg:  integer, address of eeprom
 */
int AT24C02_write(const void *bus, void *buf, int size, void *arg);

#define ls2k_at24c02_read(bus, buf, size, arg)  AT24C02_read(bus, buf, size, arg)
#define ls2k_at24c02_write(bus, buf, size, arg) AT24C02_write(bus, buf, size, arg)

#endif


#ifdef __cplusplus
}
#endif

#endif // _AT24C02_H

