/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * osal_freertos.c
 *
 * created: 2025-01-17
 *  author: 
 */

#ifdef OS_FREERTOS

#include <malloc.h>
#include <string.h>

#include "osal.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

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
    TaskHandle_t htask = NULL;
    
    stack_size /= sizeof(StackType_t);
    xTaskCreate(entry,
                name,
                stack_size,
                args,
                configMAX_PRIORITIES - prio,
                &htask);

    if (htask == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TASK_FAIL "\r\n", name);
        while (1)
        {
            //
        }
    }
    
    return (osal_task_t)htask;
}

void osal_task_delete(osal_task_t task)
{
    vTaskDelete((TaskHandle_t)task);
}

void osal_task_suspend(osal_task_t task)
{
    vTaskSuspend((TaskHandle_t)task);
}

void osal_task_resume(osal_task_t task)
{
    vTaskResume((TaskHandle_t)task);
}

void osal_task_sleep(uint32_t ms)
{
    vTaskDelay((const TickType_t)ms);
}

void osal_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks)
{
    if (prev_ticks)
        vTaskDelayUntil((TickType_t* const)prev_ticks, (const TickType_t)inc_ticks);
    else
        vTaskDelay((const TickType_t)inc_ticks);
}

//-----------------------------------------------------------------------------
// Event
//-----------------------------------------------------------------------------

osal_event_t osal_event_create(const char *name, uint32_t opt)
{
    osal_event_t event = (osal_event_t)xEventGroupCreate();
    if (event == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_EVENT_FAIL "\r\n");
        while (1)
        {
            //
        }
    }
    
    return event;
}

void osal_event_delete(osal_event_t event)
{
    vEventGroupDelete((EventGroupHandle_t)event);
}

int osal_event_send(osal_event_t event, uint32_t bits)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
    
    if (xPortIsInsideInterrupt())
    {
		xResult = xEventGroupSetBitsFromISR((EventGroupHandle_t)event,
                                            (const EventBits_t)bits,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xResult = (xEventGroupSetBits((EventGroupHandle_t)event,
                                      (const EventBits_t)bits) == bits) ? pdPASS : pdFAIL;
    }
    
    return (xResult == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
}

uint32_t osal_event_receive(osal_event_t event, uint32_t bits,
                            uint32_t flag, uint32_t timeout_ms)
{

    EventBits_t recv_event;
    recv_event = xEventGroupWaitBits((EventGroupHandle_t)event,
                        (const EventBits_t)bits,
                        (flag & OSAL_EVENT_FLAG_CLEAR) ? pdTRUE : pdFALSE,
                        (flag & OSAL_EVENT_FLAG_OR   ) ? pdTRUE : pdFALSE,
                        (timeout_ms != OSAL_WAIT_FOREVER) ? timeout_ms : portMAX_DELAY);

    return (uint32_t)recv_event;
}

void osal_event_set_bits(osal_event_t event, uint32_t bits)
{
    xEventGroupSetBits((EventGroupHandle_t)event, (const EventBits_t)bits);
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
    osal_sem_t sem = (osal_sem_t)xSemaphoreCreateCounting(1, initial_count);
    if (sem == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_SEM_FAIL "\r\n");
        while (1)
        {
            //
        }
    }
    
    return sem;
}

void osal_sem_delete(osal_sem_t sem)
{
    vSemaphoreDelete((QueueHandle_t)sem);
}

