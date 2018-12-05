/****************************************************************************
 *
 * Copyright 2016-2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 *
 *   Copyright (C) 2007-2009, 2013-2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_MM_MM_H
#define __INCLUDE_MM_MM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <sys/types.h>
#include <stdbool.h>
#include <semaphore.h>
#ifdef CONFIG_HEAPINFO_USER_GROUP
#include <tinyara/mm/heapinfo_internal.h>
#endif

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/
/* Configuration ************************************************************/
/* If the MCU has a small (16-bit) address capability, then we will use
 * a smaller chunk header that contains 16-bit size/offset information.
 * We will also use the smaller header on MCUs with wider addresses if
 * CONFIG_MM_SMALL is selected.  This configuration is common with MCUs
 * that have a large FLASH space, but only a tiny internal SRAM.
 */

#ifdef CONFIG_SMALL_MEMORY
/* If the MCU has a small addressing capability, then for the smaller
 * chunk header.
 */

#undef  CONFIG_MM_SMALL
#define CONFIG_MM_SMALL 1
#endif

/* Terminology:
 *
 * - Flat Build: In the flat build (CONFIG_BUILD_FLAT=y), there is only a
 *   single heap access with the standard allocations (malloc/free).  This
 *   heap is referred to as the user heap.  The kernel logic must
 *   initialize this single heap at boot time.
 * - Protected build: In the protected build (CONFIG_BUILD_PROTECTED=y)
 *   where an MPU is used to protect a region of otherwise flat memory,
 *   there will be two allocators:  One that allocates protected (kernel)
 *   memory and one that allocates unprotected (user) memory.  These are
 *   referred to as the kernel and user heaps, respectively.  Both must be
 *   initialized by the kernel logic at boot time.
 * - Kernel Build: If the architecture has an MMU, then it may support the
 *   kernel build (CONFIG_BUILD_KERNEL=y).  In this configuration, there
 *   is one kernel heap but multiple user heaps:  One per task group.
 *   However, in this case, the kernel need only be concerned about
 *   initializing the single kernel heap here.  User heaps will be created
 *   as tasks are created.
 *
 * These special definitions are provided:
 *
 *   MM_KERNEL_USRHEAP_INIT
 *     Special kernel interfaces to the kernel user-heap are required
 *     for heap initialization.
 *   CONFIG_MM_KERNEL_HEAP
 *     The configuration requires a kernel heap that must initialized
 *     at boot-up.
 */

#undef MM_KERNEL_USRHEAP_INIT
#if defined(CONFIG_BUILD_PROTECTED) && defined(__KERNEL__)
#define MM_KERNEL_USRHEAP_INIT 1
#elif !defined(CONFIG_BUILD_KERNEL)
#define MM_KERNEL_USRHEAP_INIT 1
#endif

/* The kernel heap is never accessible from user code */

#ifndef __KERNEL__
#undef CONFIG_MM_KERNEL_HEAP
#endif

/* Chunk Header Definitions *************************************************/
/* These definitions define the characteristics of allocator
 *
 * MM_MIN_SHIFT is used to define MM_MIN_CHUNK.
 * MM_MIN_CHUNK - is the smallest physical chunk that can be allocated.  It
 *   must be at least a large as sizeof(struct mm_freenode_s).  Larger values
 *   may improve performance slightly, but will waste memory due to
 *   quantization losses.
 *
 * MM_MAX_SHIFT is used to define MM_MAX_CHUNK
 * MM_MAX_CHUNK is the largest, contiguous chunk of memory that can be
 *   allocated.  It can range from 16-bytes to 4Gb.  Larger values of
 *   MM_MAX_SHIFT can cause larger data structure sizes and, perhaps,
 *   minor performance losses.
 */

#if defined(CONFIG_MM_SMALL) && UINTPTR_MAX <= UINT32_MAX
/* Two byte offsets; Pointers may be 2 or 4 bytes;
 * sizeof(struct mm_freenode_s) is 8 or 12 bytes.
 * REVISIT: We could do better on machines with 16-bit addressing.
 */

#define MM_MIN_SHIFT    4		/* 16 bytes */
#define MM_MAX_SHIFT   15		/* 32 Kb */

#elif defined(CONFIG_HAVE_LONG_LONG)
/* Four byte offsets; Pointers may be 4 or 8 bytes
 * sizeof(struct mm_freenode_s) is 16 or 24 bytes.
 */

