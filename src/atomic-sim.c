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

#include "unic/atomic.h"
#include "unic/mutex.h"

/* We have to use the slow, but safe locking method. */
static mutex_t *pp_atomic_mutex = NULL;

int
u_atomic_int_get(const volatile int *atomic) {
  int value;

  u_mutex_lock(pp_atomic_mutex);
  value = *atomic;
  u_mutex_unlock(pp_atomic_mutex);
  return value;
}

void
u_atomic_int_set(volatile int *atomic, int val) {

  u_mutex_lock(pp_atomic_mutex);
  *atomic = val;
  u_mutex_unlock(pp_atomic_mutex);
}

void
u_atomic_int_inc(volatile int *atomic) {
  u_mutex_lock(pp_atomic_mutex);
  (*atomic)++;
  u_mutex_unlock(pp_atomic_mutex);
}

bool
u_atomic_int_dec_and_test(volatile int *atomic) {
  bool is_zero;

  u_mutex_lock(pp_atomic_mutex);
  is_zero = --(*atomic) == 0;
  u_mutex_unlock(pp_atomic_mutex);
  return is_zero;
}

bool
u_atomic_int_compare_and_exchange(volatile int *atomic, int oldval,
  int newval) {
  bool success;

  u_mutex_lock(pp_atomic_mutex);
  if ((success = (*atomic == oldval))) {
    *atomic = newval;
  }
  u_mutex_unlock(pp_atomic_mutex);
  return success;
}

int
u_atomic_int_add(volatile int *atomic, int val) {
  int oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval + val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

uint_t
u_atomic_int_and(volatile uint_t *atomic, uint_t val) {
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval & val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

uint_t
u_atomic_int_or(volatile uint_t *atomic, uint_t val) {
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval | val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

uint_t
u_atomic_int_xor(volatile uint_t *atomic, uint_t val) {
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval ^ val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

ptr_t
u_atomic_pointer_get(const volatile void *atomic) {
  const volatile ptr_t *ptr = atomic;
  ptr_t value;

  u_mutex_lock(pp_atomic_mutex);
  value = *ptr;
  u_mutex_unlock(pp_atomic_mutex);
  return value;
}

void
u_atomic_pointer_set(volatile void *atomic, ptr_t val) {
  volatile ptr_t *ptr = atomic;

  u_mutex_lock(pp_atomic_mutex);
  *ptr = val;
  u_mutex_unlock(pp_atomic_mutex);
}

bool
u_atomic_pointer_compare_and_exchange(volatile void *atomic, ptr_t oldval,
  ptr_t newval) {
  volatile ptr_t *ptr = atomic;
  bool success;

  u_mutex_lock(pp_atomic_mutex);
  if ((success = (*ptr == oldval))) {
    *ptr = newval;
  }
  u_mutex_unlock(pp_atomic_mutex);
  return success;
}

ssize_t
u_atomic_pointer_add(volatile void *atomic, ssize_t val) {
  volatile ssize_t *ptr = atomic;
  ssize_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval + val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

size_t
u_atomic_pointer_and(volatile void *atomic, size_t val) {
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval & val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

size_t
u_atomic_pointer_or(volatile void *atomic, size_t val) {
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval | val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

size_t
u_atomic_pointer_xor(volatile void *atomic, size_t val) {
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval ^ val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
}

bool
u_atomic_is_lock_free(void) {
  return false;
}

void
u_atomic_thread_init(void) {
  if (U_LIKELY (pp_atomic_mutex == NULL)) {
    pp_atomic_mutex = u_mutex_new();
  }
}

void
u_atomic_thread_shutdown(void) {
  if (U_LIKELY (pp_atomic_mutex != NULL)) {
    u_mutex_free(pp_atomic_mutex);
    pp_atomic_mutex = NULL;
  }
}
