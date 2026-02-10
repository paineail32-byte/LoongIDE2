/*
 * "Hello World", RTEMS 4.11.2 for LS2K500
 */

#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>

#include <bsp.h>

#include "udp_tcp_test.h"

#if UDP_TSET || TCP_TSET
#include <rtems/rtems_bsdnet.h>
#include "ls2k_network_config.h"
#endif

//-----------------------------------------------------------------------------

/*
 * UARTx as console port
 */
#if defined(LS2K300)
unsigned int BSPPrintkPort = 0;
#elif defined(LS2K500)
unsigned int BSPPrintkPort = 2;
#elif defined(LS2K1000)
unsigned int BSPPrintkPort = 0;
#endif

//-----------------------------------------------------------------------------

#define DEMO_LED1       0
#define DEMO_LED2       0

//-----------------------------------------------------------------------------

#if DEMO_LED1

static rtems_id led1_taskid;

static rtems_task led1_task(rtems_task_argument arg)
{
    /*
     * Add led1 initialize code here.
     */
    printf("enter task led1\r\n");
    
    for ( ; ; )
    {
        /*
         * Add led1 task code here.
         */
        rtems_interval ticks = rtems_clock_get_ticks_since_boot();

        printf("LED1 ticks = %i\r\n", ticks);

        // ...

        /* abandon cpu time to run other task */
        rtems_task_wake_after(1000);                      // task sleep 10 ms
        //rtems_task_wake_after(RTEMS_YIELD_PROCESSOR); // task sleep until cpu idle
        //rtems_task_wake_when(When);                   // task sleep to time When

    }
}

int led1_create(void)
{
    rtems_status_code rc;

    /* create the task */
    rc = rtems_task_create(rtems_build_name('L', 'E', 'D', '1'),
                           100,         // TODO PRIORITY
                           (8*1024),    // TODO Stack Size
                           RTEMS_DEFAULT_MODES,
                           RTEMS_DEFAULT_ATTRIBUTES,
                           &led1_taskid);

    if (RTEMS_SUCCESSFUL == rc)
    {
        /* start the task */
        rc = rtems_task_start(led1_taskid, led1_task, 0);
        if (RTEMS_SUCCESSFUL != rc)
        {
            /* start fail, clear the task */
            rtems_task_delete(led1_taskid);
        }
        printf("start task led1 %s\r\n", /*rtems_status_text(rc)*/ "successful");
    }
    else
        printf("create task led1 %s\r\n", /*rtems_status_text(rc)*/ "fail");

    return rc;
}

#endif // #if DEMO_LED1

//-----------------------------------------------------------------------------

#if DEMO_LED2

static rtems_id led2_taskid;

static rtems_task led2_task(rtems_task_argument arg)
{
    /*
     * Add led2 initialize code here.
     */
    printf("enter task led2\r\n");

    for ( ; ; )
    {
        /*
         * Add led1 task code here.
         */
        rtems_interval ticks = rtems_clock_get_ticks_since_boot();

        printf("LED2 ticks = %i\r\n", ticks);

        // ...

        /* abandon cpu time to run other task */
        
        rtems_task_wake_after(300);                      // task sleep 10 ms
        
        //rtems_task_wake_after(RTEMS_YIELD_PROCESSOR); // task sleep until cpu idle
        //rtems_task_wake_when(When);                   // task sleep to time When

    }
}

int led2_create(void)
{
    rtems_status_code rc;

    /* create the task */
    rc = rtems_task_create(rtems_build_name('L', 'E', 'D', '2'),
                           99,          // TODO PRIORITY
                           (8*1024),    // TODO Stack Size
                           RTEMS_DEFAULT_MODES,
                           RTEMS_DEFAULT_ATTRIBUTES,
                           &led2_taskid);

    if (RTEMS_SUCCESSFUL == rc)
    {
        /* start the task */
        rc = rtems_task_start(led2_taskid, led2_task, 0);
        if (RTEMS_SUCCESSFUL != rc)
        {
            /* start fail, clear the task */
            rtems_task_delete(led2_taskid);
        }
        printf("start task led2 %s\r\n", /*rtems_status_text(rc)*/ "successful");
    }
    else
        printf("create task led2 %s\r\n", /*rtems_status_text(rc)*/ "fail");

    return rc;
}

#endif // #if DEMO_LED2

//-----------------------------------------------------------------------------

/*
 * main function
 */
rtems_task Init(rtems_task_argument ignored)
{
    printf( "\n*** HELLO WORLD TEST ***\n" );
    printf( "Hello World\n" );

#if DEMO_LED1
    led1_create();
#endif
#if DEMO_LED2
    led2_create();
#endif

	/*
	 * 初始化网络
	 */
#if UDP_TSET || TCP_TSET
	printf("Initializing Network\n");
	rtems_bsdnet_initialize_network();
#endif

#if UDP_TSET
    /*
     * 启动 UDP server 测试线程
     */
    udp_server_task_create();
#endif

#if TCP_TSET
    /*
     * 启动 TCP server 测试线程
     */
    tcp_server_task_create();
#endif

    printf( "*** END OF HELLO WORLD TEST ***\n" );

    rtems_task_delete(RTEMS_SELF);

    /*
     * never go here!
     */
    exit(0);
}

/*
 * Cut the OS here.
 */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS                     5
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS    5
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_MAXIMUM_DRIVERS                   5
#define CONFIGURE_MAXIMUM_SEMAPHORES                5

#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES            2

#define CONFIGURE_INIT_TASK_STACK_SIZE              (2 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_EXTRA_TASK_STACKS                 (2 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MINIMUM_TASK_STACK_SIZE           (4 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_MICROSECONDS_PER_TICK             1000    // system tick

#define CONFIGURE_INIT_TASK_PRIORITY                120
#define CONFIGURE_INIT_TASK_INITIAL_MODES          (RTEMS_PREEMPT | \
                                                    RTEMS_NO_TIMESLICE | \
                                                    RTEMS_NO_ASR | \
                                                    RTEMS_INTERRUPT_LEVEL(0))

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

