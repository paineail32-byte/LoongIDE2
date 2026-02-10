/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

#include "arch/sys_arch.h"

//*****************************************************************************
// Use OSAL instead
//*****************************************************************************

#include "osal.h"

#define LWIP_NAME_MAX   16

//-----------------------------------------------------------------------------
// message queue
//-----------------------------------------------------------------------------

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    static unsigned short counter = 0;
    char tname[LWIP_NAME_MAX];
    sys_mbox_t tmpmbox;

    snprintf(tname, LWIP_NAME_MAX, "%s%d", SYS_LWIP_MBOX_NAME, counter);
    counter++;

    tmpmbox = osal_mq_create(tname, OSAL_OPT_FIFO, size, 8);
    if (tmpmbox == NULL)
        return ERR_MEM;

    *mbox = tmpmbox;
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    osal_mq_delete(*mbox);
}

/*
 * What sending is a pointer
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
#ifdef OS_UCOS
    osal_mq_send(*mbox, (const void *)msg, sizeof(uintptr_t));
#else
    osal_mq_send(*mbox, (const void *)&msg, sizeof(uintptr_t));
#endif
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
#ifdef OS_UCOS
    if (osal_mq_send(*mbox, (const void *)msg, sizeof(uintptr_t)) == 0)
#else
    if (osal_mq_send(*mbox, (const void *)&msg, sizeof(uintptr_t)) == 0)
#endif
        return ERR_OK;

    return ERR_MEM;
}

/*
 * What receiving is a pointer
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    u32_t ticks = get_clock_ticks();
    int tmo = (timeout == 0) ? OSAL_WAIT_FOREVER : timeout;

    if (osal_mq_receive(*mbox, (void *)msg, sizeof(uintptr_t), tmo) < 0)
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    /* get elapse msecond */
    ticks = get_clock_ticks() - ticks;
    if (ticks == 0)
        ticks = 1;

    return ticks;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
#ifdef OS_UCOS
    if (osal_mq_receive(*mbox, (void *)msg, sizeof(uintptr_t), 1) < 0)
#else
    if (osal_mq_receive(*mbox, (void *)msg, sizeof(uintptr_t), 0) < 0)
#endif
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    return 1;
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (int)(size_t)(*mbox);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = NULL;
}

//-----------------------------------------------------------------------------
// Semphore
//-----------------------------------------------------------------------------

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    static unsigned short counter = 0;
    char tname[LWIP_NAME_MAX];
    sys_sem_t tmpsem;

    snprintf(tname, LWIP_NAME_MAX, "%s%d", SYS_LWIP_SEM_NAME, counter);
    counter++;

    tmpsem = osal_sem_create(tname, OSAL_OPT_FIFO, count);
    if (tmpsem == NULL)
        return ERR_MEM;

    *sem = tmpsem;
    return ERR_OK;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    u32_t ticks = get_clock_ticks();
    int tmo = (timeout == 0) ? OSAL_WAIT_FOREVER : timeout;

    if (osal_sem_obtain(*sem, tmo) != 0)
    {
        return SYS_ARCH_TIMEOUT;
    }

    /* get elapse msecond */
    ticks = get_clock_ticks() - ticks;
    if (ticks == 0)
        ticks = 1;

    return ticks;
}

void sys_sem_signal(sys_sem_t *sem)
{
    osal_sem_release(*sem);
}

void sys_sem_free(sys_sem_t *sem)
{
    osal_sem_delete(*sem);
}

int sys_sem_valid(sys_sem_t *sem)
{
    return (int)(size_t)(*sem);
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = NULL;
}

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    static unsigned short counter = 0;
    char tname[LWIP_NAME_MAX];
    sys_mutex_t tmpmutex;

    snprintf(tname, LWIP_NAME_MAX, "%s%d", SYS_LWIP_MUTEX_NAME, counter);
    counter++;

    tmpmutex = osal_mutex_create(tname, OSAL_OPT_FIFO);
    if (tmpmutex == NULL)
        return ERR_MEM;

    *mutex = tmpmutex;
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    osal_mutex_obtain(*mutex, OSAL_WAIT_FOREVER);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    osal_mutex_release(*mutex);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    osal_mutex_delete(*mutex);
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
    return (int)(size_t)(*mutex);
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = NULL;
}

//-----------------------------------------------------------------------------
// Lock/Unlock
//-----------------------------------------------------------------------------

sys_prot_t sys_arch_protect(void)
{
    return (sys_prot_t)osal_enter_critical_section();
}

void sys_arch_unprotect(sys_prot_t pval)
{
    osal_leave_critical_section((size_t)pval);
}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------

void sys_init(void)
{
    /* do nothing */
}

//-----------------------------------------------------------------------------
// Thread
//-----------------------------------------------------------------------------

sys_thread_t sys_thread_new(const char *name,
                            lwip_thread_fn function,
                            void *arg,
                            int stacksize,
                            int prio)
{
    return osal_task_create(name,
                            stacksize,
                            prio,
                            10,     // slice
                            (osal_task_entry_t)function,
                            arg);
}

//-----------------------------------------------------------------------------
// Tick
//-----------------------------------------------------------------------------

u32_t sys_now(void)
{
	return get_clock_ticks();      // * (1000 / TICKS_PER_SECOND);
}

u32_t sys_jiffies(void)
{
    return get_clock_ticks();
}

//-----------------------------------------------------------------------------

/*
 * @@END
 */
