/*
*********************************************************************************************************
*                                              uC/OS-III
*                                        The Real-Time Kernel
*
*                    Copyright 2009-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           LOONGSON 2K300
*
* File    : os_cpu_c.c
* Version : V3.08.01
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS

#include <string.h>

#include <larchintrin.h>
#include "cpu.h"

#include "os.h"

#ifdef __cplusplus
extern  "C" {
#endif

//--------------------------------------------------------------------------------------------------

volatile CPU_INT32U  OSIntCtxSwFlag = 0;        /* Used to flag a context switch  */

//--------------------------------------------------------------------------------------------------

/*
************************************************************************************************************************
*                                                OS INITIALIZATION HOOK
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSInitHook (void)
{
	//
}


/*
************************************************************************************************************************
*                                                  TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if (defined(OS_CFG_TRACE_EN) && (OS_CFG_TRACE_EN > 0u))
    /*
     * Unique ID for third-party debuggers and tracers.
     */
    static CPU_INT16U   v_task_id = 1;

    p_tcb->TaskID = v_task_id;
    v_task_id++;

#endif

#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskCreateHookPtr != (OS_APP_HOOK_TCB)0)
    {
        (*OS_AppTaskCreateHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning */
#endif
}


/*
************************************************************************************************************************
*                                                   TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskDelHookPtr != (OS_APP_HOOK_TCB)0)
    {
        (*OS_AppTaskDelHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;                                            /* Prevent compiler warning */
#endif
}


/*
************************************************************************************************************************
*                                                   IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do such things as
*              STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSIdleTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppIdleTaskHookPtr != (OS_APP_HOOK_VOID)0)
    {
        (*OS_AppIdleTaskHookPtr)();
    }
#endif
}

/*
************************************************************************************************************************
*                                                 STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-III's statistics task.  This allows your application to add
*              functionality to the statistics task.
*
* Arguments  : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSStatTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppStatTaskHookPtr != (OS_APP_HOOK_VOID)0)
    {
        (*OS_AppStatTaskHookPtr)();
    }
#endif
}

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is processor-specific.
*
* Arguments  : task     is a pointer to the task code.
*
*              p_arg    is a pointer to a user supplied data area
*
*              ptos     is a pointer to the top of stack.  OSTaskStkInit() assumes that 'ptos' points to
*                       a free entry on the stack.  If OS_STK_GROWTH is set to 1 then 'ptos' will contain
*                       the HIGHEST valid address of the stack.  Similarly, if OS_STK_GROWTH is set to 0,
*                       'ptos' will contain the lowest valid address of the stack.
*
*              opt      specifies options that can be used to alter the behavior of OSTaskStkInit()
*                       (see ucos_ii.h for OS_TASK_OPT_???).
*
* Returns    : The location corresponding to the top of the stack
*
* Note(s)    : 1) Interrupts are enabled when each task starts executing.
*
*              2) An initialized stack has the structure shown below.
*
*********************************************************************************************************
*/

#include "context.h"

register CPU_INT64U $GP __asm__ ("$r2");

