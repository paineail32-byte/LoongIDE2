/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_task.c
 *
 * created: 2025-01-06
 *  author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>
#include <setjmp.h>

#include "pesudoos.h"
#include "pesudo_task.h"

//-----------------------------------------------------------------------------

extern int pesudoos_task_list_add(struct pesudo_task *task);
extern int pesudoos_task_list_remove(struct pesudo_task *task);

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

struct pesudo_task *pesudo_task_create(const char *name,
                                       uint32_t stack_size,
                                       uint32_t run_after_ms,
                                       pesudo_task_entry_t entry,
                                       void *args)
{
    struct pesudo_task *task;
    
    if (NULL == entry)
    {
        errno = EINVAL;
        return NULL;
    }
    
    task = (struct pesudo_task *)calloc(sizeof(struct pesudo_task), 1);
    if (NULL == task)
    {
        errno = ENOMEM;
        return NULL;
    }
    
    stack_size &= ~0x07;
    if (stack_size < PESUDO_STACK_MIN)
        stack_size = PESUDO_STACK_MIN;

    task->stack_size = stack_size;
    task->stack_base = (size_t *)calloc(1, stack_size);
    if (NULL == task->stack_base)
    {
        free(task);
        errno = ENOMEM;
        return NULL;
    }

    /*
     * ÔÚ task->stack_base ´¦Ð´ 1 ¸ö PESUDO_STACK_MAGIC
     */
    task->stack_base[0] = PESUDO_STACK_MAGIC;
    task->stack_cur_top = (size_t)task->stack_base + stack_size - 8 * sizeof(size_t); /* PAD */

    strncpy(task->task_name, name, PESUDO_NAME_MAX);
    task->task_name[PESUDO_NAME_MAX-1] = '\0';
    task->handler = entry;
    task->arg = args;
    task->state = PT_STATE_IDLE;
    task->first_run_until = get_clock_ticks() + run_after_ms;
    task->ID = (uintptr_t)task;     /* TODO */

    pesudoos_task_list_add(task);
    
    return task;
}

void pesudo_task_delete(struct pesudo_task *task)
{
    if (!task)
    {
        return;
    }

    if ((task->state == PT_STATE_IDLE)  ||
        (task->state == PT_STATE_READY) ||
        (task->state == PT_STATE_SUSPEND))
    {
        if (pesudoos_task_list_remove(task) == 0)
        {
            free(task->stack_base);
            free(task);
        }
    }
    else if ((task->state == PT_STATE_RUNNING) || IS_BLOCKED(task->state))
    {
        task->state |= PT_WANT_DELETE;
    }
}

void pesudo_task_suspend(struct pesudo_task *task)
{
    if (!task)
    {
        return;
    }

    if ((task->state == PT_STATE_IDLE)  ||
        (task->state == PT_STATE_READY) ||
        (task->state == PT_STATE_SUSPEND))
    {
        task->state = PT_STATE_SUSPEND;
    }
    else if ((task->state == PT_STATE_RUNNING) || IS_BLOCKED(task->state))
    {
        task->state |= PT_WANT_SUSPEND;
    }
}

void pesudo_task_resume(struct pesudo_task *task)
{
    if (task && (task->state == PT_STATE_SUSPEND))
    {
        task->state = PT_STATE_READY;
    }
}

void pesudo_task_sleep(uint32_t ms)
{
    if (current_pesudo_task && ms)
    {
        if (setjmp(current_pesudo_task->func_exit_pos) == 0)
        {
            size_t cur_ticks = get_clock_ticks();

            current_pesudo_task->sleep_until = cur_ticks + ms;
            current_pesudo_task->state &= ~PT_STATE_RUNNING;
            current_pesudo_task->state |= PT_STATE_SLEEP;

            longjmp(__mainloop_jmp_pos, -PT_STATE_SLEEP);
        }
        else
        {
            /*
             * wakeup
             */
            current_pesudo_task->sleep_until = 0;
            current_pesudo_task->state &= ~PT_STATE_SLEEP;
            current_pesudo_task->state |= PT_STATE_RUNNING;
        }
    }
}

void pesudo_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks)
{
    uint32_t cur_ticks = (uint32_t)get_clock_ticks();

    if (current_pesudo_task && (*prev_ticks + inc_ticks > cur_ticks))
    {
        if (setjmp(current_pesudo_task->func_exit_pos) == 0)
        {
            current_pesudo_task->sleep_until = *prev_ticks + inc_ticks;
            current_pesudo_task->state &= ~PT_STATE_RUNNING;
            current_pesudo_task->state |= PT_STATE_SLEEP;

            longjmp(__mainloop_jmp_pos, -PT_STATE_SLEEP);
        }
        else
        {
            /*
             * wakeup
             */
            *prev_ticks += inc_ticks;       /* for next delay */
            cur_ticks = get_clock_ticks();
            current_pesudo_task->wakeup_ticks = cur_ticks;

            current_pesudo_task->sleep_until = 0;
            current_pesudo_task->state &= ~PT_STATE_SLEEP;
            current_pesudo_task->state |= PT_STATE_RUNNING;
        }
    }
    else
    {
        delay_ms(inc_ticks);
    }
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


