/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_mq.c
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

#if PESUDO_OS_VER <= 2
extern TAILQ_HEAD(__pesudo_task_list, pesudo_task) pesudo_task_list;
#endif

extern unsigned int RunningInsideISR;

//-----------------------------------------------------------------------------

struct pesudo_mq
{
    char name[PESUDO_NAME_MAX];
    uint32_t opt;                       /* 标志 */
    uint32_t item_size;                 /* Message 项大小 */
    uint32_t item_count;                /* Message 项目数 */
    int      msg_count;                 /* 缓冲的消息数 */
    int      wr_index;                  /* 写入项索引 */
    int      rd_index;                  /* 读取项索引 */
    uint8_t *data;                      /* Message 数据 */

#if PESUDO_OS_VER >= 2
    TAILQ_ENTRY(pesudo_mq) list;
#endif
#if PESUDO_OS_VER >= 3
    TAILQ_HEAD(__task_list, pesudo_task) waiting_tasks;
#endif
};

#define MQ_OPT_EMPTY    0x00010000
#define MQ_OPT_FULL     0x00020000

//-----------------------------------------------------------------------------
// MQ 链表
//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2
static TAILQ_HEAD(__pesudo_mq_list, pesudo_mq) pesudo_mq_list = \
       TAILQ_HEAD_INITIALIZER(pesudo_mq_list);
#endif

//-----------------------------------------------------------------------------
// Implement
//-----------------------------------------------------------------------------

