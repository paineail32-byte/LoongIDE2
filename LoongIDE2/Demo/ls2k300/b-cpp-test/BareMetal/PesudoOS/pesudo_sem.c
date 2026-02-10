/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_sem.c
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

struct pesudo_sem
{
    char name[PESUDO_NAME_MAX];
    uint32_t opt;                       /* 选项 */
    volatile uint16_t count;            /* 计数器 */

#if PESUDO_OS_VER >= 2
    TAILQ_ENTRY(pesudo_sem) list;
#endif
#if PESUDO_OS_VER >= 3
    TAILQ_HEAD(__task_list, pesudo_task) waiting_tasks;
#endif
};

//-----------------------------------------------------------------------------
// Semphore 链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2
static TAILQ_HEAD(__pesudo_sem_list, pesudo_sem) pesudo_sem_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_sem_list);
#endif

static int sem_count = 0;

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

struct pesudo_sem *pesudo_sem_create(const char *name, uint32_t opt,
                                     uint32_t initial_count)
{
    struct pesudo_sem *sem;
    
    sem = (struct pesudo_sem *)calloc(sizeof(struct pesudo_sem), 1);
    if (sem == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    strncpy(sem->name, name, PESUDO_NAME_MAX);
    sem->name[PESUDO_NAME_MAX-1] = '\0';
    sem->count = initial_count;
    if (opt == 0) opt = PESUDO_OPT_FIFO;
    sem->opt = opt;

#if PESUDO_OS_VER >= 3
    TAILQ_INIT(&sem->waiting_tasks);
#endif
#if PESUDO_OS_VER >= 2
    TAILQ_INSERT_TAIL(&pesudo_sem_list, sem, list);
#endif

    sem_count++;
    return sem;
}

void pesudo_sem_delete(struct pesudo_sem *sem)
{
    if (sem)
    {
#if PESUDO_OS_VER >= 2
        TAILQ_REMOVE(&pesudo_sem_list, sem, list);
#endif

        sem_count--;
        free(sem);
    }
}

int pesudo_sem_obtain(struct pesudo_sem *sem, uint32_t timeout_ms)
{
    int ret = 0;

    if (!sem)
    {
        errno = EINVAL;
        return -1;
    }

    if (sem->count > 0)
    {
        sem->count--;
        return 0;
    }
    else if (!current_pesudo_task)      /* outside PesudoOS? */
    {
        pesudoos_dbg(PESUDOOS_DBG_SEM, "pesudo_sem_obtain() can use with PesudoOS only\r\n");
        errno = EIO;
        return -1;
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
        current_pesudo_task->state |= PT_BLOCKED_SEM;

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = sem;
#else
        TAILQ_INSERT_TAIL(&sem->waiting_tasks, current_pesudo_task, list4sem);
#endif

        longjmp(__mainloop_jmp_pos, -PT_BLOCKED_SEM);
    }
    else
    {
        /*
         * blocked back
         */
        if (PT_OBTAIN_SEM == ret)
        {
            // do something?
        }
        
#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = NULL;
#endif
        current_pesudo_task->block_when = 0;
        current_pesudo_task->block_until = 0;
        current_pesudo_task->state &= ~(PT_BLOCKED_SEM | PT_OBTAIN_SEM);
        current_pesudo_task->state |= PT_STATE_RUNNING;
    }

    if (ret < 0)
    {
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&sem->waiting_tasks, current_pesudo_task, list4sem);
#endif
        errno = ETIMEDOUT;
        return -1;
    }

    return 0;
}

#if PESUDO_OS_VER >= 2
static struct pesudo_task *pesudo_sem_obtain_internal(struct pesudo_sem *sem)
{
    struct pesudo_task *task, *tmp, *find_task = NULL;
    
    if (sem->count <= 0)
    {
        return NULL;
    }
    
#if PESUDO_OS_VER == 2
    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    if ((task->state & PT_BLOCKED_SEM) && (task->block_obj == sem))
#else
    TAILQ_FOREACH_SAFE(task, &sem->waiting_tasks, list4sem, tmp)
#endif
    {
#if PESUDO_OS_VER >= 3
        if ((task->state & PT_BLOCKED_SEM) == 0)
        {
            TAILQ_REMOVE(&sem->waiting_tasks, task, list4sem);
        }
        else
#endif
        {
            switch (sem->opt & PESUDO_OPT_MASK)
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
        sem->count--;

        find_task->state |= PT_OBTAIN_SEM;
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&sem->waiting_tasks, find_task, list4sem);
#endif
    }

    return find_task;
}
#endif

int pesudo_sem_release(struct pesudo_sem *sem)
{
    if (!sem)
    {
        errno = EINVAL;
        return -1;
    }

    /*
     * Increase semphore
     */
    sem->count++;

#if PESUDO_OS_VER == 1

    /**
     * 根据 OPT 给阻塞任务设置接收
     */
    struct pesudo_task *task, *tmp, *find_task = NULL;

    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    {
        if ((task->state & PT_BLOCKED_SEM) && (task->block_obj == sem))
        {
            switch (sem->opt & PESUDO_OPT_MASK)
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
        find_task->state |= PT_OBTAIN_SEM;
        sem->count--;
    }

#elif PESUDO_OS_VER >= 2

    /**
     * process sem from task immediately
     */
    if (!RunningInsideISR)
    {
        pesudo_sem_obtain_internal(sem);
    }

#endif

    return 0;
}

void pesudo_sem_clear(struct pesudo_sem *sem)
{
    if (sem)
    {
        sem->count = 0;
    }
}

//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2

void process_pesudo_sem_from_isr(void)
{
    struct pesudo_sem *sem, *tmp1;

    TAILQ_FOREACH_SAFE(sem, &pesudo_sem_list, list, tmp1)
    {
        if (sem->count > 0)
        {
            pesudo_sem_obtain_internal(sem);
        }
    }
}

#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


