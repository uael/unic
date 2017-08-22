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

/*!@file p/shm.h
 * @brief Shared memory
 * @author Alexander Saprykin
 *
 * Shared memory is a memory segment which can be accessed from several threads
 * or processes. It provides an efficient way to transfer large blocks of data
 * between processes. It can be used as any other regular memory segment in an
 * application.
 *
 * Shared memory acts like an inter-process communication method. This memory
 * exchange implementation is process-wide so you can transfer data not only
 * between the threads. But it makes this IPC method (actually like any other
 * IPC method, as well) relatively heavy. Consider using other approaches
 * instead if you do not need to cross the process boundary.
 *
 * A shared memory segment doesn't provide any synchronization primitives itself
 * which means that several processes or threads can concurrently write and read
 * from it. This can lead to data consistency problems. To avoid such situations
 * a locking mechanism is provided: use p_shm_lock() before entering a critical
 * section on the memory segment and p_shm_unlock() when leaving this section.
 * The locking mechanism is working across the process boundary.
 *
 * A process-wide shared memory segment is identified by its name across the
 * system, thus it is also called a named memory segment. Use p_shm_new() to
 * open the named shared memory segment and p_shm_free() to close it.
 *
 * Please note the following platform specific differences:
 *
 * - Windows and OS/2 don't own IPC objects (processes own them), which means
 * that a shared memory segment will be removed after the last process or thread
 * detaches (or after terminating all the processes and threads attached to the
 * segment) it.
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive
 * an application crash: the attached shared memory segment can contain data
 * from the previous working session. This could happen if you have not detached
 * from all the shared memory segments explicitly before terminating the
 * application.
 *
 * - HP-UX has limitations due to its MPAS/MGAS features, so you couldn't attach
 * to the same memory segment twice from the same process.
 *
 * - IRIX allows to open several instances of the same buffer within the single
 * process, but it will close the object after the first close call from any of
 * the threads within the process.
 *
 * - OpenVMS (as of 8.4 release) has broken implementation of process-wide named
 * semaphores which leads to the broken shared memory also.
 *
 * - Syllable lacks support for process-wide named semaphores which leads to the
 * absence of shared memory.
 *
 * - BeOS lacks support for process-wide named semaphores which leads to the
 * absence of shared memory.
 *
 * You can take ownership of the shared memory segment with
 * p_shm_take_ownership() to explicitly remove it from the system after closing.
 */
#ifndef P_SHM_H__
# define P_SHM_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/err.h"

typedef enum shm_access shm_access_t;

/*!@brief Shared memory opaque data structure. */
typedef struct shm shm_t;

/*!@brief Enum with shared memory access permissions. */
enum shm_access {

  /*!@brief Read-only access. */
  P_SHM_ACCESS_READONLY = 0,

  /*!@brief Read/write access. */
  P_SHM_ACCESS_READWRITE = 1
};

/*!@brief Creates a new #PShm object.
 * @param name Shared memory name.
 * @param size Size of the memory segment in bytes, can't be changed later.
 * @param perms Memory segment permissions, see #PShmAccessPerms.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to a newly created #PShm object in case of success, NULL
 * otherwise.
 * @since 0.0.1
 */
P_API shm_t *
p_shm_new(const byte_t *name, size_t size, shm_access_t perms, err_t **error);

/*!@brief Takes ownership of a shared memory segment.
 * @param shm Shared memory segment.
 * @since 0.0.1
 *
 * If you take ownership of the shared memory object, p_shm_free() will try to
 * completely unlink it and remove from the system. This is useful on UNIX
 * systems where shared memory can survive an application crash. On the Windows
 * and OS/2 platforms this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * memory segment from the previous crash will be unlinked from the system. To
 * do that, call p_shm_new() and check if its condition is normal (the segment
 * size, the data). If not, take ownership of the shared memory object and
 * remove it with the p_shm_free() call. After that, create it again.
 */
P_API void
p_shm_take_ownership(shm_t *shm);

/*!@brief Frees #PShm object.
 * @param shm #PShm to free.
 * @since 0.0.1
 *
 * It doesn't unlock a given shared memory segment, be careful to not to make a
 * deadlock or a segfault while freeing the memory segment which is under usage.
 */
P_API void
p_shm_free(shm_t *shm);

/*!@brief Locks #PShm object for usage.
 * @param shm #PShm to lock.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * If the object is already locked then the thread will be suspended until the
 * object becomes unlocked.
 */
P_API bool
p_shm_lock(shm_t *shm, err_t **error);

/*!@brief Unlocks #PShm object.
 * @param shm #PShm to unlock.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_API bool
p_shm_unlock(shm_t *shm, err_t **error);

/*!@brief Gets a starting address of a #PShm memory segment.
 * @param shm #PShm to get the address for.
 * @return Pointer to the starting address in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_API ptr_t
p_shm_get_address(const shm_t *shm);

/*!@brief Gets the size of a #PShm memory segment.
 * @param shm #PShm to get the size for.
 * @return Size of the given memory segment in case of success, 0 otherwise.
 * @since 0.0.1
 *
 * Note that the returned size would be a slightly larger than specified during
 * the p_shm_new() call due to service information stored inside.
 */
P_API size_t
p_shm_get_size(const shm_t *shm);

#endif /* !P_SHM_H__ */