#if UINTPTR_MAX <= UINT32_MAX
#define MM_MIN_SHIFT  4			/* 16 bytes */
#elif UINTPTR_MAX <= UINT64_MAX
#define MM_MIN_SHIFT  5			/* 32 bytes */
#endif
#define MM_MAX_SHIFT   22		/*  4 Mb */

#else
/* Four byte offsets; Pointers must be 4 bytes.
 * sizeof(struct mm_freenode_s) is 16 bytes.
 */

#define MM_MIN_SHIFT    4		/* 16 bytes */
#define MM_MAX_SHIFT   22		/*  4 Mb */
#endif

/* All other definitions derive from these two */

#define MM_MIN_CHUNK     (1 << MM_MIN_SHIFT)
#define MM_MAX_CHUNK     (1 << MM_MAX_SHIFT)
#define MM_NNODES        (MM_MAX_SHIFT - MM_MIN_SHIFT + 1)

#define MM_GRAN_MASK     (MM_MIN_CHUNK-1)
#define MM_ALIGN_UP(a)   (((a) + MM_GRAN_MASK) & ~MM_GRAN_MASK)
#define MM_ALIGN_DOWN(a) ((a) & ~MM_GRAN_MASK)

/* An allocated chunk is distinguished from a free chunk by bit 31 (or 15)
 * of the 'preceding' chunk size.  If set, then this is an allocated chunk.
 */

#ifdef CONFIG_MM_SMALL
#define MM_ALLOC_BIT    0x8000
#else
#define MM_ALLOC_BIT    0x80000000
#endif
#define MM_IS_ALLOCATED(n) ((int)((struct mm_allocnode_s*)(n)->preceding) < 0)

/****************************************************************************
 * Public Types
 ****************************************************************************/

#define HEAPINFO_SIMPLE 1
#define HEAPINFO_DETAIL_ALL 2
#define HEAPINFO_DETAIL_PID 3
#define HEAPINFO_DETAIL_FREE 4
#define HEAPINFO_DETAIL_SPECIFIC_HEAP 5
#define HEAPINFO_PID_ALL -1
#define HEAPINFO_INIT_INFO 0
#define HEAPINFO_ADD_INFO 1
#define HEAPINFO_DEL_INFO 2

#define REGION_START (size_t)regionx_start[0]
#define REGION_SIZE  regionx_size[0]
#define REGION_END (REGION_START + REGION_SIZE)
#define INVALID_HEAP_IDX -1

/* Determines the size of the chunk size/offset type */

#ifdef CONFIG_MM_SMALL
typedef uint16_t mmsize_t;
#define MMSIZE_MAX 0xffff
#else
typedef size_t mmsize_t;
#define MMSIZE_MAX SIZE_MAX
#endif

/* typedef is used for defining size of address space */

#ifdef CONFIG_DEBUG_MM_HEAPINFO
typedef size_t mmaddress_t;		/* 32 bit address space */

#if defined(CONFIG_ARCH_MIPS)
/* Macro gets return address of malloc API */
#define ARCH_GET_RET_ADDRESS \
	mmaddress_t retaddr = 0; \
	do { \
		asm volatile ("sw $ra, %0" : "=m" (retaddr)); \
	} while (0);
#elif defined(CONFIG_ARCH_ARM)
#define ARCH_GET_RET_ADDRESS \
	mmaddress_t retaddr = 0; \
	do { \
		asm volatile ("mov %0,lr\n" : "=r" (retaddr));\
	} while (0);

#else
#error Unknown CONFIG_ARCH option, malloc debug feature wont work.
#endif

#define SIZEOF_MM_MALLOC_DEBUG_INFO \
	(sizeof(mmaddress_t) + sizeof(pid_t) + sizeof(uint16_t))
#endif

/* This describes an allocated chunk.  An allocated chunk is
 * distinguished from a free chunk by bit 15/31 of the 'preceding' chunk
 * size.  If set, then this is an allocated chunk.
 */

struct mm_allocnode_s {
	mmsize_t size;					/* Size of this chunk */
	mmsize_t preceding;				/* Size of the preceding chunk */
#ifdef CONFIG_DEBUG_MM_HEAPINFO
	mmaddress_t alloc_call_addr;			/* malloc call address */
	pid_t pid;					/* PID info */
	uint16_t reserved;				/* Reserved for future use. */
#endif

};

/* What is the size of the allocnode? */

