/*
 * ls2k_spi_bus.h
 *
 * this file contains the ls2k SPI driver declarations
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#ifndef _LS2K_SPI_H
#define _LS2K_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// SPI Flash fast access memory base
//-----------------------------------------------------------------------------

/*
 * SPI flash读使能, 无效时csn[0]可由软件控制
 *
 * spi0 flash 芯片驱动程序会用到
 *
 */
#ifndef SPI_PARAM_MEMORY_EN
#define SPI_PARAM_MEMORY_EN		0x01
#endif

#define FLASH_MEMBASE			0x800000001C000000ULL

/*
 * IOCTL of spi_flash read engine mode
 */
#define IOCTL_FLASH_FAST_READ_ENABLE	0x1000		// none. - set spi flash param to memory-en
#define IOCTL_FLASH_FAST_READ_DISABLE	0x2000		// none. - unset spi flash param to memory-en
#define IOCTL_FLASH_GET_FAST_READ_MODE	0x4000		// unsigned int * - get spi flash is set to memory-en

//-----------------------------------------------------------------------------

#define LS2K_DEVNM_SPI0		"/dev/spi0"
#ifdef LS2K500
#define LS2K_DEVNM_SPI1		"/dev/spi1"
#define LS2K_DEVNM_SPI2		"/dev/spi2"
#define LS2K_DEVNM_SPI3		"/dev/spi3"
#define LS2K_DEVNM_SPI4		"/dev/spi4"
#define LS2K_DEVNM_SPI5		"/dev/spi5"
#endif

//-----------------------------------------------------------------------------

/*
 * for install sub-device.
 * parameter: busname is like above.
 */
int bsp_find_spi_busno(char *busname);

/*
 * install all spi driver
 */
rtems_status_code bsp_install_all_spi_drivers(void);

/*
 * install one spi driver
 */
rtems_status_code bsp_install_spi_driver(char *busname);


#ifdef __cplusplus
}
#endif

#endif /* _LS2K_SPI_H */
