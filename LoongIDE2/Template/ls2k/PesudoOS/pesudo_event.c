/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_event.c
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

#if PESUDO_OS_VER <= 2
extern TAILQ_HEAD(__pesudo_task_list, pesudo_task) pesudo_task_list;
#endif

extern unsigned int RunningInsideISR;

//-----------------------------------------------------------------------------

struct pesudo_event
{
    char name[PESUDO_NAME_MAX];
    uint32_t opt;                       /* 选项 */
    volatile uint32_t bits;             /* 接收到的事件 */

#if PESUDO_OS_VER >= 2
    TAILQ_ENTRY(pesudo_event) list;
#endif
#if PESUDO_OS_VER >= 3
    TAILQ_HEAD(__task_list, pesudo_task) waiting_tasks;
#endif
};

//-----------------------------------------------------------------------------
// Event 链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2
static TAILQ_HEAD(__pesudo_event_list, pesudo_event) pesudo_event_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_event_list);
#endif

static int event_count = 0;

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

struct pesudo_event *pesudo_event_create(const char *name, uint32_t opt)
{
    struct pesudo_event *event;

    event = (struct pesudo_event *)calloc(sizeof(struct pesudo_event), 1);
    if (event == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    strncpy(event->name, name, PESUDO_NAME_MAX);
    event->name[PESUDO_NAME_MAX-1] = '\0';
    event->bits = 0;
    if (opt == 0) opt = PESUDO_OPT_FIFO;
    event->opt = opt;

#if PESUDO_OS_VER >= 3
    TAILQ_INIT(&event->waiting_tasks);
#endif
#if PESUDO_OS_VER >= 2
    TAILQ_INSERT_TAIL(&pesudo_event_list, event, list);
#endif

    event_count++;
    return event;
}

void pesudo_event_delete(struct pesudo_event *event)
{
    if (event)
    {
#if PESUDO_OS_VER >= 2
        TAILQ_REMOVE(&pesudo_event_list, event, list);
#endif

        event_count--;
        free(event);
    }
}

static uint32_t pesudo_event_set_usage(struct pesudo_event *event, uint32_t bits, uint32_t flag)
{
    uint32_t use_bits = event->bits & bits;

    switch (flag)
    {
        case (EVENT_FLAG_AND):
        case (EVENT_FLAG_AND | EVENT_FLAG_CLEAR):
            if (use_bits == bits)
            {
                if (flag & EVENT_FLAG_CLEAR)
                    event->bits &= ~use_bits;
                return use_bits;
            }
            break;

        case (EVENT_FLAG_OR):
        case (EVENT_FLAG_OR | EVENT_FLAG_CLEAR):
            if (use_bits)
            {
                if (flag & EVENT_FLAG_CLEAR)
                    event->bits &= ~use_bits;
                return use_bits;
            }
            break;
    }

    return 0;
}

#if PESUDO_OS_VER >= 2
static struct pesudo_task *pesudo_event_receive_internal(struct pesudo_event *event)
{
    uint32_t use_bits;
    struct pesudo_task *task, *tmp, *find_task = NULL;

    if (!event->bits)
    {
        return NULL;
    }

#if PESUDO_OS_VER == 2
    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    if ((task->state & PT_BLOCKED_EVENT) && (task->block_obj == event))
#else
    TAILQ_FOREACH_SAFE(task, &event->waiting_tasks, list4event, tmp)
#endif
    {
        uint32_t wait_bits = task->wait_event_bits;
        uint32_t wait_flag = task->wait_event_flag;
        use_bits = event->bits & wait_bits;

#if PESUDO_OS_VER >= 3
        if ((task->state & PT_BLOCKED_EVENT) == 0)
        {
            TAILQ_REMOVE(&event->waiting_tasks, task, list4event);
        }
        else
#endif
        if (((wait_flag & EVENT_FLAG_OR ) && (use_bits != 0)) ||
            ((wait_flag & EVENT_FLAG_AND) && (use_bits == wait_bits)))
        {
            switch (event->opt & PESUDO_OPT_MASK)
            {
                case PESUDO_OPT_FIFO:
                    if (find_task == NULL)
                        find_task = task;
                    else if (task->block_when > find_task->block_when)
                        find_task = task;
                    break;

                case PESUDO_OPT_LIFO:
                    if (find_task == NULL)
                        find_task = task;
                    else if (task->block_when < find_task->block_when)
                        find_task = task;
                    break;

                default:
                    use_bits = pesudo_event_set_usage(event, wait_bits, wait_flag);
                    if (use_bits)
                    {
                        task->state |= PT_RECV_EVENT;
                        task->wait_event_bits = use_bits;
#if PESUDO_OS_VER >= 3
                        TAILQ_REMOVE(&event->waiting_tasks, task, list4event);
#endif
                    }
                    break;
            }
        }

        if (event->bits == 0)
        {
            break;
        }
    }

    if (find_task)
    {
        use_bits = pesudo_event_set_usage(event,
                        find_task->wait_event_bits,
                        find_task->wait_event_flag);
        if (use_bits)
        {
            find_task->state |= PT_RECV_EVENT;
            find_task->wait_event_bits = use_bits;
#if PESUDO_OS_VER >= 3
            TAILQ_REMOVE(&event->waiting_tasks, find_task, list4event);
#endif
        }
    }

