/*
 * start_shell.c
 *
 * created: 2023-10-21
 *  author:
 */

#if 1
#include <rtems/shell.h>

void start_shell(void)
{
    printf(" =========================\n");
    printf(" starting shell\n");
    printf(" =========================\n");
    rtems_shell_init(
        "SHLL",                             /* task name */
        RTEMS_MINIMUM_STACK_SIZE * 4,       /* task stack size */
        100,                                /* task priority */
        "/dev/console",  // "/dev/tty6",    /* device name */
        false,                              /* run forever */
        true,                               /* wait for shell to terminate */
        NULL // rtems_shell_login_check     /* login check function, use NULL to disable a login check */
    );
}

#endif

