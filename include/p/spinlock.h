/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file p/spinlock.h
 * @brief Light-weight atomic spinlock
 * @author Alexander Saprykin
 *
 * A spinlock is an inter-thread synchronization primitive based on atomic
 * operations. It allows to guard a critical section from concurrent access of
 * multiple threads at once. It is very similar to a mutex in semantics, but
 * inside it provides a more light-weight and fast locking mechanism without
 * thread sleeping and undesirable context switching. Thus spinlocks should be
 * used only for small code sections, otherwise long-time spinning can cause
 * extensive CPU time waste by waiting threads.
 *
 * As the spinlock is based on atomic operations it would have the real meaning
 * only if an underlying atomic model is lock-free (not simulated using the
 * mutex). You can check if the atomic model is lock-free with
 * p_atomic_is_lock_free(). Otherwise usage of spinlocks will be the same as the
 * ordinary mutex.
 *
 * To create a new spinlock primitive the p_spinlock_new() routine should be
 * called, to delete the unused spinlock primitive use p_spinlock_free().
 *
 * Use p_spinlock_lock() or p_spinlock_trylock() to synchronize access at the
 * beginning of the critical section. Only the one thread is allowed to pass
 * this call, others will wait for the p_spinlock_unlock() call which marks the
 * end of the critical section. This way the critical section code is guarded
 * against concurrent access of multiple threads at once.
 */
#ifndef P_SPINLOCK_H__
# define P_SPINLOCK_H__

#include "p/macros.h"
#include "p/types.h"

/*!@brief Spinlock opaque data structure. */
typedef struct spinlock spinlock_t;

/*!@brief Creates a new #spinlock_t object.
 * @return Pointer to a newly created #spinlock_t object.
 * @since 0.0.1
 */
P_API spinlock_t *
p_spinlock_new(void);

/*!@brief Locks a spinlock.
 * @param spinlock #spinlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 *
 * A thread will not sleep in this call if another thread is holding the lock,
 * instead it will try to lock @a spinlock in an infinite loop.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_lock().
 *
 * Do not lock a spinlock recursively - this may lead to an application
 * deadlock.
 */
P_API bool
p_spinlock_lock(spinlock_t *spinlock);

/*!@brief Tries to lock a spinlock immediately.
 * @param spinlock #spinlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 *
 * Tries to lock @a spinlock and returns immediately if it is not available for
 * locking.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_trylock().
 *
 * Do not lock a spinlock recursively - this may lead to an application
 * deadlock.
 */
P_API bool
p_spinlock_trylock(spinlock_t *spinlock);

/*!@brief Releases a locked spinlock.
 * @param spinlock #spinlock_t to release.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 *
 * If @a spinlock was previously locked then it becomes unlocked. Any thread
 * can unlock any spinlock. It is also safe to call this routine on an unlocked
 * spinlock.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_unlock(), thus it is not safe to call this routine on an unlocked
 * spinlock.
 */
P_API bool
p_spinlock_unlock(spinlock_t *spinlock);

/*!@brief Frees #spinlock_t object.
 * @param spinlock #spinlock_t to free.
 * @since 0.0.1
 *
 * It doesn't unlock @a spinlock before freeing memory, so you should do it
 * manually.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_free().
 */
P_API void
p_spinlock_free(spinlock_t *spinlock);

#endif /* !P_SPINLOCK_H__ */
