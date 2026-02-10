/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#include "bsp.h"

#if !BSP_USE_FS

/*
 * This is as part of newlib glue, when using rtems gcc.
 */

#ifdef OS_RTTHREAD

//-------------------------------------------------------------------------------------------------
// RTThread
//-------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include "rtthread.h"

void *malloc(size_t size)
{
    return rt_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
    return rt_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
    return rt_realloc(ptr, size);
}

void free(void *ptr)
{
    rt_free(ptr);
}

void *aligned_malloc(size_t size, unsigned int align)
{
    return rt_malloc_align((rt_size_t)size, (rt_size_t)align);
}

void aligned_free(void *addr)
{
    rt_free_align(addr);
}

#else

//-------------------------------------------------------------------------------------------------
// BareMetal/uCOSIII/FreeRTOS
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

#include "osal.h"

//-------------------------------------------------------------------------------------------------

#ifndef align_up
#define align_up(num, align) (((num) + ((align)-1)) & ~((align)-1))
#endif

/**
 * memory block node struct
 */
typedef struct blk_node
{
    unsigned int flag;                        /* =1: used; =0: blank */
	size_t       size;
	void        *block;
    TAILQ_ENTRY(blk_node) link;         /* doubly linked queue */
} blk_node_t;

#define ALLOC_FLAG          0xdeadbeaf

#define BLOCK_HEADER_SZ     sizeof(blk_node_t)

#define MIN_ALLOC_SZ        (BLOCK_HEADER_SZ + 32)

//-------------------------------------------------------------------------------------------------

void malloc_lock();
void malloc_unlock();

//-------------------------------------------------------------------------------------------------

/**
 * free memory queue head
 */
static TAILQ_HEAD(blk_nodes_q, blk_node) free_list_head;

//-------------------------------------------------------------------------------------------------
// debug view queue
//-------------------------------------------------------------------------------------------------

#define MALLOC_DEBUG    0

#if MALLOC_DEBUG

static void debug_dump_list(void)
{
    int i = 0;
    blk_node_t *block, *tmp;
    
    TAILQ_FOREACH_SAFE(block, &free_list_head, link, tmp)
    {
        printk("\r\nnode[%i] @0x%016lx\r\n", i++, (long)block);
        printk("  flag  = 0x%08x\r\n", block->flag);
        printk("  size  = 0x%08x\r\n", block->size);
        printk("  block = 0x%016lx\r\n", (long)block->block);
        printk("  _next = 0x%016lx\r\n", (long)block->link.tqe_next);
	    printk("  _prev = 0x%016lx\r\n", (long)block->link.tqe_prev);
    }
}

#else

#define debug_dump_list()

#endif

//-------------------------------------------------------------------------------------------------

void *malloc(size_t size)
{
	void *ptr = NULL;
	blk_node_t *found_block = NULL, *tmp;

	if (size > 0)
	{
		size = align_up(size, sizeof(void *));

		malloc_lock();

        /*
         * search a usage block
         */
		TAILQ_FOREACH_SAFE(found_block, &free_list_head, link, tmp)
		{
			if (found_block->size >= size + MIN_ALLOC_SZ)
			{
				ptr = found_block->block;
				break;
			}
		}

		if (ptr)
		{
		    /*
             * found_block:
             *   ------------------------------------------------------------------
             *  |H| block, size >= size + MIN_ALLOC_SZ                             |
             *   ------------------------------------------------------------------
             *
             * after split:
             *   ---------------|--------------------------------------------------
             *  | found_block   | new_block                                        |
             *  |H| block       |H| block                                          |
             *   ---------------|--------------------------------------------------
             *
             * memory physical address: low -¡úhigh
             *
             */

			blk_node_t *new_block = (blk_node_t *)((uintptr_t)(found_block->block) + size);
			new_block->flag  = 0;
			new_block->size  = found_block->size - size - BLOCK_HEADER_SZ;
			new_block->block = (char *)new_block + BLOCK_HEADER_SZ;

			found_block->size = size;
			found_block->flag = ALLOC_FLAG;
				
			/*
             * queue list:
             *
             *              new_block            found_block
             *          -¡ú ----------       -¡ú ----------       -¡ú...
             *        ¨J   |          |    ¨J   |          |    ¨J
             *      ¨J     |          |  ¨J     |          |  ¨J
             * ...¨J       | tqe_next |¨J       | tqe_next |¨J
             *             |          |         |          |
             *             |          |         |          |
             *              ----------           ----------
             *
             * XXX new_block has free memory to malloc
             */

            TAILQ_INSERT_BEFORE(found_block, new_block, link);
		}

		malloc_unlock();
	}

    debug_dump_list();

	return ptr;
}