#ifdef CONFIG_MM_SMALL
#ifdef CONFIG_DEBUG_MM_HEAPINFO
/* 10 = (uint16_t + uint16_t + uint16_t + uint16_t + uint16_t ) */
#define SIZEOF_MM_ALLOCNODE   (sizeof(mmsize_t) + sizeof(mmsize_t) + SIZEOF_MM_MALLOC_DEBUG_INFO)
#else
/* 4 = (uint16_t + uint16_t) */
#define SIZEOF_MM_ALLOCNODE   (sizeof(mmsize_t) + sizeof(mmsize_t))
#endif

#else

#ifdef CONFIG_DEBUG_MM_HEAPINFO
/* 16 = (uint32_t + uint32_t + uint32_t + uint16_t + uint16_t ) */
#define SIZEOF_MM_ALLOCNODE  (sizeof(mmsize_t) + sizeof(mmsize_t) + SIZEOF_MM_MALLOC_DEBUG_INFO)
#else
/* 8 = (uint32_t + uint32_t) */
#define SIZEOF_MM_ALLOCNODE   (sizeof(mmsize_t) + sizeof(mmsize_t))
#endif
#endif

#define CHECK_ALLOCNODE_SIZE \
	DEBUGASSERT(sizeof(struct mm_allocnode_s) == SIZEOF_MM_ALLOCNODE)

/* This describes a free chunk */

struct mm_freenode_s {
	mmsize_t size;				/* Size of this chunk */
	mmsize_t preceding;			/* Size of the preceding chunk */
	FAR struct mm_freenode_s *flink;	/* Supports a doubly linked list */
	FAR struct mm_freenode_s *blink;
};

/* What is the size of the freenode? */

#define MM_PTR_SIZE sizeof(FAR struct mm_freenode_s *)
#ifdef CONFIG_DEBUG_MM_HEAPINFO
#define SIZEOF_MM_FREENODE \
	(SIZEOF_MM_ALLOCNODE - SIZEOF_MM_MALLOC_DEBUG_INFO + 2 * MM_PTR_SIZE)
#else
#define SIZEOF_MM_FREENODE (SIZEOF_MM_ALLOCNODE + 2 * MM_PTR_SIZE)
#endif

#define CHECK_FREENODE_SIZE \
	DEBUGASSERT(sizeof(struct mm_freenode_s) == SIZEOF_MM_FREENODE)

#ifdef CONFIG_HEAPINFO_USER_GROUP
struct heapinfo_group_info_s {
	int pid;
	int group;
	int stack_size;
};

struct heapinfo_group_s {
	int curr_size;
	int peak_size;
	int stack_size;
	int heap_size;
};
#endif
/* This describes one heap (possibly with multiple regions) */

struct mm_heap_s {
	/* Mutually exclusive access to this data set is enforced with
	 * the following un-named semaphore.
	 */

	sem_t mm_semaphore;
	pid_t mm_holder;
	int mm_counts_held;

	/* This is the size of the heap provided to mm */

	size_t mm_heapsize;
#ifdef CONFIG_DEBUG_MM_HEAPINFO
	size_t peak_alloc_size;
	size_t total_alloc_size;
#ifdef CONFIG_HEAPINFO_USER_GROUP
	int max_group;
	struct heapinfo_group_s group[HEAPINFO_USER_GROUP_NUM];
#endif
#endif

	/* This is the first and last nodes of the heap */

	FAR struct mm_allocnode_s *mm_heapstart[CONFIG_MM_REGIONS];
	FAR struct mm_allocnode_s *mm_heapend[CONFIG_MM_REGIONS];

#if CONFIG_MM_REGIONS > 1
	int mm_nregions;
#endif

	/* All free nodes are maintained in a doubly linked list.  This
	 * array provides some hooks into the list at various points to
	 * speed searches for free nodes.
	 */

