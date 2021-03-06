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

/*!@file unic/sema.h
 * @brief Semaphore routines
 * @author Alexander Saprykin
 *
 * A semaphore is a synchronization primitive which controls access to shared
 * data from the concurrently running threads. Unlike a mutex (which is a
 * particular case of a binary semaphore) it allows concurrent access not to
 * only the one thread.
 *
 * The semaphore has a counter which means the number of available resources
 * (units). Before entering a critical section a thread must perform the so
 * called P (acquire) operation: if the counter is positive it decrements the
 * counter by 1 and continues execution; otherwise the thread is suspended until
 * the counter becomes positive. Before leaving the critical section the thread
 * must perform the so called V (release) operation: increments the counter by 1
 * and wakes up a waiting thread from the queue (if any).
 *
 * You can think about the semaphore as a resource controller: the P operation
 * takes one unit, while the V operation gives one unit back. The thread could
 * not continue execution without taking a resource unit. By setting the initial
 * semaphore counter value you can control how much concurrent threads can work
 * with a shared resource.
 *
 * This semaphore implementation is process-wide so you can synchronize not only
 * the threads. But it makes this IPC primitive (actually like any other IPC
 * primitive, as well) relatively heavy. Consider using a mutex or a spinlock
 * instead if you do not need to cross a process boundary.
 *
 * A process-wide semaphore is identified by its name across the system, thus it
 * is also called a named semaphore. Use u_sema_new() to open the named
 * semaphore and u_sema_free() to close it.
 *
 * Please note the following platform specific differences:
 *
 * - Windows doesn't own IPC objects (processes own them), which means that a
 * semaphore will be removed from the system after the last process or thread
 * closes it (or after terminating all the processes and threads holding open
 * semaphore).
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive
 * an application crash: an already used semaphore can be opened in a locked
 * state and an application can fail into a deadlock or an inconsistent state.
 * This could happen if you have not closed all the open semaphores explicitly
 * before terminating the application.
 *
 * - IRIX allows to open several instances of a semaphore within the single
 * process, but it will close the object after the first close call from any of
 * the threads within the process.
 *
 * - OpenVMS (as of 8.4 release) has declared prototypes for process-wide named
 * semaphores but the actual implementation is broken.
 *
 * - OS/2 lacks support for process-wide named semaphores.
 *
 * - Syllable lacks support for process-wide named semaphores.
 *
 * - BeOS lacks support for process-wide named semaphores.
 *
 * Use the third argument as #U_SEMA_CREATE in u_sema_new() to reset
 * a semaphore value while opening it. This argument is ignored on Windows. You
 * can also take ownership of the semaphore with u_sema_take_ownership() to
 * explicitly remove it from the system after closing.
 */
#ifndef U_SEM_H__
# define U_SEM_H__

#include "unic/macros.h"
#include "unic/types.h"
#include "unic/err.h"

/*!@brief Semaphore opaque data structure. */
typedef struct sema sema_t;

/*!@brief Enum with semaphore creation modes. */
enum sema_access {

  /*!@brief Opens an existing semaphore or creates one with a given value. */
  U_SEMA_OPEN = 0,

  /*!@brief Creates semaphore, resets to a given value if exists. */
  U_SEMA_CREATE = 1
};

typedef enum sema_access sema_access_t;

/*!@brief Creates a new #sema_t object.
 * @param name Semaphore name.
 * @param init_val Initial semaphore value.
 * @param mod Creation mode.
 * @param[out] err Error report object, NULL to ignore.
 * @return Pointer to a newly created #sema_t object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 *
 * The @a init_val is used only in one of following cases: a semaphore with the
 * such name doesn't exist, or the semaphore with the such name exists but
 * @a mode specified as #U_SEMA_CREATE (non-Windows platforms only). In
 * other cases @a init_val is ignored. The @a name is system-wide, so any other
 * process can open that semaphore passing the same name.
 */
U_API sema_t *
u_sema_new(const byte_t *name, int init_val, sema_access_t mod, err_t **err);

/*!@brief Takes ownership of a semaphore.
 * @param sem Semaphore to take ownership.
 * @since 0.0.1
 *
 * If you take ownership of a semaphore object, u_sema_free() will try to
 * completely unlink it and remove from the system. This is useful on UNIX
 * systems where the semaphore can survive an application crash. On the Windows
 * platform this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * semaphore from the previous crash will be unlinked from the system. To do
 * that, call u_sema_new(), take ownership of the semaphore object and
 * remove it with the u_sema_free() call. After that, create it again.
 *
 * You can also do the same thing upon semaphore creation passing
 * #U_SEMA_CREATE to u_sema_new(). The only difference is that you
 * should already know whether this semaphore object is from the previous crash
 * or not.
 */
U_API void
u_sema_take_ownership(sema_t *sem);

/*!@brief Acquires (P operation) a semaphore.
 * @param sem #sema_t to acquire.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_sema_acquire(sema_t *sem, err_t **error);

/*!@brief Releases (V operation) a semaphore.
 * @param sem #sema_t to release.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_sema_release(sema_t *sem, err_t **error);

/*!@brief Frees #sema_t object.
 * @param sem #sema_t to free.
 * @since 0.0.1
 *
 * It doesn't release an acquired semaphore, be careful to not to make a
 * deadlock while removing the acquired semaphore.
 */
U_API void
u_sema_free(sema_t *sem);

#endif /* !U_SEM_H__ */
