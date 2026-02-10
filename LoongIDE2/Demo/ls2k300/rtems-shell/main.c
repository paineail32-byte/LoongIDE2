/*
 * "Hello World", RTEMS 4.11.2 for LS2K500
 */

#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>

#include <bsp.h>

#include <rtems/shell.h>

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

void start_shell(void);

/*
 * main function
 */
rtems_task Init(rtems_task_argument ignored)
{
    printf( "\n*** HELLO WORLD TEST ***\n" );
    printf( "Hello World\n" );

    /*
     * Shell
     */
    start_shell();

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

/**
 * Shell
 */
#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
//#define CONFIGURE_SHELL_MOUNT_RFS
#define CONFIGURE_SHELL_MOUNT_MSDOS
//#define CONFIGURE_SHELL_MOUNT_NFS
//#define CONFIGURE_SHELL_COMMAND_IFCONFIG
//#define CONFIGURE_SHELL_COMMAND_ROUTE
//#define CONFIGURE_SHELL_COMMAND_PING
//#define CONFIGURE_SHELL_COMMAND_NETSTATS
#include <rtems/shellconfig.h>

/*
 * Posix
 */
#define CONFIGURE_MAXIMUM_POSIX_KEYS        10
#define CONFIGURE_MAXIMUM_POSIX_THREADS     10
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES     10

#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES 10
#define CONFIGURE_MAXIMUM_POSIX_QUEUED_SIGNALS      10
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES      10
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES          10
#define CONFIGURE_MAXIMUM_POSIX_BARRIERS            10
#define CONFIGURE_MAXIMUM_POSIX_SPINLOCKS           10
#define CONFIGURE_MAXIMUM_POSIX_RWLOCKS             10

/*
 * FATFS
 */
#define CONFIGURE_FILESYSTEM_DOSFS
#define CONFIGURE_MAXIMUM_DOSFS_MOUNTS              10      // seemly not use

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

