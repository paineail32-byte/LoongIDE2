/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_timer.c
 *
 * created: 2025-01-06
 *  author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>

#include "pesudoos.h"
#include "pesudo_task.h"

//-----------------------------------------------------------------------------

extern int pesudoos_task_list_add(struct pesudo_task *task);
extern int pesudoos_task_list_remove(struct pesudo_task *task);

//-----------------------------------------------------------------------------

struct pesudo_timer
{
    struct pesudo_task *tmr_task;
    uint32_t timeout_ms;
    bool is_period;

#if PESUDO_OS_VER >= 2
    TAILQ_ENTRY(pesudo_timer) list;
#endif
};

//-----------------------------------------------------------------------------
// Timer 链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2
static TAILQ_HEAD(__pesudo_timer_list, pesudo_timer) pesudo_timer_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_timer_list);
#endif

static int timer_count = 0;

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

/*
 * Timer Task 执行完成后的回调函数
 */
static void pesudo_timer_callback(void *arg)
{
    struct pesudo_task *task = (struct pesudo_task *)arg;

    if (task && task->p_timer)
    {
        struct pesudo_timer *tmr = task->p_timer;
    
        if (tmr->is_period)
        {
            task->sleep_until += tmr->timeout_ms;
            task->state = PT_STATE_SLEEP;
        }
        else
        {
            task->state = PT_STATE_SUSPEND;
        }
    }
}

struct pesudo_timer *pesudo_timer_create(const char *name,
                                         pesudo_task_entry_t entry,
                                         void *argument,
                                         uint32_t timeout_ms,
                                         bool is_period)
{
    struct pesudo_timer *tmr;

    if (!entry)
    {
        errno = EINVAL;
        return NULL;
    }

    tmr = (struct pesudo_timer *)calloc(sizeof(struct pesudo_timer), 1);
    if (tmr == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    tmr->tmr_task = pesudo_task_create(name, PESUDO_STACK_MIN, 0, entry, argument);
    if (tmr->tmr_task == NULL)
    {
        free(tmr);
        errno = ENOMEM;
        return NULL;
    }

    if (timeout_ms < PESUDO_TIMER_MS_MIN)
        timeout_ms = PESUDO_TIMER_MS_MIN;

    tmr->timeout_ms = timeout_ms;
    tmr->is_period = is_period;

    tmr->tmr_task->p_timer = tmr;
    tmr->tmr_task->timer_callback = pesudo_timer_callback;

    /*
     * 刚创建时挂起, 直到 start 时进入 sleep
     */
    tmr->tmr_task->state = PT_STATE_SUSPEND;

#if PESUDO_OS_VER >= 2
    TAILQ_INSERT_TAIL(&pesudo_timer_list, tmr, list);
#endif

    timer_count++;
    return tmr;
}

void pesudo_timer_delete(struct pesudo_timer *tmr)
{
    if (tmr)
    {
#if PESUDO_OS_VER >= 2
        TAILQ_REMOVE(&pesudo_timer_list, tmr, list);
#endif

        timer_count--;
        pesudo_task_delete(tmr->tmr_task);
        free(tmr);
    }
}

int pesudo_timer_start(struct pesudo_timer *tmr, uint32_t timeout_ms)
{
    if (tmr && tmr->tmr_task && (tmr->tmr_task->state == PT_STATE_SUSPEND))
    {
        if (timeout_ms)
        {
            if (timeout_ms < PESUDO_TIMER_MS_MIN)
                timeout_ms = PESUDO_TIMER_MS_MIN;

            tmr->timeout_ms = timeout_ms;
        }

        tmr->tmr_task->sleep_until = get_clock_ticks() + tmr->timeout_ms;
        tmr->tmr_task->state = PT_STATE_SLEEP;
    }

    return 0;
}

int pesudo_timer_stop(struct pesudo_timer *tmr)
{
    if (tmr && tmr->tmr_task)
    {
        if (tmr->tmr_task->state == PT_STATE_RUNNING)
        {
            tmr->tmr_task->state |= PT_WANT_SUSPEND;
        }
        else
        {
            tmr->tmr_task->state = PT_STATE_SUSPEND;
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


