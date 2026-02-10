/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_mutex.c
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

struct pesudo_mutex
{
    char name[PESUDO_NAME_MAX];
    uint32_t opt;                       /* 选项 */
    volatile int counter;               /* 拥有计数 */
    volatile struct pesudo_task *owner; /* 拥有改互斥量的任务 */

#if PESUDO_OS_VER >= 2
    TAILQ_ENTRY(pesudo_mutex) list;
#endif
#if PESUDO_OS_VER >= 3
    TAILQ_HEAD(__task_list, pesudo_task) waiting_tasks;
#endif
};

//-----------------------------------------------------------------------------
// Mutex 链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2
static TAILQ_HEAD(__pesudo_mutex_list, pesudo_mutex) pesudo_mutex_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_mutex_list);
#endif

static int mutex_count = 0;

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

struct pesudo_mutex *pesudo_mutex_create(const char *name, uint32_t opt)
{
    struct pesudo_mutex *mutex;

    mutex = (struct pesudo_mutex *)calloc(sizeof(struct pesudo_mutex), 1);
    if (mutex == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    strncpy(mutex->name, name, PESUDO_NAME_MAX);
    mutex->name[PESUDO_NAME_MAX-1] = '\0';
    mutex->counter = 0;
    if (opt == 0) opt = PESUDO_OPT_FIFO;
    mutex->opt = opt;

#if PESUDO_OS_VER >= 3
    TAILQ_INIT(&mutex->waiting_tasks);
#endif
#if PESUDO_OS_VER >= 2
    TAILQ_INSERT_TAIL(&pesudo_mutex_list, mutex, list);
#endif

    mutex_count++;
    return mutex;
}

void pesudo_mutex_delete(struct pesudo_mutex *mutex)
{
    if (mutex)
    {
#if PESUDO_OS_VER >= 2
        TAILQ_REMOVE(&pesudo_mutex_list, mutex, list);
#endif

        mutex_count--;
        free(mutex);
    }
}

int pesudo_mutex_obtain(struct pesudo_mutex *mutex, uint32_t timeout_ms)
{
    int ret = 0;

    if (!mutex)
    {
        errno = EINVAL;
        return -1;
    }

    if (RunningInsideISR)
    {
        errno = EACCES;
        return -1;
    }

    if (!mutex->owner && current_pesudo_task)   /* 占用 */
    {
        mutex->owner = current_pesudo_task;
        mutex->counter = 1;
        return 0;
    }
    else if (!current_pesudo_task)              /* outside PesudoOS? */
    {
        pesudoos_dbg(PESUDOOS_DBG_MUTEX, "pesudo_mutex_obtain() can use with PesudoOS only\r\n");
        errno = EIO;
        return -1;
    }

    if (mutex->owner == current_pesudo_task)    /* 本任务继续占用, 增加计数 */
    {
        mutex->counter++;
        return 0;
    }

    if (timeout_ms == 0)
    {
        errno = EBUSY;
        return -1;
    }

    /*
     * 如果超时等待, 加入等待队列
     */
    if ((ret = setjmp(current_pesudo_task->func_exit_pos)) == 0)
    {
        size_t cur_ticks = get_clock_ticks();

        current_pesudo_task->block_when = cur_ticks;
        current_pesudo_task->block_until = cur_ticks + timeout_ms;
        current_pesudo_task->state &= ~PT_STATE_RUNNING;
        current_pesudo_task->state |= PT_BLOCKED_MUTEX;

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = mutex;
#else
        TAILQ_INSERT_TAIL(&mutex->waiting_tasks, current_pesudo_task, list4mutex);
#endif

        longjmp(__mainloop_jmp_pos, -PT_BLOCKED_MUTEX);
    }
    else
    {
        /*
         * blocked back
         */
        if (PT_OBTAIN_MUTEX == ret)
        {
            // do something?
        }

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = NULL;
#endif
        current_pesudo_task->block_when = 0;
        current_pesudo_task->block_until = 0;
        current_pesudo_task->state &= ~(PT_BLOCKED_MUTEX | PT_OBTAIN_MUTEX);
        current_pesudo_task->state |= PT_STATE_RUNNING;
    }

    if (ret < 0)
    {
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&mutex->waiting_tasks, current_pesudo_task, list4mutex);
#endif
        errno = ETIMEDOUT;
        return -1;
    }

    return 0;
}

int pesudo_mutex_release(struct pesudo_mutex *mutex)
{
    if (!mutex)
    {
        errno = EINVAL;
        return -1;
    }

    if (RunningInsideISR)
    {
        errno = EACCES;
        return -1;
    }

    /*
     * Set access
     */
    if (mutex->owner && (mutex->owner == current_pesudo_task))
    {
        mutex->counter--;
        if (mutex->counter == 0)
            mutex->owner = NULL;
    }
    else if (!current_pesudo_task)          /* outside PesudoOS? */
    {
        pesudoos_dbg(PESUDOOS_DBG_MUTEX, "pesudo_mutex_release() can use with PesudoOS only\r\n");
        errno = EIO;
        return -1;
    }

    /**
     * 已经释放, 根据 opt 给阻塞任务设置占用
     */
    if (mutex->owner == NULL)
    {
        struct pesudo_task *task, *tmp, *find_task = NULL;

#if PESUDO_OS_VER <= 2
        TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
        if ((task->state & PT_BLOCKED_MUTEX) && (task->block_obj == mutex))
#else
        TAILQ_FOREACH_SAFE(task, &mutex->waiting_tasks, list4mutex, tmp)
#endif
        {
#if PESUDO_OS_VER >= 3
            if ((task->state & PT_BLOCKED_MUTEX) == 0)
            {
                TAILQ_REMOVE(&mutex->waiting_tasks, task, list4mutex);
            }
            else
#endif
            {
                switch (mutex->opt & PESUDO_OPT_MASK)
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
                }
            }
        }

        if (find_task)
        {
            mutex->owner = find_task;
            mutex->counter++;

            find_task->state |= PT_OBTAIN_MUTEX;
#if PESUDO_OS_VER >= 3
            TAILQ_REMOVE(&mutex->waiting_tasks, find_task, list4mutex);
#endif
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


