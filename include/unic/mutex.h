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

/*!@file unic/mutex.h
 * @brief Mutex routines
 * @author Alexander Saprykin
 *
 * A mutex is a mutual exclusive (hence mutex) synchronization primitive which
 * allows access to a critical section only to one of the concurrently running
 * threads. It is used to protected shared data structures from concurrent
 * modifications which could lead to unpredictable behavior.
 *
 * When entering a critical section a thread must call u_mutex_lock() to get a
 * lock. If another thread is already holding the lock all other threads will
 * be suspended until the lock is released with u_mutex_unlock(). After
 * releasing the lock one of the waiting threads is resumed to continue
 * execution. On most systems it is not specified whether a mutex waiting queue
 * is fair (FIFO) or not.
 *
 * The typical mutex usage:
 * @code
 * u_mutex_lock (mutex);
 *
 * ... code in critical section ...
 *
 * u_mutex_unlock (mutex);
 * @endcode
 * You can also think of the mutex as a binary semaphore.
 *
 * It is implementation dependent whether recursive locking or non-locked mutex
 * unlocking is allowed, but such actions can lead to unpredictable behavior.
 * Do not rely on such behavior in cross-platform applications.
 *
 * This is the thread scoped mutex implementation. You could not share this
 * mutex outside the process adress space, but you can share it between the
 * threads of the same process.
 */
#ifndef U_MUTEX_H__
# define U_MUTEX_H__

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Mutex opaque data structure. */
typedef struct mutex mutex_t;

/*!@brief Creates a new #mutex_t object.
 * @return Pointer to a newly created #mutex_t object.
 * @since 0.0.1
 */
U_API mutex_t *
u_mutex_new(void);

/*!@brief Locks a mutex.
 * @param mutex #mutex_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock the mutex recursively - it may lead to an application
 * deadlock (implementation dependent).
 *
 * Forces the calling thread to sleep until @a mutex becomes available for
 * locking.
 */
U_API bool
u_mutex_lock(mutex_t *mutex);

/*!@brief Tries to lock a mutex immediately.
 * @param mutex #mutex_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock the mutex recursively - it may lead to an application
 * deadlock (implementation dependent).
 *
 * Tries to lock @a mutex and returns immediately if it is not available for
 * locking.
 */
U_API bool
u_mutex_trylock(mutex_t *mutex);

/*!@brief Releases a locked mutex.
 * @param mutex #mutex_t to release.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not use this function on non-locked mutexes - behavior may be
 * unpredictable.
 *
 * If @a mutex was previously locked then it becomes unlocked.
 *
 * It's implementation dependent whether only the same thread can lock and
 * unlock the same mutex.
 */
U_API bool
u_mutex_unlock(mutex_t *mutex);

/*!@brief Frees #mutex_t object.
 * @param mutex #mutex_t to free.
 * @since 0.0.1
 * @warning It doesn't unlock @a mutex before freeing memory, so you should do
 * it manually.
 */
U_API void
u_mutex_free(mutex_t *mutex);

#endif /* !U_MUTEX_H__ */
