/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * osal_ucos.c
 *
 * created: 2025-01-17
 *  author: 
 */

#ifdef OS_UCOS

#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "osal.h"

#include "os.h"

#if 0
#include <inttypes.h>
extern void printk(const char *fmt, ...);
#define debug_mq(...)   printk(__VA_ARGS__)
#else
#define debug_mq(...)
#endif

/******************************************************************************
 * uCOS message queue 存储的是指针, 当 osal 需要读写实体数据时, 必须符合:
 *
 * 1. 收发的 message 大小相等
 * 2. 收发的 message 大小不等于 sizeof(uintptr_t)
 * 3. 接收到的 message 是一个指针
 *
 * 这时, osal_mq_receive() 将向参数 void *msg 复制数据,
 * 否则, 仅把接收到的指针赋值给参数 void *msg.
 *
 ******************************************************************************/

#define MQ_TRY_COPY     1

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
    OS_ERR   ucErr;
    OS_TCB  *p_tcb;;
    CPU_STK *p_stk;
    int      namelen;
    char    *__name;

    p_tcb = (OS_TCB *)calloc(sizeof(OS_TCB), 1);
    if (!p_tcb)
    {
        errno = ENOMEM;
        return NULL;
    }
    
    p_stk = (CPU_STK *)calloc(1, stack_size);
    if (!p_stk)
    {
        free(p_tcb);
        errno = ENOMEM;
        return NULL;
    }

    stack_size /= sizeof(CPU_STK);

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSTaskCreate(p_tcb,
                 (CPU_CHAR *)name,
                 entry,
                 args,
                 prio,                              // TaskPrio,
                 (void *)p_stk,                     // TaskStkBasePtr,
                 stack_size*10/100,                 // TaskStkLimit,
                 stack_size,                        // TaskStkSize,
                 0u,
                 0u,
                 NULL,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_NO_TLS),
                 &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_TASK_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }

    return (osal_task_t)p_tcb;
}

void osal_task_delete(osal_task_t task)
{
    OS_ERR   ucErr;
    OS_TCB  *p_tcb = (OS_TCB *)task;
    CPU_STK *p_stk = p_tcb->StkBasePtr;
    char *p_name = p_tcb->NamePtr;

    OSTaskDel(p_tcb, &ucErr);

    if (OS_ERR_NONE == ucErr)
    {
        if (p_name)
            free(p_name);
        free(p_tcb);
        free(p_stk);
    }
}

void osal_task_suspend(osal_task_t task)
{
    OS_ERR ucErr;
    
    OSTaskSuspend((OS_TCB *)task, &ucErr);
}

void osal_task_resume(osal_task_t task)
{
    OS_ERR ucErr;
    
    OSTaskResume((OS_TCB *)task, &ucErr);
}

void osal_task_sleep(uint32_t ms)
{
    OS_ERR ucErr;

    OSTimeDly(ms, OS_OPT_TIME_DLY, &ucErr);
}

void osal_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks)
{
    OS_ERR ucErr;

    if (prev_ticks)
    {
        OSTimeDly(*prev_ticks + inc_ticks, OS_OPT_TIME_MATCH, &ucErr);
        *prev_ticks += inc_ticks;
    }
    else
    {
        OSTimeDly(inc_ticks, OS_OPT_TIME_DLY, &ucErr);
    }
}

//-----------------------------------------------------------------------------
// Event
//-----------------------------------------------------------------------------

osal_event_t osal_event_create(const char *name, uint32_t opt)
{
    OS_ERR ucErr;
    OS_FLAG_GRP *p_grp;
    int    namelen;
    char  *__name;

    p_grp = (OS_FLAG_GRP *)calloc(sizeof(OS_TCB), 1);
    if (!p_grp)
    {
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSFlagCreate(p_grp, (CPU_CHAR *)name, 0, &ucErr);   /* 0: init value */

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_EVENT_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }
    
    return (osal_event_t)p_grp;
}

