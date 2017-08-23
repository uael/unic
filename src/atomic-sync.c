/*
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

#include "p/atomic.h"

int
p_atomic_int_get(const volatile int *atomic) {
  __sync_synchronize();
  return *atomic;
}

void
p_atomic_int_set(volatile int *atomic, int val) {
  *atomic = val;
  __sync_synchronize();
}

void
p_atomic_int_inc(volatile int *atomic) {
  (void) __sync_fetch_and_add(atomic, 1);
}

bool
p_atomic_int_dec_and_test(volatile int *atomic) {
  return __sync_fetch_and_sub(atomic, 1) == 1 ? true : false;
}

bool
p_atomic_int_compare_and_exchange(volatile int *atomic, int oldval,
  int newval) {
  return (bool) __sync_bool_compare_and_swap(atomic, oldval, newval);
}

int
p_atomic_int_add(volatile int *atomic, int val) {
  return (int) __sync_fetch_and_add(atomic, val);
}

uint_t
p_atomic_int_and(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __sync_fetch_and_and(atomic, val);
}

uint_t
p_atomic_int_or(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __sync_fetch_and_or(atomic, val);
}

uint_t
p_atomic_int_xor(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __sync_fetch_and_xor(atomic, val);
}

ptr_t
p_atomic_pointer_get(const volatile void *atomic) {
  __sync_synchronize();
  return (ptr_t) *((const volatile size_t *) atomic);
}

void
p_atomic_pointer_set(volatile void *atomic, ptr_t val) {
  volatile size_t *cur_val = (volatile size_t *) atomic;

  *cur_val = (size_t) val;
  __sync_synchronize();
}

bool
p_atomic_pointer_compare_and_exchange(volatile void *atomic, ptr_t oldval,
  ptr_t newval) {
  return (bool) __sync_bool_compare_and_swap((volatile size_t *) atomic,
    (size_t) oldval,
    (size_t) newval
  );
}

ssize_t
p_atomic_pointer_add(volatile void *atomic, ssize_t val) {
  return (ssize_t) __sync_fetch_and_add((volatile ssize_t *) atomic, val);
}

size_t
p_atomic_pointer_and(volatile void *atomic, size_t val) {
  return (size_t) __sync_fetch_and_and((volatile size_t *) atomic, val);
}

size_t
p_atomic_pointer_or(volatile void *atomic, size_t val) {
  return (size_t) __sync_fetch_and_or((volatile size_t *) atomic, val);
}

size_t
p_atomic_pointer_xor(volatile void *atomic, size_t val) {
  return (size_t) __sync_fetch_and_xor((volatile size_t *) atomic, val);
}

bool
p_atomic_is_lock_free(void) {
  return true;
}

void
p_atomic_thread_init(void) {
}

void
p_atomic_thread_shutdown(void) {
}