struct pesudo_mq *pesudo_mq_create(const char *name, uint32_t opt,
                                   uint32_t item_size, uint32_t max_msgs)
{
    struct pesudo_mq *mq;

    if ((item_size == 0) || (max_msgs == 0))
    {
        errno = EINVAL;
        return NULL;
    }

    mq = (struct pesudo_mq *)calloc(sizeof(struct pesudo_mq), 1);
    if (mq == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    mq->data = (uint8_t *)malloc(item_size * max_msgs);
    if (mq->data == NULL)
    {
        free(mq);
        errno = ENOMEM;
        return NULL;
    }

    strncpy(mq->name, name, PESUDO_NAME_MAX);
    mq->name[PESUDO_NAME_MAX-1] = '\0';
    mq->item_size = item_size;
    mq->item_count = max_msgs;
    if (opt == 0) opt = PESUDO_OPT_FIFO;
    mq->opt = MQ_OPT_EMPTY | opt;

#if PESUDO_OS_VER >= 3
    TAILQ_INIT(&mq->waiting_tasks);
#endif
#if PESUDO_OS_VER >= 2
    TAILQ_INSERT_TAIL(&pesudo_mq_list, mq, list);
#endif

    return mq;
}

void pesudo_mq_delete(struct pesudo_mq *mq)
{
    if (mq)
    {
#if PESUDO_OS_VER >= 2
        TAILQ_REMOVE(&pesudo_mq_list, mq, list);
#endif

        free(mq->data);
        free(mq);
    }
}

/**
 * mq->count > 0
 */
static int pesudo_mq_read(struct pesudo_mq *mq, void *msg)
{
    uint8_t *ptr = mq->data + mq->item_size * mq->rd_index;

    memcpy(msg, ptr, mq->item_size);

    mq->msg_count--;
    mq->rd_index++;
    if (mq->rd_index >= mq->item_count)
        mq->rd_index = 0;

    if (mq->msg_count == 0)
        mq->opt |= MQ_OPT_EMPTY;
    else
        mq->opt &= ~MQ_OPT_EMPTY;

    return mq->item_size;       /* 返回 Message 项大小 */
}

/**
 * mq->count < mq->item_count
 */
static int pesudo_mq_write(struct pesudo_mq *mq, const void *msg)
{
    uint8_t *ptr = mq->data + mq->item_size * mq->wr_index;

    memcpy(ptr, msg, mq->item_size);

    mq->msg_count++;
    mq->wr_index++;
    if (mq->wr_index >= mq->item_count)
        mq->wr_index = 0;

    if (mq->msg_count == mq->item_count)
        mq->opt |= MQ_OPT_FULL;
    else
        mq->opt &= ~MQ_OPT_FULL;

    return mq->item_size;       /* 返回 Message 项大小 */
}

#if PESUDO_OS_VER >= 2
static struct pesudo_task *pesudo_mq_receive_internal(struct pesudo_mq *mq)
{
    struct pesudo_task *task, *tmp, *find_task = NULL;

    if (mq->msg_count <= 0)
    {
        return NULL;
    }

#if PESUDO_OS_VER == 2
    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    if ((task->state & PT_BLOCKED_MQ) && (task->block_obj == mq))
#else
    TAILQ_FOREACH_SAFE(task, &mq->waiting_tasks, list4mq, tmp)
#endif
    {
#if PESUDO_OS_VER >= 3
        if ((task->state & PT_BLOCKED_MQ) == 0)
        {
            TAILQ_REMOVE(&mq->waiting_tasks, task, list4mq);
        }
        else
#endif
        {
            switch (mq->opt & PESUDO_OPT_MASK)
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
        find_task->state |= PT_RECV_MQ;
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&mq->waiting_tasks, find_task, list4mq);
#endif
    }

    return find_task;
}
#endif

int pesudo_mq_send(struct pesudo_mq *mq, const void *msg)
{
    if (!mq || !msg)
    {
        errno = EINVAL;
        return -1;
    }

    /*
     * If full, throw It
     */
    if (mq->msg_count == mq->item_count)
    {
        errno = ENOSPC;
        return -1;
    }

    /**
     * 写入消息缓冲区
     */
    pesudo_mq_write(mq, msg);

#if PESUDO_OS_VER == 1

    /**
     * 根据 OPT 给阻塞任务设置接收
     */
    struct pesudo_task *task, *tmp, *find_task = NULL;

    TAILQ_FOREACH_SAFE(task, &pesudo_task_list, list, tmp)
    {
        if ((task->state & PT_BLOCKED_MQ) && (task->block_obj == mq))
        {
            switch (mq->opt & PESUDO_OPT_MASK)
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
        find_task->state |= PT_RECV_MQ;
    }

#elif PESUDO_OS_VER >= 2

    /**
     * process mq from task immediately
     */
    if (!RunningInsideISR)
    {
        pesudo_mq_receive_internal(mq);
    }

#endif

    return 0;
}

int pesudo_mq_receive(struct pesudo_mq *mq, void *msg, uint32_t timeout_ms)
{
    int ret = 0;

    if (!mq || !msg)
    {
        errno = EINVAL;
        return -1;
    }

    if (mq->msg_count > 0)
    {
        return pesudo_mq_read(mq, msg);
    }

    if (timeout_ms == 0)
    {
        errno = ENODATA;
        return -1;
    }

    if (!current_pesudo_task)       /* outside PesudoOS? */
    {
        pesudoos_dbg(PESUDOOS_DBG_MQ, "pesudo_mq_receive() can use with PesudoOS only\r\n");
        errno = EIO;
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
        current_pesudo_task->state |= PT_BLOCKED_MQ;

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = mq;
#else
        TAILQ_INSERT_TAIL(&mq->waiting_tasks, current_pesudo_task, list4mq);
#endif

        longjmp(__mainloop_jmp_pos, -PT_BLOCKED_MQ);
    }
    else
    {
        /*
         * blocked back
         */
        if ((PT_RECV_MQ == ret) && (mq->msg_count > 0))
        {
            pesudo_mq_read(mq, msg);
        }

#if PESUDO_OS_VER <= 2
        current_pesudo_task->block_obj = NULL;
#endif
        current_pesudo_task->block_when = 0;
        current_pesudo_task->block_until = 0;
        current_pesudo_task->state &= ~(PT_BLOCKED_MQ | PT_RECV_MQ);
        current_pesudo_task->state |= PT_STATE_RUNNING;
    }

    if (ret < 0)
    {
#if PESUDO_OS_VER >= 3
        TAILQ_REMOVE(&mq->waiting_tasks, current_pesudo_task, list4mq);
#endif
        errno = ETIMEDOUT;
        return -1;
    }

    return mq->item_size;
}

int pesudo_mq_is_full(struct pesudo_mq *mq)
{
    if (mq)
    {
        return (mq->opt & MQ_OPT_FULL) ? 1 : 0;
    }
    else
    {
        return 1;
    }
}

int pesudo_mq_flush(struct pesudo_mq *mq)
{
    if (mq)
    {
        mq->msg_count = 0;
        mq->wr_index = mq->rd_index = 0;
    }

    return 0;
}

//-----------------------------------------------------------------------------

#if PESUDO_OS_VER >= 2

void process_pesudo_mq_from_isr(void)
{
    struct pesudo_mq *mq, *tmp1;

    TAILQ_FOREACH_SAFE(mq, &pesudo_mq_list, list, tmp1)
    {
        if (mq->msg_count > 0)
        {
            pesudo_mq_receive_internal(mq);
        }
    }
}

#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