	struct mm_freenode_s mm_nodelist[MM_NNODES];
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#if !defined(CONFIG_BUILD_PROTECTED) || !defined(__KERNEL__)
/* User heap structure:
 *
 * - Flat build:  In the FLAT build, the user heap structure is a globally
 *   accessible variable.
 * - Protected build:  The user heap structure is directly available only
 *   in user space.
 * - Kernel build: There are multiple heaps, one per process.  The heap
 *   structure is associated with the address environment and there is
 *   no global user heap structure.
 */
extern struct mm_heap_s g_mmheap[CONFIG_MM_NHEAPS];
#endif

#ifdef CONFIG_MM_KERNEL_HEAP
/* This is the kernel heap */

EXTERN struct mm_heap_s g_kmmheap;
#endif

#if defined(CONFIG_ARCH_ADDRENV) && defined(CONFIG_BUILD_KERNEL)
/* In the kernel build, there are multiple user heaps; one for each task
 * group.  In this build configuration, the user heap structure lies
 * in a reserved region at the beginning of the .bss/.data address
 * space (CONFIG_ARCH_DATA_VBASE).  The size of that region is given by
 * ARCH_DATA_RESERVE_SIZE
 */

#include <tinyara/addrenv.h>
#define BASE_HEAP (&ARCH_DATA_RESERVE->ar_usrheap)

#else
/* Otherwise, the user heap data structures are in common .bss */

#define BASE_HEAP &g_mmheap[0]
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Functions contained in mm_initialize.c ***********************************/

void mm_initialize(FAR struct mm_heap_s *heap, FAR void *heap_start, size_t heap_size);
void mm_addregion(FAR struct mm_heap_s *heap, FAR void *heapstart, size_t heapsize);

/* Functions contained in umm_initialize.c **********************************/

void umm_initialize(FAR void *heap_start, size_t heap_size);

/* Functions contained in kmm_initialize.c **********************************/

#ifdef CONFIG_MM_KERNEL_HEAP
void kmm_initialize(FAR void *heap_start, size_t heap_size);
#endif

/* Functions contained in umm_addregion.c ***********************************/

#if !defined(CONFIG_BUILD_PROTECTED) || !defined(__KERNEL__)
void umm_addregion(FAR void *heapstart, size_t heapsize);
#endif

/* Functions contained in kmm_addregion.c ***********************************/

#ifdef CONFIG_MM_KERNEL_HEAP
void kmm_addregion(FAR void *heapstart, size_t heapsize);
#endif

/* Functions contained in mm_sem.c ******************************************/

void mm_seminitialize(FAR struct mm_heap_s *heap);
void mm_takesemaphore(FAR struct mm_heap_s *heap);
int mm_trysemaphore(FAR struct mm_heap_s *heap);
void mm_givesemaphore(FAR struct mm_heap_s *heap);

/* Functions contained in umm_sem.c ****************************************/

#if !defined(CONFIG_BUILD_PROTECTED) || !defined(__KERNEL__)
int umm_trysemaphore(void *address);
void umm_givesemaphore(void *address);
#endif

/* Functions contained in kmm_sem.c ****************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
int kmm_trysemaphore(void *address);
void kmm_givesemaphore(void *address);
#endif
#ifdef CONFIG_DEBUG_MM_HEAPINFO

/* Functions contained in mm_malloc.c ***************************************/

FAR void *mm_malloc(FAR struct mm_heap_s *heap, size_t size, mmaddress_t caller_retaddr);

#else

/* Functions contained in mm_malloc.c ***************************************/

FAR void *mm_malloc(FAR struct mm_heap_s *heap, size_t size);
#endif

/* Functions contained in kmm_malloc.c **************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_malloc(size_t size);
#endif

/* Functions contained in mm_free.c *****************************************/

void mm_free(FAR struct mm_heap_s *heap, FAR void *mem);

/* Functions contained in kmm_free.c ****************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
void kmm_free(FAR void *mem);
#endif

#ifdef CONFIG_DEBUG_MM_HEAPINFO

/* Functions contained in mm_realloc.c **************************************/

FAR void *mm_realloc(FAR struct mm_heap_s *heap, FAR void *oldmem, size_t size, mmaddress_t caller_retaddr);

#else

/* Functions contained in mm_realloc.c **************************************/

FAR void *mm_realloc(FAR struct mm_heap_s *heap, FAR void *oldmem, size_t size);
#endif

/* Functions contained in kmm_realloc.c *************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_realloc(FAR void *oldmem, size_t newsize);
#endif
#ifdef CONFIG_DEBUG_MM_HEAPINFO

/* Functions contained in mm_calloc.c ***************************************/

FAR void *mm_calloc(FAR struct mm_heap_s *heap, size_t n, size_t elem_size, mmaddress_t caller_retaddr);

#else

/* Functions contained in mm_calloc.c ***************************************/

FAR void *mm_calloc(FAR struct mm_heap_s *heap, size_t n, size_t elem_size);
#endif

/* Functions contained in kmm_calloc.c **************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_calloc(size_t n, size_t elem_size);
#endif

#ifdef CONFIG_DEBUG_MM_HEAPINFO
/* Functions contained in mm_zalloc.c ***************************************/

FAR void *mm_zalloc(FAR struct mm_heap_s *heap, size_t size, mmaddress_t caller_retaddr);
#else
/* Functions contained in mm_zalloc.c ***************************************/

FAR void *mm_zalloc(FAR struct mm_heap_s *heap, size_t size);
#endif

/* Functions contained in kmm_zalloc.c **************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_zalloc(size_t size);
#endif
#ifdef CONFIG_DEBUG_MM_HEAPINFO
/* Functions contained in mm_memalign.c *************************************/

FAR void *mm_memalign(FAR struct mm_heap_s *heap, size_t alignment, size_t size, mmaddress_t caller_retaddr);

#else

/* Functions contained in mm_memalign.c *************************************/

FAR void *mm_memalign(FAR struct mm_heap_s *heap, size_t alignment, size_t size);
#endif

/* Functions contained in kmm_memalign.c ************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_memalign(size_t alignment, size_t size);
#endif

/* Functions contained in kmm_heapmember.c **********************************/

#if defined(CONFIG_MM_KERNEL_HEAP) && defined(CONFIG_DEBUG)
bool kmm_heapmember(FAR void *mem);
#endif

/* Functions contained in mm_brkaddr.c **************************************/

FAR void *mm_brkaddr(FAR struct mm_heap_s *heap, int region);

/* Functions contained in umm_brkaddr.c *************************************/

#if !defined(CONFIG_BUILD_PROTECTED) || !defined(__KERNEL__)
FAR void *umm_brkaddr(int region);
#endif

/* Functions contained in kmm_brkaddr.c *************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
FAR void *kmm_brkaddr(int region);
#endif

/* Functions contained in mm_sbrk.c *****************************************/

#if defined(CONFIG_ARCH_ADDRENV) && defined(CONFIG_MM_PGALLOC) && \
	defined(CONFIG_ARCH_USE_MMU)
FAR void *mm_sbrk(FAR struct mm_heap_s *heap, intptr_t incr, uintptr_t maxbreak);
#endif

/* Functions contained in kmm_sbrk.c ****************************************/

#if defined(CONFIG_MM_KERNEL_HEAP) && defined(CONFIG_ARCH_ADDRENV) && \
	defined(CONFIG_MM_PGALLOC) && defined(CONFIG_ARCH_USE_MMU)
FAR void *kmm_sbrk(intptr_t incr);
#endif

/* Functions contained in mm_extend.c ***************************************/

void mm_extend(FAR struct mm_heap_s *heap, FAR void *mem, size_t size, int region);

/* Functions contained in umm_extend.c **************************************/

#if !defined(CONFIG_BUILD_PROTECTED) || !defined(__KERNEL__)
void umm_extend(FAR void *mem, size_t size, int region);
#endif

/* Functions contained in kmm_extend.c **************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
void kmm_extend(FAR void *mem, size_t size, int region);
#endif

/* Functions contained in mm_mallinfo.c *************************************/

struct mallinfo;				/* Forward reference */
int mm_mallinfo(FAR struct mm_heap_s *heap, FAR struct mallinfo *info);

/* Functions contained in kmm_mallinfo.c ************************************/

#ifdef CONFIG_MM_KERNEL_HEAP
#ifdef CONFIG_CAN_PASS_STRUCTS
struct mallinfo kmm_mallinfo(void);
#else
int kmm_mallinfo(struct mallinfo *info);
#endif
#endif							/* CONFIG_CAN_PASS_STRUCTS */

/* Functions contained in mm_shrinkchunk.c **********************************/

void mm_shrinkchunk(FAR struct mm_heap_s *heap, FAR struct mm_allocnode_s *node, size_t size);

/* Functions contained in mm_addfreechunk.c *********************************/

void mm_addfreechunk(FAR struct mm_heap_s *heap, FAR struct mm_freenode_s *node);

/* Functions contained in mm_size2ndx.c.c ***********************************/

int mm_size2ndx(size_t size);

#ifdef CONFIG_DEBUG_MM_HEAPINFO
/* Functions contained in kmm_mallinfo.c . Used to display memory allocation details */
void heapinfo_parse(FAR struct mm_heap_s *heap, int mode, pid_t pid);
/* Funciton to add memory allocation info */
void heapinfo_update_node(FAR struct mm_allocnode_s *node, mmaddress_t caller_retaddr);

void heapinfo_add_size(pid_t pid, mmsize_t size);
void heapinfo_subtract_size(pid_t pid, mmsize_t size);
void heapinfo_update_total_size(struct mm_heap_s *heap, mmsize_t size, pid_t pid);
void heapinfo_exclude_stacksize(void *stack_ptr);
#ifdef CONFIG_HEAPINFO_USER_GROUP
void heapinfo_update_group_info(pid_t pid, int group, int type);
void heapinfo_check_group_list(pid_t pid, char *name);
#endif
#endif
void mm_is_sem_available(void *address);

/* Functions to get heap information */
struct mm_heap_s *mm_get_heap_info(void *address);

int mm_get_heapindex(void *mem);
#if CONFIG_MM_NHEAPS > 1
struct heapinfo_total_info_s {
	int total_heap_size;
	int cur_free;
	int largest_free_size;
	int cur_dead_thread;
	int sum_of_stacks;
	int sum_of_heaps;
	int cur_alloc_size;
	int peak_alloc_size;
};
typedef struct heapinfo_total_info_s heapinfo_total_info_t;

/**
 * @cond
 * @internal
 */

/**
 * @brief Allocate memory to the specific heap.
 * @details @b #include <tinyara/mm/mm.h>\n
 *   malloc_at tries to allocate memory for a specific heap which passed by api argument.
 *   If there is no enough space to allocate, it will return NULL.
 * @param[in] heap_index Index of specific heap
 * @param[in] size size (in bytes) of the memory region to be allocated
 * 
 * @return On success, the address of the allocated memory is returned. On failure, NULL is returned.
 * @since TizenRT v2.1 PRE
 */
void *malloc_at(int heap_index, size_t size);
/**
 * @brief Calloc to the specific heap.
 * @details @b #include <tinyara/mm/mm.h>\n
 *   calloc_at tries to allocate memory for a specific heap which passed by api argument.
 *   If there is no enough space to allocate, it will return NULL.
 * @param[in] heap_index Index of specific heap
 * @param[in] n the number of elements to be allocated
 * @param[in] elem_size the size of elements
 * 
 * @return On success, the address of the allocated memory is returned. On failure, NULL is returned.
 * @since TizenRT v2.1 PRE
 */
void *calloc_at(int heap_index, size_t n, size_t elem_size);
/**
 * @brief Memalign to the specific heap.
 * @details @b #include <tinyara/mm/mm.h>\n
 *   memalign_at tries to align the memory for a specific heap which passed by api argument.
 *   If there is no enough space, it will return NULL.
 * @param[in] heap_index Index of specific heap
 * @param[in] alignment A power of two for alignment
 * @param[in] size Allocated memory size
 * 
 * @return On success, the address of the allocated memory is returned. On failure, NULL is returned.
 * @since TizenRT v2.1 PRE
 */
void *memalign_at(int heap_index, size_t alignment, size_t size);
/**
 * @brief Realloc to the specific heap.
 * @details @b #include <tinyara/mm/mm.h>\n
 *   realloc_at tries to allocate memory for a specific heap which passed by api argument.
 *   If there is no enough space to allocate, it will return NULL.
 * @param[in] heap_index Index of specific heap
 * @param[in] oldmem the pointer to a memory block previously allocated
 * @param[in] size the new size for the memory block
 * 
 * @return On success, the address of the allocated memory is returned. On failure, NULL is returned.
 * @since TizenRT v2.1 PRE
 */
void *realloc_at(int heap_index, void *oldmem, size_t size);
/**
 * @brief Zalloc to the specific heap.
 * @details @b #include <tinyara/mm/mm.h>\n
 *   zalloc_at tries to allocate memory for a specific heap which passed by api argument.
 *   If there is no enough space to allocate, it will return NULL.
 * @param[in] heap_index Index of specific heap
 * @param[in] size size (in bytes) of the memory region to be allocated
 * 
 * @return On success, the address of the allocated memory is returned. On failure, NULL is returned.
 * @since TizenRT v2.1 PRE
 */
void *zalloc_at(int heap_index, size_t size);
#else
#define malloc_at(heap_index, size)              malloc(size)
#define calloc_at(heap_index, n, elem_size)      calloc(n, elem_size)
#define memalign_at(heap_index, alignment, size) memalign(alignment, size)
#define realloc_at(heap_index, oldmem, size)     realloc(oldmem, size)
#define zalloc_at(heap_index, size)              zalloc(size)
#endif

/**
 * @endcond
 */

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif							/* __INCLUDE_MM_MM_H */