int osal_sem_obtain(osal_sem_t sem, uint32_t timeout)
{
    BaseType_t xResult;

    xResult = xSemaphoreTake((QueueHandle_t)sem,
                             (timeout != OSAL_WAIT_FOREVER) ? timeout : portMAX_DELAY);
    
    return (xResult == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_sem_release(osal_sem_t sem)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;

    if (xPortIsInsideInterrupt())
    {
        xResult = xSemaphoreGiveFromISR((QueueHandle_t)sem, &xHigherPriorityTaskWoken);
        if (xResult == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xResult = xSemaphoreGive((QueueHandle_t)sem);
    }

    return (xResult == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
}

void osal_sem_reset(osal_sem_t sem)
{
    xQueueReset((QueueHandle_t)sem);
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
    osal_mutex_t mutex = (osal_mutex_t)xSemaphoreCreateMutex();
    if (mutex == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_MUTEX_FAIL "\r\n");
        while (1)
        {
            //
        }
    }

    return mutex;
}

void osal_mutex_delete(osal_mutex_t mutex)
{
    vSemaphoreDelete((QueueHandle_t)mutex);
}

int osal_mutex_obtain(osal_mutex_t mutex, uint32_t timeout_ms)
{
    BaseType_t xResult;

    xResult = xSemaphoreTake((QueueHandle_t)mutex,
                             (timeout_ms != OSAL_WAIT_FOREVER) ? timeout_ms : portMAX_DELAY);

    return (xResult == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_mutex_release(osal_mutex_t mutex)
{
    return (xSemaphoreGive((QueueHandle_t)mutex) == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
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
    osal_mq_t mq = (osal_mq_t)xQueueCreate(max_msgs, item_size);
    if (mq == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_MQ_FAIL "\r\n");
        while (1)
        {
            //
        }
    }
    
    return mq;
}

void osal_mq_delete(osal_mq_t mq)
{
    vQueueDelete((QueueHandle_t)mq);
}

int osal_mq_send(osal_mq_t mq, const void *msg, int size)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;

    debug_mq("mq send: 0x%" PRIx64 "\r\n", *(uint64_t *)msg);

    if (xPortIsInsideInterrupt())
    {
        xResult = xQueueSendFromISR((QueueHandle_t)mq, msg, &xHigherPriorityTaskWoken);
        if (xResult == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xResult = xQueueSendToBack((QueueHandle_t)mq, msg, portMAX_DELAY);
    }

    return (xResult == pdPASS) ? 0 : -OSAL_ERR_TIMEOUT;
}

int osal_mq_receive(osal_mq_t mq, void *msg, int size, uint32_t timeout)
{
    BaseType_t xResult;

    xResult = xQueueReceive((QueueHandle_t)mq, msg,
                            (timeout != OSAL_WAIT_FOREVER) ? timeout : portMAX_DELAY);

    if (xResult == pdPASS)
    {
    	debug_mq("mq recv: 0x%" PRIx64 "\r\n", *(uint64_t *)msg);
    	return size;
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
    void *argument;
    bool is_period;
    uint32_t ticks;
    void *timer;
};

static void __fr_timeout(TimerHandle_t *handle)
{
    struct osal_timer *tmr = (struct osal_timer *)pvTimerGetTimerID((TimerHandle_t)handle);
    tmr->handler(tmr->argument);
}

osal_timer_t osal_timer_create(const char *name,
                               osal_task_entry_t handler,
                               void *argument,
                               uint32_t timeout_ms,
                               bool is_period)
{
    struct osal_timer *tmr;

    tmr = pvPortMalloc(sizeof(struct osal_timer));
    if (tmr == NULL)
    {
        LOG_ERR(STR_OSAL_CREATE_TIMER_FAIL "\r\n");
        while (1)
        {
            //
        }
    }
    memset(tmr, 0, sizeof(struct osal_timer));

    tmr->handler = handler;
    tmr->argument = argument;

    tmr->timer = (void *)xTimerCreate(name,
                                      pdMS_TO_TICKS(timeout_ms),
                                      is_period,
                                      tmr,
                                      (TimerCallbackFunction_t)__fr_timeout);
    if (tmr->timer == NULL)
    {
        LOG_ERR("Create timer failed\r\n");
        while (1)
        {
            //
        }
    }

    return (osal_timer_t)tmr;
}

void osal_timer_delete(osal_timer_t timer)
{
    struct osal_timer *tmr = (struct osal_timer *)timer;
    xTimerStop(tmr->timer, 0);
    xTimerDelete(tmr->timer, 0);
    vPortFree(tmr);
}

void osal_timer_start(osal_timer_t timer, uint32_t timeout_ms)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
    struct osal_timer *tmr = (struct osal_timer *)timer;
    
    if (xPortIsInsideInterrupt())
    {
        xResult = xTimerStartFromISR(tmr->timer, &xHigherPriorityTaskWoken);
        if (xResult == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xTimerStart(tmr->timer, 0);
    }
}

void osal_timer_stop(osal_timer_t timer)
{
    struct osal_timer *tmr = (struct osal_timer *)timer;
    xTimerStop(tmr->timer, 0);
}

//-----------------------------------------------------------------------------

size_t osal_enter_critical_section(void)
{
    UBaseType_t xResult;

    if (xPortIsInsideInterrupt())
    {
        xResult = taskENTER_CRITICAL_FROM_ISR();
    }
    else
    {
        taskENTER_CRITICAL();
        xResult = 1;
    }

    return xResult;
}

void osal_leave_critical_section(size_t flag)
{
    if (xPortIsInsideInterrupt())
    {
        taskEXIT_CRITICAL_FROM_ISR(flag);
    }
    else
    {
        taskEXIT_CRITICAL();
    }
}

//-----------------------------------------------------------------------------

void osal_msleep(uint32_t ms)
{
	if (ms > 0)
	{
		if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
			vTaskDelay(pdMS_TO_TICKS(ms));
		else
			delay_ms(ms);
	}
}

//-----------------------------------------------------------------------------

void *osal_malloc(size_t size)
{
    return pvPortMalloc(size);
}

void osal_free(void *ptr)
{
    vPortFree(ptr);
}

//-----------------------------------------------------------------------------

int osal_is_osrunning(void)
{
    return (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) ? 1 : 0;
}

#endif // #ifdef OS_FREERTOS

//-----------------------------------------------------------------------------

/*
 * @@END
 */

