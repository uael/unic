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

#include "unic/atomic.h"
#ifdef UNIC_ATOMIC_SIM
# include "unic/mutex.h"
#endif

#if defined UNIC_ATOMIC_WIN
/* Prepare MemoryBarrier() */
# if defined (U_CC_WATCOM) || defined (U_CC_BORLAND)
#   if defined (_M_X64) || defined (_M_AMD64)
#     define MemoryBarrier __faststorefence
#   elseif defined (_M_IA64)
#     define MemoryBarrier __mf
#   else
#     ifdef U_CC_WATCOM
inline
#     else
FORCEINLINE
#     endif
VOID
MemoryBarrier(VOID) {
  LONG Barrier = 0;

  (void) (Barrier);
  __asm {
    xchg Barrier, eax
  }
}
#   endif
# endif

/* Tell compiler about intrinsics to suppress warnings,
 * see: https://msdn.microsoft.com/en-us/library/hh977023.aspx */
# if !defined (U_OS_WIN64) && (defined (U_CC_MSVC) && _MSC_VER > 1200)
#   define InterlockedAnd _InterlockedAnd
#   define InterlockedOr _InterlockedOr
#   define InterlockedXor _InterlockedXor
#   pragma intrinsic(_InterlockedAnd)
#   pragma intrinsic(_InterlockedOr)
#   pragma intrinsic(_InterlockedXor)
# endif

/* Inlined versions for older compilers */
#  if (defined (U_CC_MSVC) && _MSC_VER <= 1200) || defined (U_CC_WATCOM) \
      || defined (U_CC_BORLAND)
static LONG
ppInterlockedAnd(LONG volatile *atomic, LONG  val) {
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i & val, i);
  } while (i != j);

  return j;
}
#   define InterlockedAnd(a,b) ppInterlockedAnd(a,b)
static LONG
ppInterlockedOr(LONG volatile  *atomic, LONG  val) {
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i | val, i);
  } while (i != j);

  return j;
}
#   define InterlockedOr(a,b) ppInterlockedOr(a,b)
static LONG
ppInterlockedXor(LONG volatile *atomic, LONG  val) {
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i ^ val, i);
  } while (i != j);

  return j;
}
#   define InterlockedXor(a,b) ppInterlockedXor(a,b)
# endif
#elif defined UNIC_ATOMIC_DECC
# ifdef __ia64
#   define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst) \
  __CMP_SWAP_LONG ((volatile void *) (atomic_src), \
    (int) (oldval), \
    (int) (newval))
#   define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst) \
  __CMP_SWAP_QUAD ((volatile void *) (atomic_src), \
    (i64_t) (oldval), \
    (i64_t) (newval))
# else
#   define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst) \
  __CMP_STORE_LONG ((volatile void *) (atomic_src), \
    (int) (oldval), \
    (int) (newval), \
    (volatile void *) (atomic_dst))
#   define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst) \
  __CMP_STORE_QUAD ((volatile void *) (atomic_src), \
    (i64_t) (oldval), \
    (i64_t) (newval), \
    (volatile void *) (atomic_dst))
# endif
#endif

#ifdef U_CC_SUN
# define U_ATOMIC_INT_CAST(x) ((int *) (x))
# define U_ATOMIC_SIZE_CAST(x) ((size_t *) (x))
#else
# define U_ATOMIC_INT_CAST(x) x
# define U_ATOMIC_SIZE_CAST(x) x
#endif