    return find_task;
}
#endif

int pesudo_event_send(struct pesudo_event *event, uint32_t bits)
{
    if (!event || !bits)
    {
        errno = EINVAL;
        return -1;
    }

    /*
     * 记录接收事件
     */
    event->bits |= bits;

#if PESUDO_OS_VER == 1

    /**
     * 根据 opt 给阻塞任务设置接收
     */
    uint32_t use_bits;
    struct pesudo_task *task, *tmp, *find_task = NULL;
        
    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    {
        uint32_t wait_bits = task->wait_event_bits;
        uint32_t wait_flag = task->wait_event_flag;
        use_bits = event->bits & wait_bits;

        if ((task->state & PT_BLOCKED_EVENT) && (task->block_obj == event) &&
            (((wait_flag & EVENT_FLAG_OR ) && (use_bits != 0)) ||
             ((wait_flag & EVENT_FLAG_AND) && (use_bits == wait_bits))))
        {
            switch (event->opt & PESUDO_OPT_MASK)
            {
                case PESUDO_OPT_FIFO:
                    if (find_task == NULL)
                        find_task = task;
                    else if (task->block_when > find_task->block_when)
                        find_task = task;
                    break;

                case PESUDO_OPT_LIFO:
                    if (find_task == NULL)
                        find_task = task;
                    else if (task->block_when < find_task->block_when)
                        find_task = task;
                    break;

                default:
                    use_bits = pesudo_event_set_usage(event, wait_bits, wait_flag);
                    if (use_bits)
                    {
                        task->state |= PT_RECV_EVENT;
                        task->wait_event_bits = use_bits;
                    }
                    break;
            }
        }

        if (event->bits == 0)
        {
            break;
        }
    }

    if (find_task)
    {
        use_bits = pesudo_event_set_usage(event,
                        find_task->wait_event_bits,
                        find_task->wait_event_flag);
        if (use_bits)
        {
            find_task->state |= PT_RECV_EVENT;
            find_task->wait_event_bits = use_bits;
        }
    }

#elif PESUDO_OS_VER >= 2

    /**
     * process event from task immediately
     */
    if (!RunningInsideISR)
    {
        pesudo_event_receive_internal(event);
    }

#endif

    return 0;
}

uint32_t pesudo_event_receive(struct pesudo_event *event, uint32_t bits,
                              uint32_t flag, uint32_t timeout_ms)
{
    int ret;
    uint32_t use_bits = 0;

    if (!event || !bits)
    {
        errno = EINVAL;
        return -1;
    }

    switch (flag)
    {
        case (EVENT_FLAG_OR):
        case (EVENT_FLAG_AND):
        case (EVENT_FLAG_OR  | EVENT_FLAG_CLEAR):
        case (EVENT_FLAG_AND | EVENT_FLAG_CLEAR):
            break;

        default:
            flag = EVENT_FLAG_OR  | EVENT_FLAG_CLEAR;
            break;
    }

    /*
     * 需要的事件已经存在
     */
    if (event->bits != 0)
    {
        use_bits = pesudo_event_set_usage(event, bits, flag);
        if (use_bits != 0)
        {
            return use_bits;
        }
    }

    /*
     * 如果不等待, 立即返回
     */
    if (timeout_ms == 0)
    {
        errno = EBUSY;
        return 0;
    }

    if (!current_pesudo_task)       /* outside PesudoOS? */
    {
        pesudoos_dbg(PESUDOOS_DBG_EVENT, "pesudo_event_receive() can use with PesudoOS only\r\n");
        errno = EIO;
        return -1;
    }

    /*
     * 如果超时等待, 加入等待队列
     */
    if ((ret = setjmp(current_pesudo_task->func_exit_pos)) == 0)
    {
        size_t cur_ticks = get_clock_ticks();

        current_pesudo_task->wait_event_bits = bits;
        current_pesudo_task->wait_event_flag = flag;
        current_pesudo_task->block_when = cur_ticks;
        current_pesudo_task->block_until = cur_ticks + timeout_ms;
        current_pesudo_task->state &= ~PT_STATE_RUNNING;
        current_pesudo_task->state |= PT_BLOCKED_EVENT;

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = event;
#else
        TAILQ_INSERT_TAIL(&event->waiting_tasks, current_pesudo_task, list4event);
#endif

        longjmp(__mainloop_jmp_pos, -PT_BLOCKED_EVENT);
    }
    else
    {
        /*
         * blocked back
         */
        if (PT_RECV_EVENT == ret)
        {
            use_bits = current_pesudo_task->wait_event_bits;
        }

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = NULL;
#endif
        current_pesudo_task->wait_event_bits = 0;
        current_pesudo_task->wait_event_flag = 0;
        current_pesudo_task->block_when = 0;
        current_pesudo_task->block_until = 0;
        current_pesudo_task->state &= ~(PT_BLOCKED_EVENT | PT_RECV_EVENT);
        current_pesudo_task->state |= PT_STATE_RUNNING;
    }

    if (ret < 0)
    {
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&event->waiting_tasks, current_pesudo_task, list4event);
#endif
        errno = ETIMEDOUT;
    }

    return use_bits;
}

void pesudo_event_set(struct pesudo_event *event, uint32_t bits)
{
    if (event)
    {
        event->bits = bits;
    }
}

//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2

void process_pesudo_event_from_isr(void)
{
    struct pesudo_event *event, *tmp1;

    TAILQ_FOREACH_SAFE(event, &pesudo_event_list, list, tmp1)
    {
        if (event->bits != 0)
        {
            pesudo_event_receive_internal(event);
        }
    }
}

#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

