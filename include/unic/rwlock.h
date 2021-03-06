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

/*!@file unic/rwlock.h
 * @brief Read-write lock
 * @author Alexander Saprykin
 *
 * A read-write lock is a synchronization primitive which allows access to a
 * critical section to not only the one thread, but instead it splits all
 * threads into the two groups:
 * - reader threads which perform only the reading operations without any shared
 * data modifications;
 * - writer threads which may perform the shared data modifitcations as well as
 * its reading.
 *
 * When there are only the reader threads inside a critical section it is called
 * a shared lock - actually you do not need any locking mechanism and all the
 * threads share the lock. In this situation an arbitrary number of reader
 * threads can perform shared data reading.
 *
 * The situation changes when a writer thread requests access to the same
 * critical section. It will wait until all the current readers finish
 * executing the critical section before acquiring the lock in exclusive manner:
 * no one else can access the critical section until the writer finishes with
 * it. Even another writer thread will have to wait for the lock to be released
 * by the first writer before entering the critical section.
 *
 * To prevent writer startvation usually writers are in favor over readers,
 * which is actually implementation dependent, though most operating systems try
 * to follow this rule.
 *
 * A read-write lock is usually used when the writing operations are not
 * performed too frequently, or when the number of reader threads is a way more
 * than writer ones.
 *
 * A reader enters a critical section with u_rwlock_reader_lock() or
 * u_rwlock_reader_trylock() and exits with u_rwlock_reader_unlock().
 *
 * A writer enters the critical section with u_rwlock_writer_lock() or
 * u_rwlock_writer_trylock() and exits with u_rwlock_writer_unlock().
 */
#ifndef U_RWLOCK_H__
# define U_RWLOCK_H__

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Read-write lock opaque data structure. */
typedef struct rwlock rwlock_t;

/*!@brief Creates a new #rwlock_t object.
 * @return Pointer to a newly created #rwlock_t object.
 * @since 0.0.1
 */
U_API rwlock_t *
u_rwlock_new(void);

/*!@brief Locks a read-write lock for reading.
 * @param lock #rwlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock a read-write lock recursively - it may lead to an
 * application deadlock (implementation dependent).
 *
 * Forces the calling thread to sleep until the @a lock becomes available for
 * locking.
 */
U_API bool
u_rwlock_reader_lock(rwlock_t *lock);

/*!@brief Tries to lock a read-write lock for reading immediately.
 * @param lock #rwlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock a read-write lock recursively - it may lead to an
 * application deadlock (implementation dependent).
 *
 * Tries to lock the @a lock and returns immediately if it is not available for
 * locking.
 */
U_API bool
u_rwlock_reader_trylock(rwlock_t *lock);

/*!@brief Releases a locked for reading read-write lock.
 * @param lock #rwlock_t to release.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not use this function on non-locked read-write locks - behavior
 * may be unpredictable.
 * @warning Do not use this function to unlock a read-write lock which was
 * locked for writing.
 *
 * If the @a lock was previously locked for reading then it becomes unlocked.
 *
 * It's implementation dependent whether only the same thread can lock and
 * unlock the same read-write lock.
 */
U_API bool
u_rwlock_reader_unlock(rwlock_t *lock);

/*!@brief Locks a read-write lock for writing.
 * @param lock #rwlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock a read-write lock recursively - it may lead to an
 * application deadlock (implementation dependent).
 *
 * Forces the calling thread to sleep until the @a lock becomes available for
 * locking.
 */
U_API bool
u_rwlock_writer_lock(rwlock_t *lock);

/*!@brief Tries to lock a read-write lock immediately.
 * @param lock #rwlock_t to lock.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not lock a read-write lock recursively - it may lead to an
 * application deadlock (implementation dependent).
 *
 * Tries to lock the @a lock and returns immediately if it is not available for
 * locking.
 */
U_API bool
u_rwlock_writer_trylock(rwlock_t *lock);

/*!@brief Releases a locked for writing read-write lock.
 * @param lock #rwlock_t to release.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @warning Do not use this function on non-locked read-write locks - behavior
 * may be unpredictable.
 * @warning Do not use this function to unlock a read-write lock which was
 * locked for reading.
 *
 * If the @a lock was previously locked for writing then it becomes unlocked.
 *
 * It's implementation dependent whether only the same thread can lock and
 * unlock the same read-write lock.
 */
U_API bool
u_rwlock_writer_unlock(rwlock_t *lock);

/*!@brief Frees a #rwlock_t object.
 * @param lock #rwlock_t to free.
 * @since 0.0.1
 * @warning It doesn't unlock the @a lock before freeing memory, so you should
 * do it manually.
 */
U_API void
u_rwlock_free(rwlock_t *lock);

#endif /* !U_RWLOCK_H__ */
