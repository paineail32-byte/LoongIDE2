/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudoos.c
 *
 * created: 2025-01-06
 *  author: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/queue.h>
#include <setjmp.h>

#include "pesudoos.h"
#include "pesudo_task.h"

//-----------------------------------------------------------------------------
// extern function
//-----------------------------------------------------------------------------

/**
 * 跳转到函数执行, 并切换堆栈, 返回时换回堆栈
 */
void jmp_func_with_stack(void (*func)(void *), void *arg, size_t stack);

//-----------------------------------------------------------------------------
// Definations
//-----------------------------------------------------------------------------

#ifndef __WEAK
#define __WEAK      // __attribute__((weak))
#endif

//-----------------------------------------------------------------------------
// loacl variable
//-----------------------------------------------------------------------------

__WEAK jmp_buf  __mainloop_jmp_pos;         /* 跳转回到主循环的位置 */

__WEAK struct pesudo_task *current_pesudo_task = NULL;  /* 当前正在执行的任务 */

//-----------------------------------------------------------------------------
// 任务链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER <= 2
__WEAK TAILQ_HEAD(__pesudo_task_list, pesudo_task) pesudo_task_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_task_list);
#else 
static TAILQ_HEAD(__pesudo_task_list, pesudo_task) pesudo_task_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_task_list);
#endif

#if PESUDO_OS_VER >= 2
extern void process_pesudo_event_from_isr(void);
extern void process_pesudo_sem_from_isr(void);
extern void process_pesudo_mq_from_isr(void);
#endif

/*
 * 任务计数
 */
static int task_count = 0;     

//-----------------------------------------------------------------------------
// 链表管理
//-----------------------------------------------------------------------------

/*
 * 向链表增加一个任务
 */
int pesudoos_task_list_add(struct pesudo_task *task)
{
    struct pesudo_task *__task, *tmp;

    TAILQ_FOREACH_SAFE(__task, &pesudo_task_list, list, tmp)
    {
        if (__task == task)
        {
            errno = EEXIST;
            return -1;
        }
    }

    task_count++;
    TAILQ_INSERT_TAIL(&pesudo_task_list, task, list);

    return 0;
}

/*
 * 从链表删除一个任务
 */
int pesudoos_task_list_remove(struct pesudo_task *task)
{
    if (task == current_pesudo_task)
    {
        errno = EBUSY;
        return -1;
    }

    task_count--;
    TAILQ_REMOVE(&pesudo_task_list, task, list);

    return 0;
}

struct pesudo_task *pesudoos_task_list_first(void)
{
    return TAILQ_FIRST(&pesudo_task_list);
}

struct pesudo_task *pesudoos_task_list_next(struct pesudo_task *task)
{
    return TAILQ_NEXT(task, list);
}

//-----------------------------------------------------------------------------
// 裸机程序主循环
//-----------------------------------------------------------------------------

#if  PESUDO_OS_VER <= 2
#define BLOCKED_OBJ     (task->block_obj)
#else
#define BLOCKED_OBJ     (1)
#endif

