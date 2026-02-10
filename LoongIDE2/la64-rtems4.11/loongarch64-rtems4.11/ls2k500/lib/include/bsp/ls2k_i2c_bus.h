/*
 * ls2k_i2c_bus.h
 *
 * this file contains the ls2k I2C driver declarations
 *
 *  Created on: 2023-11-1
 *      Author: Bian
 */

#ifndef _LS2K_I2C_H
#define _LS2K_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// I2C Device Names
//-----------------------------------------------------------------------------
 
#define LS2K_DEVNM_I2C0			"/dev/i2c0"
#define LS2K_DEVNM_I2C1			"/dev/i2c1"
#ifdef LS2K500
#define LS2K_DEVNM_I2C2			"/dev/i2c2"
#define LS2K_DEVNM_I2C3			"/dev/i2c3"
#define LS2K_DEVNM_I2C4			"/dev/i2c4"
#define LS2K_DEVNM_I2C5			"/dev/i2c5"
#endif

//-----------------------------------------------------------------------------

/*
 * for install sub-device.
 * parameter: busname is like above.
 */
int bsp_find_i2c_busno(char *busname);

/*
 * install all i2c drivers
 */
rtems_status_code bsp_install_all_i2c_drivers(void);

/*
 * install one i2c driver
 */
rtems_status_code bsp_install_i2c_driver(char *busname);


#ifdef __cplusplus
}
#endif

#endif /* _LS2K_I2C_H */

