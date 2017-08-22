/*
 * Copyright (C) 2011 Ryan Lortie <desrt@desrt.ca>
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file p/atomic.h
 * @brief Atomic operations
 * @author Alexander Saprykin
 *
 * Atomic operations can be used to avoid heavy thread synchronization
 * primitives such as mutexes, semaphores and so on. These operations are
 * performed atomically and can't be preempted by another thread.
 *
 * Lock-free atomic operations require software and hardware support. Usually
 * lock-free atomic operations are implemented with low-level using assembly
 * inlines. Some of the compilers provide built-in routines to perform atomic
 * operations. You can use the p_atomic_is_lock_free() call to check whether
 * such a support is provided or not.
 *
 * If there is no hardware or software support for lock-free atomic operations
 * then they can be simulated (though in rather slower manner) using a thread
 * global synchronization primitive (i.e. mutex), but it could block threads
 * while performing atomic operations on distinct variables from distinct
 * threads.
 *
 * The Windows platform provides all the required lock-free operations in most
 * cases, so it always has lock-free support.
 */
#ifndef P_ATOMIC_H__
# define P_ATOMIC_H__

#include "p/types.h"
#include "p/macros.h"

/*!@brief Gets #int_t value from @a atomic.
 * @param atomic Pointer to #int_t to get the value from.
 * @return Integer value.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (before the
 * get).
 */
P_API int_t
p_atomic_int_get(const volatile int_t *atomic);

/*!@brief Sets #int_t value to @a atomic.
 * @param[out] atomic Pointer to #int_t to set the value for.
 * @param val New #int_t value.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (after the
 * set).
 */
P_API void
p_atomic_int_set(volatile int_t *atomic, int_t val);

/*!@brief Increments #int_t value from @a atomic by 1.
 * @param[in,out] atomic Pointer to #int_t to increment the value.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of `{ *atomic += 1; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API void
p_atomic_int_inc(volatile int_t *atomic);

/*!@brief Decrements #int_t value from @a atomic by 1 and tests the result for
 * zero.
 * @param[in,out] atomic Pointer to #int_t to decrement the value.
 * @return true if the new value is equal to zero, false otherwise.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ *atomic -= 1; return (*atomic == 0); }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API bool
p_atomic_int_dec_and_test(volatile int_t *atomic);

/*!@brief Compares @a oldval with the value pointed to by @a atomic and if
 * they are equal, atomically exchanges the value of @a atomic with @a newval.
 * @param[in,out] atomic Pointer to #int_t.
 * @param oldval Old #int_t value.
 * @param newval New #int_t value.
 * @return true if @a atomic value was equal @a oldval, false otherwise.
 * @since 0.0.1
 *
 * This compare and exchange is done atomically.
 *
 * Think of this operation as an atomic version of
 * `{ if (*atomic == oldval) *atomic = newval; return true;
 *    else return false; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API bool
p_atomic_int_compare_and_exchange(volatile int_t *atomic, int_t oldval,
  int_t newval);

/*!@brief Atomically adds #int_t value to @a atomic value.
 * @param[in,out] atomic Pointer to #int_t.
 * @param val Integer to add to @a atomic value.
 * @return Old value before the addition.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic += val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API int_t
p_atomic_int_add(volatile int_t *atomic, int_t val);

/*!@brief Atomically performs the bitwise 'and' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #uint_t.
 * @param val #uint_t to perform bitwise 'and' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier.
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic &= val; return tmp; }`.
 */
P_API uint_t
p_atomic_int_and(volatile uint_t *atomic, uint_t val);

/*!@brief Atomically performs the bitwise 'or' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #uint_t.
 * @param val #uint_t to perform bitwise 'or' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic |= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API uint_t
p_atomic_int_or(volatile uint_t *atomic, uint_t val);

/*!@brief Atomically performs the bitwise 'xor' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #uint_t.
 * @param val #uint_t to perform bitwise 'xor' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic ^= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API uint_t
p_atomic_int_xor(volatile uint_t *atomic, uint_t val);

/*!@brief Gets #ptr_t value from @a atomic.
 * @param atomic Pointer to get the value from.
 * @return Value from the pointer.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier
 * (before the get).
 */
P_API ptr_t
p_atomic_pointer_get(const volatile void *atomic);

/*!@brief Sets @a val to #ptr_t @a atomic.
 * @param[out] atomic Pointer to set the value for.
 * @param val New value for @a atomic.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier
 * (after the set).
 */
P_API void
p_atomic_pointer_set(volatile void *atomic, ptr_t val);

/*!@brief Compares @a oldval with the value pointed to by @a atomic and if
 * they are equal, atomically exchanges the value of @a atomic with @a newval.
 * @param[in,out] atomic Pointer to #ptr_t value.
 * @param oldval Old #ptr_t value.
 * @param newval New #ptr_t value.
 * @return true if @a atomic value was equal @a oldval, false otherwise.
 * @since 0.0.1
 *
 * This compare and exchange is done atomically.
 *
 * Think of this operation as an atomic version of
 * `{ if (*atomic == oldval) *atomic = newval; return true;
 *    else return false; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API bool
p_atomic_pointer_compare_and_exchange(volatile void *atomic, ptr_t oldval,
  ptr_t newval);

/*!@brief Atomically adds #ptr_t value to @a atomic value.
 * @param[in,out] atomic Pointer to #ptr_t value.
 * @param val Value to add to @a atomic value.
 * @return Old value before the addition.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic += val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API ssize_t
p_atomic_pointer_add(volatile void *atomic, ssize_t val);

/*!@brief Atomically performs the bitwise 'and' operation of #ptr_t
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ptr_t value.
 * @param val #size_t to perform bitwise 'and' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic &= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API size_t
p_atomic_pointer_and(volatile void *atomic, size_t val);

/*!@brief Atomically performs the bitwise 'or' operation of #ptr_t
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ptr_t value.
 * @param val #size_t to perform bitwise 'or' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic |= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API size_t
p_atomic_pointer_or(volatile void *atomic, size_t val);

/*!@brief Atomically performs the bitwise 'xor' operation of #ptr_t
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ptr_t value.
 * @param val #size_t to perform bitwise 'xor' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic ^= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_API size_t
p_atomic_pointer_xor(volatile void *atomic, size_t val);

/*!@brief Checks whether atomic operations are lock-free.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 *
 * Some underlying atomic model implementations may not support lock-free
 * operations depending on hardware or software.
 */
P_API bool
p_atomic_is_lock_free(void);

#endif /* !P_ATOMIC_H__ */
