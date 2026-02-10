/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * osal_rtthread.c
 *
 * created: 2025-01-17
 *  author: 
 */

#ifdef OS_RTTHREAD

#include <malloc.h>

#include "osal.h"

#include "rtthread.h"
#include "rthw.h"

#if 0
#include <inttypes.h>
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
    rt_thread_t htask;

    if (slice == 0) slice = 10;

    htask = rt_thread_create(name,
                             entry,
                             args,
                             (rt_uint32_t)stack_size,
                             (rt_uint8_t)prio,
                             (rt_uint32_t)slice);

    if (htask == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TASK_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }

    rt_thread_startup(htask);
    return (osal_task_t)htask;
}

void osal_task_delete(osal_task_t task)
{
    if (task != NULL)
    {
        if (((rt_thread_t)task)->user_data)
            free((void *)((rt_thread_t)task)->user_data);
        rt_thread_delete((rt_thread_t)task);
    }
}

void osal_task_suspend(osal_task_t task)
{
    if (task != NULL)
    {
        rt_thread_suspend((rt_thread_t)task);
    }
}

void osal_task_resume(osal_task_t task)
{
    if (task != NULL)
    {
        rt_thread_resume((rt_thread_t)task);
    }
}

void osal_task_sleep(uint32_t ms)
{
    rt_thread_delay((rt_tick_t)ms);
}

void osal_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks)
{
    if (prev_ticks)
        rt_thread_delay_until(prev_ticks, inc_ticks);
    else
        rt_thread_delay(inc_ticks);
}

//-----------------------------------------------------------------------------
// Event
//-----------------------------------------------------------------------------

osal_event_t osal_event_create(const char *name, uint32_t opt)
{
    uint32_t os_opt = 0;
    rt_event_t event;

    if (opt & OSAL_OPT_FIFO) os_opt |= RT_IPC_FLAG_FIFO;
    if (opt & OSAL_OPT_PRIO) os_opt |= RT_IPC_FLAG_PRIO;

    event = rt_event_create(name, (rt_uint8_t)os_opt);
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
    rt_event_delete((rt_event_t)event);
}

int osal_event_send(osal_event_t event, uint32_t bits)
{
    return rt_event_send((rt_event_t)event, bits);
}

uint32_t osal_event_receive(osal_event_t event, uint32_t bits,
                            uint32_t flag, uint32_t timeout_ms)
{
    uint32_t os_flag = 0;
    uint32_t recv_event = 0;
    
    if (flag & OSAL_EVENT_FLAG_AND)   os_flag |= RT_EVENT_FLAG_AND;
    if (flag & OSAL_EVENT_FLAG_OR)    os_flag |= RT_EVENT_FLAG_OR;
    if (flag & OSAL_EVENT_FLAG_CLEAR) os_flag |= RT_EVENT_FLAG_CLEAR;

    if (rt_event_recv((rt_event_t)event,
                      bits,
                      (rt_uint8_t)os_flag,
                      (timeout_ms == OSAL_WAIT_FOREVER) ? RT_WAITING_FOREVER : timeout_ms,
                      &recv_event) == RT_EOK)
    {
        return recv_event;
    }

    return 0;
}

void osal_event_set_bits(osal_event_t event, uint32_t bits)
{
    ((rt_event_t)event)->set = bits;
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
    rt_sem_t sem;

    if (opt & OSAL_OPT_FIFO) os_opt |= RT_IPC_FLAG_FIFO;
    if (opt & OSAL_OPT_PRIO) os_opt |= RT_IPC_FLAG_PRIO;

    sem = rt_sem_create(name, initial_count, (rt_uint8_t)os_opt);

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
    rt_sem_delete((rt_sem_t)sem);
}

int osal_sem_obtain(osal_sem_t sem, uint32_t timeout)
{
    if (rt_sem_take((rt_sem_t)sem,
                    (timeout != OSAL_WAIT_FOREVER) ?
                     timeout : RT_WAITING_FOREVER) != RT_EOK)
    {
        return -OSAL_ERR_TIMEOUT;
    }
    
    return 0;
}

int osal_sem_release(osal_sem_t sem)
{
    return (int)rt_sem_release((rt_sem_t)sem);
}

