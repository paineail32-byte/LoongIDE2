
/* RT-Thread Kernel */

#define ARCH_CPU_64BIT
#define RT_ALIGN_SIZE                    8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX           32
#define RT_TICK_PER_SECOND               1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_HOOK_USING_FUNC_PTR
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE           4
#define IDLE_THREAD_STACK_SIZE           8192   // 1024 * sizeof(void *)
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO             4
#define RT_TIMER_THREAD_STACK_SIZE       8192   // 1024 * sizeof(void *)
#define RT_NAME_MAX                      16

/* kservice optimization */


/* Enable debugging features */

#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL

/* Using memheap Memory Algorithm */

#define RT_USING_MEMHEAP
#define RT_MEMHEAP_FAST_MODE
#define RT_USING_MEMHEAP_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE               512
#define RT_CONSOLE_DEVICE_NAME           "uart6"
#define RT_VER_NUM                       0x40100

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE        8192
#define RT_MAIN_THREAD_PRIORITY          10

/* MSH: command shell */


/* DFS: device virtual file system */


/* elm-chan's FatFs, Generic FAT Filesystem Module */


/* FAL: flash abstraction layer */


/* Device Drivers */


/* USING Serial device drivers */

// #define RT_USING_SERIAL


/* Using Hardware Crypto drivers */


/* Using Wi-Fi framework */


/* Using USB */


/* Network */


/* SAL: socket abstraction layer */


/* Enable network interface device */


/* LwIP: light weight TCP/IP stack */


/* Static IPv4 Address */


/* Enable AT commands */


/* Utilities */


/* log format */


/* RT-Link */


/* rt link debug option */


/* VBus: virtual software bus */


/*
 * @@END
 */