CPU_STK  *OSTaskStkInit (OS_TASK_PTR    p_task,
                         void          *p_arg,
                         CPU_STK       *p_stk_base,
                         CPU_STK       *p_stk_limit,
                         CPU_STK_SIZE   stk_size,
                         OS_OPT         opt)
{
    static CPU_INT64U crmd=0, ecfg=0, __gp=0;
    CPU_INT64U  *stk;

    (void)opt;                          	/* Prevent compiler warning for unused arguments */

    if (crmd == 0)
    {
        crmd  = __csrrd_d(LA_CSR_CRMD);
        crmd |= CSR_CRMD_WE | CSR_CRMD_IE;
        ecfg  = __csrrd_d(LA_CSR_ECFG);
        __gp   = $GP;
    }

    stk = &p_stk_base[stk_size - 1u];
    stk -= CTX_SIZE / 8;               	    /* sizeof(CPU_INT64U) */

    memset((void *)stk, 0, CTX_SIZE);       /* Clear the stack */

/*
    for (int i=0; i < 32; i++)
    {
        stk[i] = ((CPU_INT64U)(i & 0xFF) << 56) |
                 ((CPU_INT64U)(i & 0xFF) << 48) |
                 ((CPU_INT64U)(i & 0xFF) << 40) |
                 ((CPU_INT64U)(i & 0xFF) << 32) |
                 ((CPU_INT64U)(i & 0xFF) << 24) |
                 ((CPU_INT64U)(i & 0xFF) << 16) |
                 ((CPU_INT64U)(i & 0xFF) << 8 ) |
                 ((CPU_INT64U)(i & 0xFF) << 0 );
    }
 */

    *(stk + R_SP   ) = (CPU_INT64U)stk;
    *(stk + R_A0   ) = (CPU_INT64U)p_arg;       /* Parameters to pass in. */
    *(stk + R_RA   ) = (CPU_INT64U)OS_TaskReturn;

    *(stk + R_ESTAT) = 0;
    *(stk + R_CRMD)  = crmd;
    *(stk + R_GP  )  = __gp;
    *(stk + R_ECFG)  = ecfg;

    *(stk + R_PRMD)  = CSR_PRMD_PWE | CSR_PRMD_PIE;
    *(stk + R_EPC )  = (CPU_INT64U)p_task;

    return ((CPU_STK *)stk);                /* Return new top of stack */
}

/*
************************************************************************************************************************
*                                                   TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other operations
*              during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdyPtr' points to the TCB of the task that will be
*                 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points to the task being switched out
*                 (i.e. the preempted task).
************************************************************************************************************************
*/

void  OSTaskSwHook (void)
{
#if OS_CFG_TASK_PROFILE_EN > 0u
    CPU_TS  ts;
#endif
#ifdef  CPU_CFG_INT_DIS_MEAS_EN
    CPU_TS  int_dis_time;
#endif

#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskSwHookPtr != (OS_APP_HOOK_VOID)0)
    {
        (*OS_AppTaskSwHookPtr)();
    }
#endif

#if OS_CFG_TASK_PROFILE_EN > 0u
    ts = OS_TS_GET();
    if (OSTCBCurPtr != OSTCBHighRdyPtr)
    {
        OSTCBCurPtr->CyclesDelta  = ts - OSTCBCurPtr->CyclesStart;
        OSTCBCurPtr->CyclesTotal += (OS_CYCLES)OSTCBCurPtr->CyclesDelta;
    }

    OSTCBHighRdyPtr->CyclesStart = ts;
#endif

#ifdef  CPU_CFG_INT_DIS_MEAS_EN
    int_dis_time = CPU_IntDisMeasMaxCurReset();             /* Keep track of per-task interrupt disable time */
    if (OSTCBCurPtr->IntDisTimeMax < int_dis_time) {
        OSTCBCurPtr->IntDisTimeMax = int_dis_time;
    }
#endif

#if OS_CFG_SCHED_LOCK_TIME_MEAS_EN > 0u
    if (OSTCBCurPtr->SchedLockTimeMax < OSSchedLockTimeMaxCur)   /* Keep track of per-task scheduler lock time */
    {
        OSTCBCurPtr->SchedLockTimeMax = OSSchedLockTimeMaxCur;
        OSSchedLockTimeMaxCur         = (CPU_TS)0;               /* Reset the per-task value */
    }
#endif
}

/*
************************************************************************************************************************
*                                                      TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) This function is assumed to be called from the Tick ISR.
*
************************************************************************************************************************
*/

void  OSTimeTickHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTimeTickHookPtr != (OS_APP_HOOK_VOID)0)
    {
        (*OS_AppTimeTickHookPtr)();
    }
#endif
}

/*
************************************************************************************************************************
*                                                   TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should either be an
*              infinite loop or delete itself when done.
*
* Arguments  : p_tcb   is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u
    if (OS_AppTaskReturnHookPtr != (OS_APP_HOOK_TCB)0)
    {
        (*OS_AppTaskReturnHookPtr)(p_tcb);
    }
#else
    (void)p_tcb;
#endif
}

/*
 * @@ END
 */
 

