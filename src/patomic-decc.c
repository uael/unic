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

#ifdef P_OS_VMS
#  include <builtins.h>
#else
#  include <machine/builtins.h>
#endif

#ifdef __ia64
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)	\
    __CMP_SWAP_LONG ((volatile void *) (atomic_src),		\
         (int_t) (oldval),				\
         (int_t) (newval))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)	\
    __CMP_SWAP_QUAD ((volatile void *) (atomic_src),		\
         (int64_t) (oldval),				\
         (int64_t) (newval))
#else
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)  \
    __CMP_STORE_LONG ((volatile void *) (atomic_src),    \
          (int_t) (oldval),        \
          (int_t) (newval),        \
          (volatile void *) (atomic_dst))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)  \
    __CMP_STORE_QUAD ((volatile void *) (atomic_src),    \
          (int64_t) (oldval),        \
          (int64_t) (newval),        \
          (volatile void *) (atomic_dst))
#endif

P_API int_t
p_atomic_int_get(const volatile int_t *atomic) {
  __MB();
  return (int_t) *atomic;
}

P_API void
p_atomic_int_set(volatile int_t *atomic,
  int_t val) {
  (void) __ATOMIC_EXCH_LONG((volatile void *) atomic, val);
  __MB();
}

P_API void
p_atomic_int_inc(volatile int_t *atomic) {
  __MB();
  (void) __ATOMIC_INCREMENT_LONG((volatile void *) atomic);
  __MB();
}

P_API bool
p_atomic_int_dec_and_test(volatile int_t *atomic) {
  bool result;

  __MB();
  result =
    __ATOMIC_DECREMENT_LONG((volatile void *) atomic) == 1 ? true : false;
  __MB();

  return result;
}

P_API bool
p_atomic_int_compare_and_exchange(volatile int_t *atomic,
  int_t oldval,
  int_t newval) {
  bool result;

  __MB();
  result =
    PATOMIC_DECC_CAS_LONG (atomic, oldval, newval, atomic) == 1 ? true : false;
  __MB();

  return result;
}

P_API int_t
p_atomic_int_add(volatile int_t *atomic,
  int_t val) {
  int_t result;

  __MB();
  result = __ATOMIC_ADD_LONG((volatile void *) atomic, val);
  __MB();

  return result;
}

P_API uint_t
p_atomic_int_and(volatile uint_t *atomic,
  uint_t val) {
  uint_t result;

  __MB();
  result = (uint_t) __ATOMIC_AND_LONG((volatile void *) atomic, (int_t) val);
  __MB();

  return result;
}

P_API uint_t
p_atomic_int_or(volatile uint_t *atomic,
  uint_t val) {
  uint_t result;

  __MB();
  result = (uint_t) __ATOMIC_OR_LONG((volatile void *) atomic, (int_t) val);
  __MB();

  return result;
}

P_API uint_t
p_atomic_int_xor(volatile uint_t *atomic,
  uint_t val) {
  int_t i;

  do {
    __MB();
    i = (int_t) (*atomic);
  } while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((int_t) val), atomic) != 1);

  __MB();

  return i;
}

P_API ptr_t
p_atomic_pointer_get(const volatile void *atomic) {
  __MB();
  return (ptr_t) (*((const volatile size_t *) atomic));
}

P_API void
p_atomic_pointer_set(volatile void *atomic,
  ptr_t val) {
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  (void) __ATOMIC_EXCH_QUAD(atomic, (int64_t) val);
#else
  (void) __ATOMIC_EXCH_LONG (atomic, (int_t) val);
#endif
  __MB();
}

P_API bool
p_atomic_pointer_compare_and_exchange(volatile void *atomic,
  ptr_t oldval,
  ptr_t newval) {
  bool result;

  __MB();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  result =
    PATOMIC_DECC_CAS_QUAD (atomic, oldval, newval, atomic) == 1 ? true : false;
#else
  result = PATOMIC_DECC_CAS_LONG (atomic, oldval, newval, atomic) == 1 ? true : false;
#endif
  __MB();

  return result;
}

P_API ssize_t
p_atomic_pointer_add(volatile void *atomic,
  ssize_t val) {
  ssize_t result;

  __MB();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  result = (ssize_t) __ATOMIC_ADD_QUAD(atomic, (int64_t) val);
#else
  result = (ssize_t) __ATOMIC_ADD_LONG (atomic, (int_t) val);
#endif
  __MB();

  return result;
}

P_API size_t
p_atomic_pointer_and(volatile void *atomic,
  size_t val) {
  size_t result;

  __MB();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  result = (size_t) __ATOMIC_AND_QUAD(atomic, (int64_t) val);
#else
  result = (size_t) __ATOMIC_AND_LONG (atomic, (int_t) val);
#endif
  __MB();

  return result;
}

P_API size_t
p_atomic_pointer_or(volatile void *atomic,
  size_t val) {
  size_t result;

  __MB();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  result = (size_t) __ATOMIC_OR_QUAD(atomic, (int64_t) val);
#else
  result = (size_t) __ATOMIC_OR_LONG (atomic, (int_t) val);
#endif
  __MB();

  return result;
}

P_API size_t
p_atomic_pointer_xor(volatile void *atomic,
  size_t val) {
#if (PLIBSYS_SIZEOF_VOID_P == 8)
  int64_t i;

  do {
    __MB();
    i = (int64_t) (*((volatile size_t *) atomic));
  } while (PATOMIC_DECC_CAS_QUAD (atomic, i, i ^ ((int64_t) val), atomic) != 1);
#else
  int_t i;

  do {
    __MB ();
    i = (int_t) (* ((volatile size_t *) atomic));
  } while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((int_t) val), atomic) != 1);
#endif
  __MB();

  return (size_t) i;
}

P_API bool
p_atomic_is_lock_free(void) {
  return true;
}

void
p_atomic_thread_init(void) {
}

void
p_atomic_thread_shutdown(void) {
}
