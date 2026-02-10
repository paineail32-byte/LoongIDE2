/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_can.h
 *
 * created: 2022/6/7
 *  author: Bian
 */

#ifndef _DRV_CAN_H
#define _DRV_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPECIAL for LS2k CAN:
 *
 *   rt_device_read() / rt_device_write() parameter "buffer" is as "CANMsg_t *"
 *
 */

void rt_ls2k_can_install(void);

extern const char *ls2k_can_get_device_name(const void *pCAN);

#ifdef __cplusplus
}
#endif

#endif // _DRV_CAN_H