void osal_event_delete(osal_event_t event)
{
    OS_ERR ucErr;
    OS_FLAG_GRP *p_event = (OS_FLAG_GRP *)event;
    char *p_name = p_event->NamePtr;

    OSFlagDel(p_event, OS_OPT_DEL_ALWAYS, &ucErr);
    if (OS_ERR_NONE == ucErr)
    {
        if (p_name)
            free(p_name);
        free(event);
    }
}

int osal_event_send(osal_event_t event, uint32_t bits)
{
    OS_ERR ucErr;
    
    OSFlagPost((OS_FLAG_GRP *)event,
	           (OS_FLAGS)bits,
	           OS_OPT_POST_FLAG_SET,
	           &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
}

uint32_t osal_event_receive(osal_event_t event, uint32_t bits,
                            uint32_t flag, uint32_t timeout_ms)
{
    OS_ERR ucErr;
    uint32_t os_flag = 0;
    OS_FLAGS recv_event;

    if (flag & OSAL_EVENT_FLAG_AND)   os_flag |= OS_OPT_PEND_FLAG_SET_ALL;
    if (flag & OSAL_EVENT_FLAG_OR)    os_flag |= OS_OPT_PEND_FLAG_SET_ANY;
    if (flag & OSAL_EVENT_FLAG_CLEAR) os_flag |= OS_OPT_PEND_FLAG_CONSUME;

    recv_event = OSFlagPend((OS_FLAG_GRP *)event,
	                        (OS_FLAGS)bits,
	                        (timeout_ms == OSAL_WAIT_FOREVER) ? 0 : timeout_ms,
	                        (OS_OPT)os_flag,
	                        NULL,                  /* 返回等待的时间 */
	                        &ucErr);

    return (OS_ERR_NONE == ucErr) ? recv_event : 0;
}

void osal_event_set_bits(osal_event_t event, uint32_t bits)
{
    ((OS_FLAG_GRP *)event)->Flags = bits;
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
    OS_ERR ucErr;
    int    namelen;
    char  *__name;

    OS_SEM *p_sem = (OS_SEM *)calloc(sizeof(OS_SEM), 1);
    if (!p_sem)
    {
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSSemCreate(p_sem, (CPU_CHAR *)name, initial_count, &ucErr);
    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_SEM_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_sem_t)p_sem;
}

void osal_sem_delete(osal_sem_t sem)
{
    OS_ERR  ucErr;
    OS_SEM *p_sem = (OS_SEM *)sem;
    char *p_name = p_sem->NamePtr;
    
    OSSemDel(p_sem, OS_OPT_DEL_ALWAYS, &ucErr);

    if (OS_ERR_NONE == ucErr)
    {
        if (p_name)
            free(p_name);
        free(sem);
    }
}

int osal_sem_obtain(osal_sem_t sem, uint32_t timeout)
{
    OS_ERR ucErr;

    OSSemPend((OS_SEM *)sem,
              (timeout == OSAL_WAIT_FOREVER) ? 0 : timeout,
              OS_OPT_PEND_BLOCKING,
              NULL,
              &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_sem_release(osal_sem_t sem)
{
    OS_ERR ucErr;

    OSSemPost((OS_SEM *)sem,
              OS_OPT_POST_1, // | OS_OPT_POST_NO_SCHED, // OS_OPT_POST_ALL, //
              &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
}

void osal_sem_reset(osal_sem_t sem)
{
    OS_SemClr((OS_SEM *)sem);
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
    OS_ERR ucErr;
    int    namelen;
    char  *__name;

    OS_MUTEX *p_mutex = (OS_MUTEX *)calloc(sizeof(OS_MUTEX), 1);
    if (!p_mutex)
    {
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSMutexCreate(p_mutex, (CPU_CHAR *)name, &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_MUTEX_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_mutex_t)p_mutex;
}

void osal_mutex_delete(osal_mutex_t mutex)
{
    OS_ERR ucErr;
    OS_MUTEX *p_mutex = (OS_MUTEX *)mutex;
    char *p_name = p_mutex->NamePtr;

    OSMutexDel(p_mutex, OS_OPT_DEL_ALWAYS, &ucErr);

    if (OS_ERR_NONE == ucErr)
    {
        if (p_name)
            free(p_name);
        free(mutex);
    }
}

int osal_mutex_obtain(osal_mutex_t mutex, uint32_t timeout_ms)
{
    OS_ERR ucErr;

    OSMutexPend((OS_MUTEX *)mutex,
                (timeout_ms == OSAL_WAIT_FOREVER) ? 0 : timeout_ms,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_mutex_release(osal_mutex_t mutex)
{
    OS_ERR ucErr;

    OSMutexPost((OS_MUTEX *)mutex,
                OS_OPT_POST_NONE,
                &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
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
    OS_ERR ucErr;
    int    namelen;
    char  *__name;

    OS_Q *p_q = (OS_Q *)calloc(sizeof(OS_Q), 1);
    if (!p_q)
    {
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSQCreate(p_q, (CPU_CHAR *)name, max_msgs, &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_MQ_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_mq_t)p_q;
}

void osal_mq_delete(osal_mq_t mq)
{
    OS_ERR ucErr;
    OS_Q  *p_mq = (OS_Q *)mq;
    char *p_name = p_mq->NamePtr;

    OSQFlush(p_mq, &ucErr);

    OSQDel(p_mq, OS_OPT_DEL_ALWAYS, &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        if (p_name)
            free(p_name);
        free(mq);
    }
}

int osal_mq_send(osal_mq_t mq, const void *msg, int size)
{
    OS_ERR ucErr;

    debug_mq("mq send: 0x%" PRIx64 "\r\n", (uint64_t)msg);

    OSQPost((OS_Q *)mq,
            (void *)msg,
            size,            
            OS_OPT_POST_FIFO,       // OS_OPT_POST_ALL, //
            &ucErr);

    return (OS_ERR_NONE == ucErr) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_mq_receive(osal_mq_t mq, void *msg, int size, uint32_t timeout)
{
    OS_ERR ucErr;
    OS_MSG_SIZE msg_size = 0;
    void *ret;

    ret = OSQPend((OS_Q *)mq,
                  (timeout == OSAL_WAIT_FOREVER) ? 0 : timeout,
                  OS_OPT_PEND_BLOCKING,
                  &msg_size,
                  NULL,
                  &ucErr);

    if (OS_ERR_NONE == ucErr)
    {
#if MQ_TRY_COPY
        /**
         * uCOS 使用的是指针, 尝试复制内容
         */
    	if ((msg_size == size) && (size > 0) && (size != sizeof(uintptr_t))
	#ifdef __loongarch64
            && (((uint64_t)ret >> 32) == 0x90000000)    /* Received must be a pointer */
	#endif
            )
        {
            memcpy(msg, ret, size);     
        }
        else
#endif
        {
		#if 0
			/*
			 * 有对齐问题
			 */
			*(uintptr_t *)msg = (uintptr_t)ret;		/* 指针 */

		#else
			unsigned char *pch_msg = (unsigned char *)msg;
			unsigned char *pch_ret = (unsigned char *)&ret;

			for (int i=0; i<size; i++)
			{
				pch_msg[i] = pch_ret[i];
			}

		#endif
        }

        debug_mq("mq recv: 0x%" PRIx64 "\r\n", (uint64_t)ret);

        return msg_size;
    }

    return -OSAL_ERR_TIMEOUT;
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

struct osal_timer
{
    osal_task_entry_t handler;
    void   *argument;
    OS_TMR *uc_timer;
};

static void __uc_timeout(void *uc_timer, void *p_arg)
{
    struct osal_timer *tmr = (struct osal_timer *)p_arg;
    tmr->handler(tmr->argument);
    (void)uc_timer;
}

/*
 * 注意参数: OS_CFG_TMR_TASK_RATE_HZ "os_cfg_app.h"
 */
osal_timer_t osal_timer_create(const char *name,
                               osal_task_entry_t handler,
                               void *argument,
                               uint32_t timeout_ms,
                               bool is_period)
{
    OS_ERR ucErr;
    int    namelen;
    char  *__name;
    struct osal_timer *p_tmr;

    p_tmr = (struct osal_timer *)calloc(sizeof(struct osal_timer), 1);
    if (p_tmr == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TIMER_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    p_tmr->handler  = handler;
    p_tmr->argument = argument;

    p_tmr->uc_timer = (OS_TMR *)calloc(sizeof(OS_TMR), 1);
    if (!p_tmr->uc_timer)
    {
        free(p_tmr);
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name);
    if (namelen > 0)
    {
        __name = (char *)malloc(namelen + 1);
        memcpy(__name, name, namelen);
        __name[namelen] = '\0';
        name = __name;
    }

    OSTmrCreate(p_tmr->uc_timer,
                (CPU_CHAR *)name,
                is_period ? 0 : timeout_ms,
                is_period ? timeout_ms : 0,
                is_period ? OS_OPT_TMR_PERIODIC : OS_OPT_TMR_ONE_SHOT,
                (OS_TMR_CALLBACK_PTR)__uc_timeout,  // handler,
                (void *)p_tmr,                      // argument,
                &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR(STR_OSAL_CREATE_TIMER_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_timer_t )p_tmr;
}

void osal_timer_delete(osal_timer_t timer)
{
    OS_ERR  ucErr;
    struct osal_timer *p_tmr = (struct osal_timer *)timer;
    char *p_name = p_tmr->uc_timer->NamePtr;

    OSTmrDel(p_tmr->uc_timer, &ucErr);

    if (OS_ERR_NONE == ucErr)
    {
        if (p_name)
            free(p_name);
        free(p_tmr->uc_timer);
        free(p_tmr);
    }
}

void osal_timer_start(osal_timer_t timer, uint32_t timeout_ms)
{
    OS_ERR ucErr;
    struct osal_timer *p_tmr = (struct osal_timer *)timer;

    OSTmrStart(p_tmr->uc_timer, &ucErr);

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR("Start timer failed\r\n");
    }
}

void osal_timer_stop(osal_timer_t timer)
{
    /*
     * OS_OPT_TMR_ONE_SHOT Timer 超时后自动删除
     */

    OS_ERR ucErr;
    struct osal_timer *p_tmr = (struct osal_timer *)timer;

    OSTmrStop(p_tmr->uc_timer, OS_OPT_TMR_NONE, NULL, &ucErr);  // OS_OPT_TMR_CALLBACK_ARG

    if (OS_ERR_NONE != ucErr)
    {
        LOG_ERR("Stop timer failed\r\n");
    }
}

//-----------------------------------------------------------------------------

size_t osal_enter_critical_section(void)
{
    CPU_SR cpu_sr;
    CPU_CRITICAL_ENTER();

    return (size_t)cpu_sr;
}

void osal_leave_critical_section(size_t flag)
{
    CPU_SR cpu_sr = (CPU_SR)flag;
    CPU_CRITICAL_EXIT();
}

//-----------------------------------------------------------------------------

void osal_msleep(uint32_t ms)
{
	if (ms > 0)
	{
		if (OSRunning == OS_STATE_OS_RUNNING)
		{
			OS_ERR ucErr;
			OSTimeDly(ms, OS_OPT_TIME_DLY, &ucErr);
		}
		else
		{
			delay_ms(ms);
		}
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
    return (OSRunning == OS_STATE_OS_RUNNING) ? 1 : 0;
}

#endif // #ifdef OS_UCOS

//-----------------------------------------------------------------------------

/*
 * @@END
 */

