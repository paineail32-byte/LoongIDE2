/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * osal_pesudoos.c
 *
 * created: 2025-01-17
 *  author: 
 */

#ifdef OS_PESUDO

#include <stdlib.h>

#include <larchintrin.h>
#include "cpu.h"

#include "osal.h"

#include "pesudoos.h"
#include "pesudo_task.h"

#if 0
extern void printk(const char *fmt, ...);
#define debug_mq(...)   printk(__VA_ARGS__)
#else
#define debug_mq(...)
#endif

//-----------------------------------------------------------------------------
// Task
//-----------------------------------------------------------------------------

osal_task_t osal_task_create(const char *name,
                             uint32_t stack_size,
                             uint32_t prio,
                             uint32_t slice,
                             osal_task_entry_t entry,
                             void *args)
{
    struct pesudo_task *task;
    task = pesudo_task_create(name,
                              stack_size,
                              0,
                              (pesudo_task_entry_t)entry,
                              args);

    if (task == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TASK_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }

    return (osal_task_t)task;
}

void osal_task_delete(osal_task_t task)
{
    struct pesudo_task *p_task = (struct pesudo_task *)task;
    if (p_task->user_data)
    {
        free(p_task->user_data);    /* 用户加上的数据 */
        p_task->user_data = NULL;
    }

    pesudo_task_delete(p_task);
}

void osal_task_suspend(osal_task_t task)
{
    pesudo_task_suspend((struct pesudo_task *)task);
}

void osal_task_resume(osal_task_t task)
{
    pesudo_task_resume((struct pesudo_task *)task);
}

void osal_task_sleep(uint32_t ms)
{
    pesudo_task_sleep(ms);
}

void osal_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks)
{
    if (prev_ticks)
        pesudo_task_sleep_until(prev_ticks, inc_ticks);
    else
        pesudo_task_sleep(inc_ticks);
}

//-----------------------------------------------------------------------------
// Event
//-----------------------------------------------------------------------------

osal_event_t osal_event_create(const char *name, uint32_t opt)
{
    struct pesudo_event *event;
    uint32_t os_opt = 0;

    if (opt & OSAL_OPT_FIFO) os_opt |= PESUDO_OPT_FIFO;
    if (opt & OSAL_OPT_LIFO) os_opt |= PESUDO_OPT_LIFO;
    if (os_opt == 0) os_opt = PESUDO_OPT_FIFO;

    event = pesudo_event_create(name, os_opt);

    if (event == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_EVENT_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }

    return (osal_event_t)event;
}

void osal_event_delete(osal_event_t event)
{
    pesudo_event_delete(event);
}

int osal_event_send(osal_event_t event, uint32_t bits)
{
    return pesudo_event_send((struct pesudo_event *)event, bits);
}

uint32_t osal_event_receive(osal_event_t event, uint32_t bits,
                            uint32_t flag, uint32_t timeout_ms)
{
    uint32_t os_flag = 0;

    if (flag & OSAL_EVENT_FLAG_AND)   os_flag |= EVENT_FLAG_AND;
    if (flag & OSAL_EVENT_FLAG_OR)    os_flag |= EVENT_FLAG_OR;
    if (flag & OSAL_EVENT_FLAG_CLEAR) os_flag |= EVENT_FLAG_CLEAR;
    if (os_flag == 0) os_flag = EVENT_FLAG_OR | EVENT_FLAG_CLEAR;

    return pesudo_event_receive((struct pesudo_event *)event, bits, os_flag, timeout_ms);
}

void osal_event_set_bits(osal_event_t event, uint32_t bits)
{
    pesudo_event_set((struct pesudo_event *)event, bits);
}

void osal_event_set_os_opt(osal_event_t event, uint32_t opt)
{
    return;
}

//-----------------------------------------------------------------------------
// Semphore
//-----------------------------------------------------------------------------

osal_sem_t osal_sem_create(const char *name, uint32_t opt, uint32_t initial_count)
{
    uint32_t os_opt = 0;
    struct pesudo_sem *sem;

    if (opt & OSAL_OPT_FIFO) os_opt |= PESUDO_OPT_FIFO;
    if (opt & OSAL_OPT_LIFO) os_opt |= PESUDO_OPT_LIFO;
    if (os_opt == 0) os_opt = PESUDO_OPT_FIFO;

    sem = pesudo_sem_create(name, os_opt, initial_count);
    if (sem == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_SEM_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_sem_t)sem;
}

void osal_sem_delete(osal_sem_t sem)
{
    pesudo_sem_delete((struct pesudo_sem *)sem);
}

