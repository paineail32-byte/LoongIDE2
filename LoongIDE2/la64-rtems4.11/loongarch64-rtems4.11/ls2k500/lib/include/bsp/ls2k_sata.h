/*
 * ls2k_sata.h
 *
 * this file contains the ls2k SATA driver declarations
 *
 *  Created on: 2023-11-1
 *      Author: Bian
 */

#ifndef _LS2K_SATA_H
#define _LS2K_SATA_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 如果磁盘接在 sata0 上, 设备名称: /hda1 或者 /hda2 ...
 *
 * 如果磁盘接在 sata1 上, 设备名称: /hdb1 或者 /hdb2 ...
 *
 */

/*
 * install sata driver, and mount disks
 */
rtems_status_code bsp_install_sata_driver(void);


#ifdef __cplusplus
}
#endif

#endif /* _LS2K_SATA_H */
