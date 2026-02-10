/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include "arch/cc.h"

//*****************************************************************************
// Use OSAL instead
//*****************************************************************************

#include "osal.h"

#define SYS_MBOX_NULL   	(osal_mq_t)NULL
#define SYS_SEM_NULL    	(osal_sem_t)NULL

#define SYS_MBOX_SIZE       sizeof(size_t)
#define SYS_LWIP_TIMER_NAME "timer"
#define SYS_LWIP_MBOX_NAME  "mbox"
#define SYS_LWIP_SEM_NAME   "sem"
#define SYS_LWIP_MUTEX_NAME "mutex"

typedef osal_sem_t      	sys_sem_t;
typedef osal_mutex_t    	sys_mutex_t;
typedef osal_mq_t       	sys_mbox_t;
typedef osal_task_t     	sys_thread_t;

#endif /* __ARCH_SYS_ARCH_H__ */