int osal_sem_obtain(osal_sem_t sem, uint32_t timeout)
{
    if (pesudo_sem_obtain((struct pesudo_sem *)sem, timeout) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

int osal_sem_release(osal_sem_t sem)
{
    if (pesudo_sem_release((struct pesudo_sem *)sem) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

void osal_sem_reset(osal_sem_t sem)
{
    pesudo_sem_clear((struct pesudo_sem *)sem);
}

void osal_sem_set_os_opt(osal_sem_t sem, uint32_t opt)
{
    return;
}

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

osal_mutex_t osal_mutex_create(const char *name, uint32_t opt)
{
    uint32_t os_opt = 0;
    struct pesudo_mutex *mutex;

    if (opt & OSAL_OPT_FIFO) os_opt |= PESUDO_OPT_FIFO;
    if (opt & OSAL_OPT_LIFO) os_opt |= PESUDO_OPT_LIFO;
    if (os_opt == 0) os_opt = PESUDO_OPT_FIFO;

    mutex = pesudo_mutex_create(name, os_opt);
    if (mutex == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_MUTEX_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_mutex_t)mutex;
}

void osal_mutex_delete(osal_mutex_t mutex)
{
    pesudo_mutex_delete((struct pesudo_mutex *)mutex);
}

int osal_mutex_obtain(osal_mutex_t mutex, uint32_t timeout_ms)
{
    if (pesudo_mutex_obtain((struct pesudo_mutex *)mutex, timeout_ms) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

int osal_mutex_release(osal_mutex_t mutex)
{
    if (pesudo_mutex_release((struct pesudo_mutex *)mutex) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

void osal_mutex_set_os_opt(osal_mutex_t mutex, uint32_t opt)
{
    return;
}

//-----------------------------------------------------------------------------
// Message Queue: Use message pointer
//-----------------------------------------------------------------------------

osal_mq_t osal_mq_create(const char *name, uint32_t opt,
                         uint32_t item_size, uint32_t max_msgs)
{
    uint32_t os_opt = 0;
    struct pesudo_mq *mq;

    (void)item_size;
    (void)max_msgs;
    
    if (opt & OSAL_OPT_FIFO) os_opt |= PESUDO_OPT_FIFO;
    if (opt & OSAL_OPT_LIFO) os_opt |= PESUDO_OPT_LIFO;
    if (os_opt == 0) os_opt = PESUDO_OPT_FIFO;

    mq = pesudo_mq_create(name, os_opt, item_size, max_msgs);
    if (mq == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_MQ_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_mq_t)mq;
}

void osal_mq_delete(osal_mq_t mq)
{
    pesudo_mq_delete((struct pesudo_mq *)mq);
}

int osal_mq_send(osal_mq_t mq, const void *msg, int size)
{
    debug_mq("mq send: 0x%" PRIx64 "\r\n", *(uint64_t *)msg);

    if (pesudo_mq_send((struct pesudo_mq *)mq, msg) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

int osal_mq_receive(osal_mq_t mq, void *msg, int size, uint32_t timeout)
{
    if (pesudo_mq_receive((struct pesudo_mq *)mq, (void *)msg, timeout) < 0)
    {
        return -OSAL_ERR_TIMEOUT;
    }

    debug_mq("mq recv: 0x%" PRIx64 "\r\n", *(uint64_t *)msg);

    return size;
}

void osal_mq_set_os_opt(osal_mq_t mq, uint32_t opt)
{
    return;
}

int osal_mq_is_full(osal_mq_t mq)
{
    return pesudo_mq_is_full((struct pesudo_mq *)mq);
}

int osal_mq_flush(osal_mq_t mq)
{
    return pesudo_mq_flush((struct pesudo_mq *)mq);
}

//-----------------------------------------------------------------------------
// Timer
//-----------------------------------------------------------------------------

osal_timer_t osal_timer_create(const char *name,
                               osal_task_entry_t handler,
                               void *argument,
                               uint32_t timeout_ms,
                               bool is_period)
{
    struct pesudo_timer *tmr;
    tmr = pesudo_timer_create(name,
                              (pesudo_task_entry_t)handler,
                              argument,
                              timeout_ms,
                              is_period);

    if (tmr == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TIMER_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_timer_t )tmr;
}

void osal_timer_delete(osal_timer_t timer)
{
    pesudo_timer_delete((struct pesudo_timer *)timer);
}

void osal_timer_start(osal_timer_t timer, uint32_t timeout_ms)
{
    pesudo_timer_start((struct pesudo_timer *)timer, timeout_ms);
}

void osal_timer_stop(osal_timer_t timer)
{
    pesudo_timer_stop((struct pesudo_timer *)timer);
}

//-----------------------------------------------------------------------------

/*
 * BareMetal don't need into critical section
 */
size_t osal_enter_critical_section(void)
{
#if 1
    unsigned long crmd = __csrrd_d(LA_CSR_CRMD);
    __csrxchg_d(~CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD);
    return (size_t)crmd;
#else
    return 0;
#endif
}

void osal_leave_critical_section(size_t flag)
{
#if 1
    __csrwr_d((unsigned long)flag, LA_CSR_CRMD);
#else
    (void)flag;
#endif
}

//-----------------------------------------------------------------------------

void osal_msleep(uint32_t ms)
{
	if (ms > 0)
	{
		if (current_pesudo_task)
			pesudo_task_sleep(ms);
		else
			delay_ms(ms);
	}
}

//-----------------------------------------------------------------------------

void *osal_malloc(size_t size)
{
    return malloc(size);
}

void osal_free(void *ptr)
{
    free(ptr);
}

//-----------------------------------------------------------------------------

int osal_is_osrunning(void)
{
    return current_pesudo_task ? 1 : 0;
}

#endif // #ifdef OS_PESUDO

//-----------------------------------------------------------------------------

/*
 * @@END
 */