void free(void *ptr)
{
	if (ptr)
	{
		blk_node_t *current_block = (blk_node_t *)((uintptr_t)ptr - BLOCK_HEADER_SZ);
		blk_node_t *block = NULL, *tmp, *next_block = NULL, *last_block = NULL;

		malloc_lock();

		/*
         * queue list:
         *
         *              last_block           current_block        next_block
         *          -¡ú ----------       -¡ú ----------       -¡ú ----------        -¡ú...
         *        ¨J   |          |    ¨J   |          |    ¨J    |          |    ¨J
         *      ¨J     |          |  ¨J     |          |  ¨J      |          |  ¨J
         * ...¨J       | tqe_next |¨J       | tqe_next |¨J        | tqe_next |¨J
         *             |          |         |          |          |          |
         *             |          |         |          |          |          |
         *              ----------           ----------            ----------
         *
         */

		/*
         * memory physical address: low -¡úhigh
         *
         *   ----------------|---------------|---------------------------------
         *  | next_block     | current_block | last_block                      |
         *  |H| block        |H| block       |H| block                         |
         *   ----------------|---------------|---------------------------------
         *
         */

		TAILQ_FOREACH_SAFE(block, &free_list_head, link, tmp)
		{
			if (block == current_block)
			{
			    current_block->flag = 0;

			    /*
                 * combine with last block
                 */
                if ((last_block != NULL) && (last_block->flag == 0) &&
                   ((uintptr_t)block->block + block->size == (uintptr_t)last_block))
                {
                    block->size += last_block->size + BLOCK_HEADER_SZ;
                    TAILQ_REMOVE(&free_list_head, last_block, link);
                }

			    /*
                 * combine with next block
                 */
                next_block = block->link.tqe_next;
                if ((next_block != NULL) && (next_block->flag == 0) &&
                   ((uintptr_t)next_block->block + next_block->size == (uintptr_t)block))
                {
                    next_block->size += block->size + BLOCK_HEADER_SZ;
                    TAILQ_REMOVE(&free_list_head, block, link);
                }

				break;
			}
			
			last_block = block;
		}

        debug_dump_list();

		malloc_unlock();
	}
}

void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb * size);
    
    if (ptr)
    {
        memset(ptr, 0x00, nmemb * size);
    }
    
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    void *newptr = malloc(size);
    
    if (newptr)
    {
        memcpy(newptr, ptr, size);
        
        free(ptr);
        
        return newptr;
    }
    
    free(ptr);
    
    return NULL;
}

//-------------------------------------------------------------------------------------------------

void *aligned_malloc(size_t size, unsigned int align)
{
    void *head;
    void **addr;

    head = (void *)malloc(size + align - 1 + sizeof(void *));

    if (size == 0 || head == NULL)
        return NULL;

    size_t i = (uintptr_t)head + sizeof(void *);

    while (i < (uintptr_t)head + sizeof(void *) + align - 1)
    {
        if  (i % align == 0)
        {
            addr = (void **)i;
            break;
        }

        i++;
    }

    addr[-1] = head;

    return addr;
}

void aligned_free(void *addr)
{
	if (addr)
	{
		void *ptr = ((void **)addr)[-1];
		free(ptr);
	}
}

//-------------------------------------------------------------------------------------------------

/*
 * lock alloc with mutex support
 */
static osal_mutex_t p_alloc_mutex = NULL;

void malloc_create_oslock(void)
{
    if (!p_alloc_mutex)
    {
        p_alloc_mutex = osal_mutex_create("heap_loc", OSAL_OPT_FIFO);
    }
}

void malloc_lock()
{
    if (osal_is_osrunning())
    {
        osal_mutex_obtain(p_alloc_mutex, OSAL_WAIT_FOREVER);
    }
}

void malloc_unlock()
{
    if (osal_is_osrunning())
    {
        osal_mutex_release(p_alloc_mutex);
    }
}

void malloc_addblock(void *addr, size_t size)
{
	blk_node_t *new_memory_block = (void *)align_up((uintptr_t)addr, sizeof(void *));

	new_memory_block->size  = (uintptr_t)addr + size - (uintptr_t)new_memory_block - BLOCK_HEADER_SZ;
    new_memory_block->block = (void *)new_memory_block + BLOCK_HEADER_SZ;
    new_memory_block->flag  = 0;

	malloc_lock();

    TAILQ_INSERT_HEAD(&free_list_head, new_memory_block, link);

	malloc_unlock();
}

void malloc_init(void)
{
    TAILQ_INIT(&free_list_head);
}

/**
 * ²é¿´Ê£Óà¿ÕÏÐÄÚ´æ
 */
size_t get_heap_free_size(void)
{
    size_t free_size = 0;
    blk_node_t *block = NULL, *tmp;

	malloc_lock();

	TAILQ_FOREACH_SAFE(block, &free_list_head, link, tmp)
	{
	    if (block->flag == 0)
            free_size += block->size;
	}

    malloc_unlock();

    return free_size;
}

#endif // #ifndef OS_RTTHREAD

//-------------------------------------------------------------------------------------------------

#endif // #if !BSP_USE_FS

/*
 * @@ END
 */
