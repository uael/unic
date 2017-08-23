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

#include "p/atomic.h"

#ifdef P_CC_SUN
# define P_ATOMIC_INT_CAST(x) ((int *) (x))
# define P_ATOMIC_SIZE_CAST(x) ((size_t *) (x))
#else
# define P_ATOMIC_INT_CAST(x) x
# define P_ATOMIC_SIZE_CAST(x) x
#endif

int
p_atomic_int_get(const volatile int *atomic) {
  return (int) __atomic_load_4(P_ATOMIC_INT_CAST(atomic), __ATOMIC_SEQ_CST);
}

void
p_atomic_int_set(volatile int *atomic, int val) {
  __atomic_store_4(P_ATOMIC_INT_CAST(atomic), val, __ATOMIC_SEQ_CST);
}

void
p_atomic_int_inc(volatile int *atomic) {
  (void) __atomic_fetch_add(atomic, 1, __ATOMIC_SEQ_CST);
}

bool
p_atomic_int_dec_and_test(volatile int *atomic) {
  return (__atomic_fetch_sub(atomic, 1, __ATOMIC_SEQ_CST) == 1) ? true : false;
}

bool
p_atomic_int_compare_and_exchange(volatile int *atomic, int oldval,
  int newval) {
  int tmp_int = oldval;

  return (bool) __atomic_compare_exchange_n(
    P_ATOMIC_INT_CAST(atomic),
    &tmp_int,
    newval,
    0,
    __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST
  );
}

int
p_atomic_int_add(volatile int *atomic, int val) {
  return (int) __atomic_fetch_add(atomic, val, __ATOMIC_SEQ_CST);
}

uint_t
p_atomic_int_and(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __atomic_fetch_and(atomic, val, __ATOMIC_SEQ_CST);
}

uint_t
p_atomic_int_or(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __atomic_fetch_or(atomic, val, __ATOMIC_SEQ_CST);
}

uint_t
p_atomic_int_xor(volatile uint_t *atomic, uint_t val) {
  return (uint_t) __atomic_fetch_xor(atomic, val, __ATOMIC_SEQ_CST);
}

ptr_t
p_atomic_pointer_get(const volatile void *atomic) {
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  return (ptr_t) __atomic_load_8(
    P_ATOMIC_SIZE_CAST((const volatile size_t *) atomic), __ATOMIC_SEQ_CST
  );
#else
  return (ptr_t) __atomic_load_4(
    P_ATOMIC_SIZE_CAST((const volatile size_t *) atomic), __ATOMIC_SEQ_CST
  );
#endif
}

void
p_atomic_pointer_set(volatile void *atomic, ptr_t val) {
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  __atomic_store_8(
    P_ATOMIC_SIZE_CAST((volatile size_t *) atomic), (size_t) val,
    __ATOMIC_SEQ_CST
  );
#else
  __atomic_store_4(
    P_ATOMIC_SIZE_CAST((volatile size_t *) atomic), (size_t) val,
    __ATOMIC_SEQ_CST
  );
#endif
}

bool
p_atomic_pointer_compare_and_exchange(volatile void *atomic, ptr_t oldval,
  ptr_t newval) {
  ptr_t tmp_pointer = oldval;

  return (bool) __atomic_compare_exchange_n(
    P_ATOMIC_SIZE_CAST((volatile size_t *) atomic),
    (size_t *) &tmp_pointer,
    PPOINTER_TO_PSIZE (newval),
    0,
    __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST
  );
}

ssize_t
p_atomic_pointer_add(volatile void *atomic, ssize_t val) {
  return (ssize_t) __atomic_fetch_add(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
}

size_t
p_atomic_pointer_and(volatile void *atomic, size_t val) {
  return (size_t) __atomic_fetch_and(
    (volatile size_t *) atomic, val, __ATOMIC_SEQ_CST
  );
}

size_t
p_atomic_pointer_or(volatile void *atomic, size_t val) {
  return (size_t) __atomic_fetch_or(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
}

size_t
p_atomic_pointer_xor(volatile void *atomic, size_t val) {
  return (size_t) __atomic_fetch_xor(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
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