void pesudoos_run(int deadloop)
{
    /*
     * do some initializing work here
     */

    for ( ;; )
    {
        struct pesudo_task *task, *tmp;

        TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
        {
            size_t cur_ticks = get_clock_ticks();

            /**
             * 设置当前运行的任务
             */
            current_pesudo_task = task;

            /*
             * 新建的任务总是 IDLE
             */
            if (task->state == PT_STATE_IDLE)
            {
                if (task->first_run_until <= cur_ticks)
                {
                    task->state = PT_STATE_READY;
                }
            }

            /*
             * 1st wakeup sleep task
             */
            if (task->state & PT_STATE_SLEEP)
            {
                if (task->sleep_until <= cur_ticks)
                {
                    if (task->p_timer && task->timer_callback)
                    {
                        /*
                         * Timer 进入的 sleep 直接就绪: 被延迟执行了
                         */
                        task->state = PT_STATE_READY;
                    }
                    else
                    {
                        /*
                         * pesudo_task_sleep() 进入的 sleep
                         */
                        longjmp(task->func_exit_pos, 1);
                    }
                }
            }

            /*
             * 2nd check timeout of mutex、sem、event、mq
             *
             * There is only one object is blocked anytime, never more.
             */

            else if ((task->state & PT_BLOCKED_MUTEX) && BLOCKED_OBJ)
            {
                if (task->state & PT_OBTAIN_MUTEX)          /* 已经获取 */
                {
                    longjmp(task->func_exit_pos, PT_OBTAIN_MUTEX);
                }
                else if (task->block_until <= cur_ticks)    /* 超时 */
                {
                    longjmp(task->func_exit_pos, -ETIMEDOUT);
                }
            }

            else if ((task->state & PT_BLOCKED_SEM) && BLOCKED_OBJ)
            {
                if (task->state & PT_OBTAIN_SEM)            /* 已经获取 */
                {
                    longjmp(task->func_exit_pos, PT_OBTAIN_SEM);
                }
                else if (task->block_until <= cur_ticks)    /* 超时 */
                {
                    longjmp(task->func_exit_pos, -ETIMEDOUT);
                }
            }

            else if ((task->state & PT_BLOCKED_EVENT) && BLOCKED_OBJ)
            {
                if (task->state & PT_RECV_EVENT)            /* 接收到事件 */
                {
                    longjmp(task->func_exit_pos, PT_RECV_EVENT);
                }
                else if (task->block_until <= cur_ticks)    /* 超时 */
                {
                    longjmp(task->func_exit_pos, -ETIMEDOUT);
                }
            }

            else if ((task->state & PT_BLOCKED_MQ) && BLOCKED_OBJ)
            {
                if (task->state & PT_RECV_MQ)               /* 接收到消息 */
                {
                    longjmp(task->func_exit_pos, PT_RECV_MQ);
                }
                else if (task->block_until <= cur_ticks)    /* 超时 */
                {
                    longjmp(task->func_exit_pos, -ETIMEDOUT);
                }
            }

            /*
             * 3rd run ready task
             */
            else if (task->state == PT_STATE_READY)
            {
                task->state = PT_STATE_RUNNING;
                task->run_begintick = cur_ticks;

                if (setjmp(__mainloop_jmp_pos) == 0)
                {
                    jmp_func_with_stack(task->handler,
                                        task->arg,
                                        task->stack_cur_top);
                }
                else
                {
                    /*
                     * return from blocked
                     */

                    // do something here?

                }

                /*
                 * 检查任务堆栈是否越界
                 */
                if (task->stack_base[0] != PESUDO_STACK_MAGIC)
                {
                    pesudoos_dbg(PESUDOOS_DBG_TASK, "%s's stack is overflow!\r\n", task->task_name);
                    
                    while (1)
                    {
                        asm volatile( "nop" );
                    }
                }

                /*
                 * 运行可能被阻塞
                 */
                cur_ticks = get_clock_ticks();
                task->run_ticks = cur_ticks - task->run_begintick;
                task->run_begintick = cur_ticks;

                /*
                 * 运行结束
                 */
                if (task->state & PT_STATE_RUNNING)
                {
                    task->run_count++;

                    /*
                     * 检查是不是定时器函数: 回调确定下次执行时间
                     */
                    if (task->p_timer && task->timer_callback)
                    {
                        task->timer_callback(task);
                    }

                    else if (task->state & PT_WANT_SUSPEND)
                    {
                        task->state = PT_STATE_SUSPEND;
                    }
                    else
                    {
                        task->state = PT_STATE_READY;
                    }
                }
            }

            /**********************************************
             * go on next task
             */
        }

        current_pesudo_task = NULL;

        /*
         * 检查是否有待删除的任务
         */
        TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
        {
            if ((task->state & PT_STATE_RUNNING) &&
                (task->state == PT_WANT_DELETE))
            {
                pesudo_task_delete(task);
            }
        }

#if PESUDO_OS_VER >= 2

        /*
         * 延迟处理来自中断的 Event MQ Sem
         */
        process_pesudo_event_from_isr();
        process_pesudo_sem_from_isr();
        process_pesudo_mq_from_isr();

#endif

        /*
         * 退出循环意味着有调用者执行主循环
         */
        if (!deadloop)
        {
            break;
        }
    }

    current_pesudo_task = NULL;
}

//-----------------------------------------------------------------------------

int pesudoos_is_running(void)
{
    return current_pesudo_task ? 1 : 0;
}

//-----------------------------------------------------------------------------

#include <stdarg.h>

static unsigned int m_pesudoos_dbg_mask = 0;

void pesudoos_set_dbg_mask(unsigned int mask)
{
    m_pesudoos_dbg_mask = mask;
}

void pesudoos_dbg(unsigned int mask, const char *fmt, ...)
{
    va_list ap;

    if (mask & m_pesudoos_dbg_mask)
    {
        va_start(ap, fmt);
        printk(fmt, ap);
        va_end(ap);
    }
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