int
u_atomic_int_get(const volatile int *atomic) {
#if defined UNIC_ATOMIC_C11
  return atomic_load(atomic);
#elif defined UNIC_ATOMIC_INTRIN
  return (int) __atomic_load_4(U_ATOMIC_INT_CAST(atomic), __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  __sync_synchronize();
  return *atomic;
#elif defined UNIC_ATOMIC_DECC
  __MB();
  return (int) *atomic;
#elif defined UNIC_ATOMIC_WIN
  MemoryBarrier();
  return *atomic;
#else
  int value;

  u_mutex_lock(pp_atomic_mutex);
  value = *atomic;
  u_mutex_unlock(pp_atomic_mutex);
  return value;
#endif
}

void
u_atomic_int_set(volatile int *atomic, int val) {
#if defined UNIC_ATOMIC_C11
  atomic_store(atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  __atomic_store_4(U_ATOMIC_INT_CAST(atomic), val, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  *atomic = val;
  __sync_synchronize();
#elif defined UNIC_ATOMIC_DECC
  (void) __ATOMIC_EXCH_LONG((volatile void *) atomic, val);
  __MB();
#elif defined UNIC_ATOMIC_WIN
  *atomic = val;
  MemoryBarrier();
#else
  u_mutex_lock(pp_atomic_mutex);
  *atomic = val;
  u_mutex_unlock(pp_atomic_mutex);
#endif
}

void
u_atomic_int_inc(volatile int *atomic) {
#if defined UNIC_ATOMIC_C11
  atomic_fetch_add(atomic, 1);
#elif defined UNIC_ATOMIC_INTRIN
  (void) __atomic_fetch_add(atomic, 1, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  (void) __sync_fetch_and_add(atomic, 1);
#elif defined UNIC_ATOMIC_DECC
  __MB();
  (void) __ATOMIC_INCREMENT_LONG((volatile void *) atomic);
  __MB();
#elif defined UNIC_ATOMIC_WIN
  InterlockedIncrement((LONG volatile *) atomic);
#else
  u_mutex_lock(pp_atomic_mutex);
  (*atomic)++;
  u_mutex_unlock(pp_atomic_mutex);
#endif
}

bool
u_atomic_int_dec_and_test(volatile int *atomic) {
#if defined UNIC_ATOMIC_C11
  return (atomic_fetch_sub(atomic, 1) == 1) ? true : false;
#elif defined UNIC_ATOMIC_INTRIN
  return (__atomic_fetch_sub(atomic, 1, __ATOMIC_SEQ_CST) == 1) ? true : false;
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return __sync_fetch_and_sub(atomic, 1) == 1 ? true : false;
#elif defined UNIC_ATOMIC_DECC
  bool result;

  __MB();
  result =
    __ATOMIC_DECREMENT_LONG((volatile void *) atomic) == 1 ? true : false;
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return InterlockedDecrement((LONG volatile *) atomic) == 0 ? true : false;
#else
  bool is_zero;

  u_mutex_lock(pp_atomic_mutex);
  is_zero = --(*atomic) == 0;
  u_mutex_unlock(pp_atomic_mutex);
  return is_zero;
#endif
}

bool
u_atomic_int_compare_and_exchange(volatile int *atomic, int oldval,
  int newval) {
#if defined UNIC_ATOMIC_C11
  int tmp_int = oldval;

  return (bool) atomic_compare_exchange_strong(atomic, &tmp_int, newval);
#elif defined UNIC_ATOMIC_INTRIN
  int tmp_int = oldval;

  return (bool) __atomic_compare_exchange_n(
    U_ATOMIC_INT_CAST(atomic),
    &tmp_int,
    newval,
    0,
    __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (bool) __sync_bool_compare_and_swap(atomic, oldval, newval);
#elif defined UNIC_ATOMIC_DECC
  bool result;

  __MB();
  result =
    PATOMIC_DECC_CAS_LONG (atomic, oldval, newval, atomic) == 1 ? true : false;
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return InterlockedCompareExchange((LONG volatile *) atomic,
    (LONG) newval,
    (LONG) oldval
  ) == oldval;
#else
  bool success;

  u_mutex_lock(pp_atomic_mutex);
  if ((success = (*atomic == oldval))) {
    *atomic = newval;
  }
  u_mutex_unlock(pp_atomic_mutex);
  return success;
#endif
}

int
u_atomic_int_add(volatile int *atomic, int val) {
#if defined UNIC_ATOMIC_C11
  return (int) atomic_fetch_add(atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (int) __atomic_fetch_add(atomic, val, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (int) __sync_fetch_and_add(atomic, val);
#elif defined UNIC_ATOMIC_DECC
  int result;

  __MB();
  result = __ATOMIC_ADD_LONG((volatile void *) atomic, val);
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return (int) InterlockedExchangeAdd((LONG volatile *) atomic, (LONG) val);
#else
  int oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval + val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

uint_t
u_atomic_int_and(volatile uint_t *atomic, uint_t val) {
#if defined UNIC_ATOMIC_C11
  return (uint_t) atomic_fetch_and(atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (uint_t) __atomic_fetch_and(atomic, val, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (uint_t) __sync_fetch_and_and(atomic, val);
#elif defined UNIC_ATOMIC_DECC
  uint_t result;

  __MB();
  result = (uint_t) __ATOMIC_AND_LONG((volatile void *) atomic, (int) val);
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return (uint_t) InterlockedAnd((LONG volatile *) atomic, (LONG) val);
#else
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval & val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

uint_t
u_atomic_int_or(volatile uint_t *atomic, uint_t val) {
#if defined UNIC_ATOMIC_C11
  return (uint_t) atomic_fetch_or(atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (uint_t) __atomic_fetch_or(atomic, val, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (uint_t) __sync_fetch_and_or(atomic, val);
#elif defined UNIC_ATOMIC_DECC
  uint_t result;

  __MB();
  result = (uint_t) __ATOMIC_OR_LONG((volatile void *) atomic, (int) val);
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return (uint_t) InterlockedOr((LONG volatile *) atomic, (LONG) val);
#else
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval | val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

uint_t
u_atomic_int_xor(volatile uint_t *atomic, uint_t val) {
#if defined UNIC_ATOMIC_C11
  return (uint_t) atomic_fetch_xor(atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (uint_t) __atomic_fetch_xor(atomic, val, __ATOMIC_SEQ_CST);
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (uint_t) __sync_fetch_and_xor(atomic, val);
#elif defined UNIC_ATOMIC_DECC
  int i;

  do {
    __MB();
    i = (int) (*atomic);
  } while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((int) val), atomic) != 1);
  __MB();
  return (uint_t) i;
#elif defined UNIC_ATOMIC_WIN
  return (uint_t) InterlockedXor((LONG volatile *) atomic, (LONG) val);
#else
  uint_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *atomic;
  *atomic = oldval ^ val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

ptr_t
u_atomic_pointer_get(const volatile void *atomic) {
#if defined UNIC_ATOMIC_C11
  return (ptr_t) atomic_load(
    U_ATOMIC_SIZE_CAST((const volatile size_t *) atomic)
  );
#elif defined UNIC_ATOMIC_INTRIN
# if (UNIC_SIZEOF_VOID_P == 8)
  return (ptr_t) __atomic_load_8(
    U_ATOMIC_SIZE_CAST((const volatile size_t *) atomic), __ATOMIC_SEQ_CST
  );
# else
  return (ptr_t) __atomic_load_4(
    U_ATOMIC_SIZE_CAST((const volatile size_t *) atomic), __ATOMIC_SEQ_CST
  );
# endif
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  __sync_synchronize();
  return (ptr_t) *((const volatile size_t *) atomic);
#elif defined UNIC_ATOMIC_DECC
  __MB();
  return (ptr_t) (*((const volatile size_t *) atomic));
#elif defined UNIC_ATOMIC_WIN
  const volatile ptr_t *ptr = (const volatile ptr_t *) atomic;
  MemoryBarrier();
  return *ptr;
#else
  const volatile ptr_t *ptr = atomic;
  ptr_t value;

  u_mutex_lock(pp_atomic_mutex);
  value = *ptr;
  u_mutex_unlock(pp_atomic_mutex);
  return value;
#endif
}

void
u_atomic_pointer_set(volatile void *atomic, ptr_t val) {
#if defined UNIC_ATOMIC_C11
  atomic_store(
    U_ATOMIC_SIZE_CAST((volatile size_t *) atomic), (size_t) val
  );
#elif defined UNIC_ATOMIC_INTRIN
# if (UNIC_SIZEOF_VOID_P == 8)
  __atomic_store_8(
    U_ATOMIC_SIZE_CAST((volatile size_t *) atomic), (size_t) val,
    __ATOMIC_SEQ_CST
  );
# else
  __atomic_store_4(
    U_ATOMIC_SIZE_CAST((volatile size_t *) atomic), (size_t) val,
    __ATOMIC_SEQ_CST
  );
# endif
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  volatile size_t *cur_val = (volatile size_t *) atomic;

  *cur_val = (size_t) val;
  __sync_synchronize();
#elif defined UNIC_ATOMIC_DECC
# if (UNIC_SIZEOF_VOID_P == 8)
  (void) __ATOMIC_EXCH_QUAD(atomic, (i64_t) val);
# else
  (void) __ATOMIC_EXCH_LONG(atomic, (int) val);
# endif
  __MB();
#elif defined UNIC_ATOMIC_WIN
  volatile ptr_t *ptr = (volatile ptr_t *) atomic;
  *ptr = val;
  MemoryBarrier();
#else
  volatile ptr_t *ptr = atomic;

  u_mutex_lock(pp_atomic_mutex);
  *ptr = val;
  u_mutex_unlock(pp_atomic_mutex);
#endif
}

bool
u_atomic_pointer_compare_and_exchange(volatile void *atomic, ptr_t oldval,
  ptr_t newval) {
#if defined UNIC_ATOMIC_C11
  ptr_t tmp_pointer = oldval;

  return (bool) atomic_compare_exchange_strong(
    (volatile size_t *) atomic, (size_t *) &tmp_pointer, (size_t) newval
  );
#elif defined UNIC_ATOMIC_INTRIN
  ptr_t tmp_pointer = oldval;

  return (bool) __atomic_compare_exchange_n(
    U_ATOMIC_SIZE_CAST((volatile size_t *) atomic),
    (size_t *) &tmp_pointer,
    PPOINTER_TO_PSIZE (newval),
    0,
    __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (bool) __sync_bool_compare_and_swap((volatile size_t *) atomic,
    (size_t) oldval,
    (size_t) newval
  );
#elif defined UNIC_ATOMIC_DECC
  bool result;

  __MB();
# if (UNIC_SIZEOF_VOID_P == 8)
  result =
    PATOMIC_DECC_CAS_QUAD(atomic, oldval, newval, atomic) == 1 ? true : false;
# else
  result =
    PATOMIC_DECC_CAS_LONG(atomic, oldval, newval, atomic) == 1 ? true : false;
# endif
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
  return InterlockedCompareExchangePointer((volatile PVOID *) atomic,
    (PVOID) newval,
    (PVOID) oldval) == oldval ? true : false;
#else
  volatile ptr_t *ptr = atomic;
  bool success;

  u_mutex_lock(pp_atomic_mutex);
  if ((success = (*ptr == oldval))) {
    *ptr = newval;
  }
  u_mutex_unlock(pp_atomic_mutex);
  return success;
#endif
}

ssize_t
u_atomic_pointer_add(volatile void *atomic, ssize_t val) {
#if defined UNIC_ATOMIC_C11
  return (ssize_t) atomic_fetch_add((volatile ssize_t *) atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (ssize_t) __atomic_fetch_add(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (ssize_t) __sync_fetch_and_add((volatile ssize_t *) atomic, val);
#elif defined UNIC_ATOMIC_DECC
  ssize_t result;

  __MB();
# if (UNIC_SIZEOF_VOID_P == 8)
  result = (ssize_t) __ATOMIC_ADD_QUAD(atomic, (i64_t) val);
# else
  result = (ssize_t) __ATOMIC_ADD_LONG(atomic, (int) val);
# endif
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
# if UNIC_SIZEOF_VOID_P == 8
  return (ssize_t) InterlockedExchangeAdd64((LONGLONG volatile *) atomic,
    (LONGLONG) val);
# else
  return (ssize_t) InterlockedExchangeAdd((LONG volatile *) atomic, (LONG) val);
# endif
#else
  volatile ssize_t *ptr = atomic;
  ssize_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval + val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

size_t
u_atomic_pointer_and(volatile void *atomic, size_t val) {
#if defined UNIC_ATOMIC_C11
  return (size_t) atomic_fetch_and((volatile size_t *) atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (size_t) __atomic_fetch_and(
    (volatile size_t *) atomic, val, __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (size_t) __sync_fetch_and_and((volatile size_t *) atomic, val);
#elif defined UNIC_ATOMIC_DECC
  size_t result;

  __MB();
# if (UNIC_SIZEOF_VOID_P == 8)
  result = (size_t) __ATOMIC_AND_QUAD(atomic, (i64_t) val);
# else
  result = (size_t) __ATOMIC_AND_LONG(atomic, (int) val);
# endif
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
# if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedAnd64((LONGLONG volatile *) atomic, (LONGLONG) val);
# else
  return (size_t) InterlockedAnd((LONG volatile *) atomic, (LONG) val);
# endif
#else
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval & val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

size_t
u_atomic_pointer_or(volatile void *atomic, size_t val) {
#if defined UNIC_ATOMIC_C11
  return (size_t) atomic_fetch_or((volatile ssize_t *) atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (size_t) __atomic_fetch_or(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (size_t) __sync_fetch_and_or((volatile size_t *) atomic, val);
#elif defined UNIC_ATOMIC_DECC
  size_t result;

  __MB();
# if (UNIC_SIZEOF_VOID_P == 8)
  result = (size_t) __ATOMIC_OR_QUAD(atomic, (i64_t) val);
# else
  result = (size_t) __ATOMIC_OR_LONG(atomic, (int) val);
# endif
  __MB();
  return result;
#elif defined UNIC_ATOMIC_WIN
# if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedOr64((LONGLONG volatile *) atomic, (LONGLONG) val);
# else
  return (size_t) InterlockedOr((LONG volatile *) atomic, (LONG) val);
# endif
#else
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval | val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

size_t
u_atomic_pointer_xor(volatile void *atomic, size_t val) {
#if defined UNIC_ATOMIC_C11
  return (size_t) atomic_fetch_xor((volatile ssize_t *) atomic, val);
#elif defined UNIC_ATOMIC_INTRIN
  return (size_t) __atomic_fetch_xor(
    (volatile ssize_t *) atomic, val, __ATOMIC_SEQ_CST
  );
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return (size_t) __sync_fetch_and_xor((volatile size_t *) atomic, val);
#elif defined UNIC_ATOMIC_DECC
# if (UNIC_SIZEOF_VOID_P == 8)
  i64_t i;

  do {
    __MB();
    i = (i64_t) (*((volatile size_t *) atomic));
  } while (PATOMIC_DECC_CAS_QUAD (atomic, i, i ^ ((i64_t) val), atomic) != 1);
# else
  int i;

  do {
    __MB();
    i = (int) (*((volatile size_t *) atomic));
  } while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((int) val), atomic) != 1);
# endif
  __MB();
  return (size_t) i;
#elif defined UNIC_ATOMIC_WIN
# if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedXor64((LONGLONG volatile *) atomic, (LONGLONG) val);
# else
  return (size_t) InterlockedXor((LONG volatile *) atomic, (LONG) val);
# endif
#else
  volatile size_t *ptr = atomic;
  size_t oldval;

  u_mutex_lock(pp_atomic_mutex);
  oldval = *ptr;
  *ptr = oldval ^ val;
  u_mutex_unlock(pp_atomic_mutex);
  return oldval;
#endif
}

bool
u_atomic_is_lock_free(void) {
#if defined UNIC_ATOMIC_C11
  return true;
#elif defined UNIC_ATOMIC_INTRIN
  return true;
#elif defined UNIC_ATOMIC_SYNC_INTRIN
  return true;
#elif defined UNIC_ATOMIC_DECC
  return true;
#elif defined UNIC_ATOMIC_WIN
  return true;
#else
  return false;
#endif
}

void
u_atomic_thread_init(void) {
#if defined UNIC_ATOMIC_C11
#elif defined UNIC_ATOMIC_INTRIN
#elif defined UNIC_ATOMIC_SYNC_INTRIN
#elif defined UNIC_ATOMIC_DECC
  return true;
#elif defined UNIC_ATOMIC_WIN
#else
  if (U_LIKELY (pp_atomic_mutex == NULL)) {
    pp_atomic_mutex = u_mutex_new();
  }
#endif
}

void
u_atomic_thread_shutdown(void) {
#if defined UNIC_ATOMIC_C11
#elif defined UNIC_ATOMIC_INTRIN
#elif defined UNIC_ATOMIC_SYNC_INTRIN
#elif defined UNIC_ATOMIC_DECC
  //todo
#elif defined UNIC_ATOMIC_WIN
#else
  if (U_LIKELY (pp_atomic_mutex != NULL)) {
    u_mutex_free(pp_atomic_mutex);
    pp_atomic_mutex = NULL;
  }
#endif
}