void osal_sem_reset(osal_sem_t sem)
{
    ((rt_sem_t)sem)->value = 0;
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
    uint32_t os_opt = RT_IPC_FLAG_PRIO;
    rt_mutex_t mutex;

    mutex = rt_mutex_create(name, (rt_uint8_t)os_opt);
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
    rt_mutex_delete((rt_mutex_t)mutex);
}

int osal_mutex_obtain(osal_mutex_t mutex, uint32_t timeout_ms)
{
    if (rt_mutex_take((rt_mutex_t)mutex,
                      (timeout_ms != OSAL_WAIT_FOREVER) ?
                       timeout_ms : RT_WAITING_FOREVER) != RT_EOK)
                               
    {
        return -OSAL_ERR_TIMEOUT;
    }

    return 0;
}

int osal_mutex_release(osal_mutex_t mutex)
{
    return (int)rt_mutex_release((rt_mutex_t)mutex);
}

void osal_mutex_set_os_opt(osal_mutex_t mutex, uint32_t opt)
{
    return;
}

//-----------------------------------------------------------------------------
// Message Queue
//-----------------------------------------------------------------------------

osal_mq_t osal_mq_create(const char *name, uint32_t opt,
                         uint32_t item_size, uint32_t max_msgs)
{
    uint32_t os_opt = 0;
    rt_mq_t mq;

    if (opt & OSAL_OPT_FIFO) os_opt |= RT_IPC_FLAG_FIFO;
    if (opt & OSAL_OPT_PRIO) os_opt |= RT_IPC_FLAG_PRIO;

    mq = rt_mq_create(name,
                      (rt_size_t)item_size,
                      (rt_size_t)max_msgs,
                      (rt_uint8_t)os_opt);
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
    rt_mq_delete((rt_mq_t)mq);
}

int osal_mq_send(osal_mq_t mq, const void *msg, int size)
{
	debug_mq("mq send: 0x%" PRIx64 "\r\n", *(uint64_t *)msg);

    return rt_mq_send((rt_mq_t)mq, msg, (rt_size_t)size);
}

int osal_mq_receive(osal_mq_t mq, void *msg, int size, uint32_t timeout)
{
    if (rt_mq_recv((rt_mq_t)mq,
                    msg,
                   (rt_size_t)size,
                   (timeout != OSAL_WAIT_FOREVER) ?
                    timeout : RT_WAITING_FOREVER) != RT_EOK)
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
    return 0;
}

int osal_mq_flush(osal_mq_t mq)
{
    return 0;
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
    rt_timer_t tmr;

    tmr = rt_timer_create(name,
                          handler,
                          argument,
                          timeout_ms,
                          is_period ? (RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER)
                                    : (RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER)
                          );

    if (tmr == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TIMER_FAIL "\r\n");
        while (1)
        {
            //
        }
    }
    
    return (osal_timer_t)tmr;
}

void osal_timer_delete(osal_timer_t timer)
{
    rt_timer_stop((rt_timer_t)timer);
    rt_timer_delete((rt_timer_t)timer);
    rt_free((rt_timer_t)timer);
}

void osal_timer_start(osal_timer_t timer, uint32_t timeout_ms)
{
    rt_timer_start((rt_timer_t)timer);
}

void osal_timer_stop(osal_timer_t timer)
{
    rt_timer_stop((rt_timer_t)timer);
}

//-----------------------------------------------------------------------------

size_t osal_enter_critical_section(void)
{
    return rt_hw_interrupt_disable();
}

void osal_leave_critical_section(size_t flag)
{
    rt_hw_interrupt_enable(flag);
}

//-----------------------------------------------------------------------------

void osal_msleep(uint32_t ms)
{
	if (ms > 0)
	{
		if (rt_thread_os_running)
			rt_thread_delay(ms);
		else
			delay_ms(ms);
	}
}

//-----------------------------------------------------------------------------

void *osal_malloc(size_t size)
{
    return rt_malloc(size);
}

void osal_free(void *ptr)
{
    rt_free(ptr);
}

//-----------------------------------------------------------------------------

int osal_is_osrunning(void)
{
    return rt_thread_os_running;
}

#endif // #ifdef OS_RTTHREAD

//-----------------------------------------------------------------------------

/*
 * @@END
 */

