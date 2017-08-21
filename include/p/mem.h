/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file p/mem.h
 * @brief Memory management
 * @author Alexander Saprykin
 *
 * Usually the system routines for memory management are used: malloc(),
 * realloc(), free() and so on. But it is highly encouraged to use a more
 * general approach: p_malloc(), p_malloc0(), p_realloc() and p_free() family of
 * memory management routines. It gives you several advantages:
 * - automatical checking of all input parameters for the NULL values;
 * - ability to use a custom memory allocator.
 * You can also mix these two families of calls, but it is not recommended.
 *
 * By default p_* routines are mapped to system calls, thus only NULL-checking
 * is additionally performed. If you want to use the custom memory allocator,
 * then fill in #PMemVTable structure and pass it to the p_mem_set_vtable(). To
 * restore system calls back use p_mem_restore_vtable().
 *
 * Be careful when using the custom memory allocator: all memory chunks
 * allocated with the custom allocator must be freed with the same allocator. If
 * the custom allocator was installed after the library initialization call
 * p_libsys_init() then you must to restore the original allocator before
 * calling p_libsys_shutdown().
 *
 * Use p_mem_mmap() to allocate system memory using memory mapping and
 * p_mem_munmap() to release the mapped memory. This type of allocated memory
 * is not backed physically (does not consume any physical storage) by operating
 * system. It means that every memory page within the allocated region will be
 * committed to physical backend only when you first touch it. Until that
 * untouched pages will be reserved for future usage. It can be useful when
 * dealing with large memory blocks which should be filled with data on demand,
 * i.e. custom memory allocator can request a large block first, and then it
 * allocates chunks of memory within the block upon request.
 *
 * @note OS/2 supports non-backed memory pages allocation, but in a specific
 * way: an exception handler to control access to uncommitted pages must be
 * allocated on the stack of each thread before using the mapped memory. To
 * unify the behaviour, on OS/2 all memory mapped allocations are already
 * committed to the backing storage.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef P_MEM_H__
#define P_MEM_H__

#include "types.h"
#include "macros.h"
#include "error.h"

/** Memory management table. */
typedef struct PMemVTable_ {
  ptr_t (*malloc)(size_t n_bytes);  /**< malloc() implementation.	*/
  ptr_t (*realloc)(ptr_t mem,
    size_t n_bytes);  /**< realloc() implementation.	*/
  void (*free)(ptr_t mem);    /**< free() implementation.	*/
} PMemVTable;

/**
 * @brief Allocates a memory block for the specified number of bytes.
 * @param n_bytes Size of the memory block in bytes.
 * @return Pointer to a newly allocated memory block in case of success, NULL
 * otherwise.
 * @since 0.0.1
 */
P_API ptr_t p_malloc(size_t n_bytes);

/**
 * @brief Allocates a memory block for the specified number of bytes and fills
 * it with zeros.
 * @param n_bytes Size of the memory block in bytes.
 * @return Pointer to a newly allocated memory block filled with zeros in case
 * of success, NULL otherwise.
 * @since 0.0.1
 */
P_API ptr_t p_malloc0(size_t n_bytes);

/**
 * @brief Changes the memory block size.
 * @param mem Pointer to the memory block.
 * @param n_bytes New size for @a mem block.
 * @return Pointer to a newlly allocated memory block in case of success (if
 * @a mem is NULL then it acts like p_malloc()), NULL otherwise.
 * @since 0.0.1
 */
P_API ptr_t p_realloc(ptr_t mem,
  size_t n_bytes);

/**
 * @brief Frees a memory block by its pointer.
 * @param mem Pointer to the memory block to free.
 * @since 0.0.1
 *
 * You should only call this function for the pointers which were obtained using
 * the p_malloc(), p_malloc0() and p_realloc() routines, otherwise behavior is
 * unpredictable.
 *
 * Checks the pointer for the NULL value.
 */
P_API void p_free(ptr_t mem);

/**
 * @brief Sets custom memory management routines.
 * @param table Table of the memory routines to use.
 * @return TRUE if the table was accepted, FALSE otherwise.
 * @note All members of @a table must be non-NULL.
 * @note This call is not thread-safe.
 * @warning Do not forget to set the original memory management routines before
 * calling p_libsys_shutdown() if you have used p_mem_set_vtable() after the
 * library initialization.
 * @since 0.0.1
 *
 * In most cases you do not need to use this function. Use it only when you know
 * what are you doing!
 */
P_API bool p_mem_set_vtable(const PMemVTable *table);

/**
 * @brief Restores system memory management routines.
 * @note This call is not thread-safe.
 * @since 0.0.1
 *
 * The following system routines are restored: malloc(), free(), realloc().
 */
P_API void p_mem_restore_vtable(void);

/**
 * @brief Gets a memory mapped block from the system.
 * @param n_bytes Size of the memory block in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to the allocated memory block in case of success, NULL
 * otherwise.
 * @since 0.0.1
 *
 * Note that some systems can allocate memory only in chunks of the page size,
 * so if @a n_bytes is less than the page size it will try to allocate a chunk
 * of memory equal to the page size instead.
 *
 * On most systems returned memory is mapped to the null or swap device.
 *
 * @warning On OS/2 returned memory is mapped to physical storage and can be
 * swapped.
 */
P_API ptr_t p_mem_mmap(size_t n_bytes,
  p_err_t **error);

/**
 * @brief Unmaps memory back to the system.
 * @param mem Pointer to a memory block previously allocated using the
 * p_mem_mmap() call.
 * @param n_bytes Size of the memory block in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_API bool p_mem_munmap(ptr_t mem,
  size_t n_bytes,
  p_err_t **error);

#endif /* P_MEM_H__ */
