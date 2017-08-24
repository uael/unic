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

/* Prepare MemoryBarrier() */
#if defined (U_CC_WATCOM) || defined (U_CC_BORLAND)
# if defined (_M_X64) || defined (_M_AMD64)
#   define MemoryBarrier __faststorefence
# elseif defined (_M_IA64)
#   define MemoryBarrier __mf
# else
#   ifdef U_CC_WATCOM
inline
#   else
FORCEINLINE
#   endif /* U_CC_WATCOM */
VOID MemoryBarrier (VOID)
{
  LONG Barrier = 0;
  (void) (Barrier);
  __asm {
    xchg Barrier, eax
  }
}
# endif /* _M_X64 || _M_AMD64 */
#endif /* U_CC_WATCOM || U_CC_BORLAND */
#if !defined (U_OS_WIN64) && (defined (U_CC_MSVC) && _MSC_VER > 1200)
/* Tell compiler about intrinsics to suppress warnings,
 * see: https://msdn.microsoft.com/en-us/library/hh977023.aspx */
# include <intrin.h>
# define InterlockedAnd _InterlockedAnd
# define InterlockedOr _InterlockedOr
# define InterlockedXor _InterlockedXor
# pragma intrinsic(_InterlockedAnd)
# pragma intrinsic(_InterlockedOr)
# pragma intrinsic(_InterlockedXor)
#endif
#if (defined (U_CC_MSVC) && _MSC_VER <= 1200) || defined (U_CC_WATCOM) \
 || defined (U_CC_BORLAND)
/* Inlined versions for older compilers */
static LONG
ppInterlockedAnd (LONG volatile *atomic,
      LONG  val)
{
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i & val, i);
  } while (i != j);

  return j;
}

# define InterlockedAnd(a,b) ppInterlockedAnd(a,b)

static LONG
ppInterlockedOr (LONG volatile  *atomic,
     LONG  val)
{
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i | val, i);
  } while (i != j);

  return j;
}

# define InterlockedOr(a,b) ppInterlockedOr(a,b)

static LONG
ppInterlockedXor (LONG volatile *atomic,
      LONG  val)
{
  LONG i, j;

  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange (atomic, i ^ val, i);
  } while (i != j);

  return j;
}

# define InterlockedXor(a,b) ppInterlockedXor(a,b)
#endif

/* http://msdn.microsoft.com/en-us/library/ms684122(v=vs.85).aspx */

int
u_atomic_int_get(const volatile int *atomic) {
  MemoryBarrier();
  return *atomic;
}

void
u_atomic_int_set(volatile int *atomic,
  int val) {
  *atomic = val;
  MemoryBarrier();
}

void
u_atomic_int_inc(volatile int *atomic) {
  InterlockedIncrement((LONG volatile *) atomic);
}

bool
u_atomic_int_dec_and_test(volatile int *atomic) {
  return InterlockedDecrement((LONG volatile *) atomic) == 0 ? true : false;
}

bool
u_atomic_int_compare_and_exchange(volatile int *atomic,
  int oldval,
  int newval) {
  return InterlockedCompareExchange((LONG volatile *) atomic,
    (LONG) newval,
    (LONG) oldval
  ) == oldval;
}

int
u_atomic_int_add(volatile int *atomic,
  int val) {
  return (int) InterlockedExchangeAdd((LONG volatile *) atomic, (LONG) val);
}

uint_t
u_atomic_int_and(volatile uint_t *atomic,
  uint_t val) {
  return (uint_t) InterlockedAnd((LONG volatile *) atomic, (LONG) val);
}

uint_t
u_atomic_int_or(volatile uint_t *atomic,
  uint_t val) {
  return (uint_t) InterlockedOr((LONG volatile *) atomic, (LONG) val);
}

uint_t
u_atomic_int_xor(volatile uint_t *atomic,
  uint_t val) {
  return (uint_t) InterlockedXor((LONG volatile *) atomic, (LONG) val);
}

ptr_t
u_atomic_pointer_get(const volatile void *atomic) {
  const volatile ptr_t *ptr = (const volatile ptr_t *) atomic;
  MemoryBarrier();
  return *ptr;
}

void
u_atomic_pointer_set(volatile void *atomic,
  ptr_t val) {
  volatile ptr_t *ptr = (volatile ptr_t *) atomic;
  *ptr = val;
  MemoryBarrier();
}

bool
u_atomic_pointer_compare_and_exchange(volatile void *atomic,
  ptr_t oldval,
  ptr_t newval) {
  return InterlockedCompareExchangePointer((volatile PVOID *) atomic,
    (PVOID) newval,
    (PVOID) oldval) == oldval ? true : false;
}

ssize_t
u_atomic_pointer_add(volatile void *atomic,
  ssize_t val) {
#if UNIC_SIZEOF_VOID_P == 8
  return (ssize_t) InterlockedExchangeAdd64((LONGLONG volatile *) atomic,
    (LONGLONG) val);
#else
  return (ssize_t) InterlockedExchangeAdd((LONG volatile *) atomic, (LONG) val);
#endif
}

size_t
u_atomic_pointer_and(volatile void *atomic,
  size_t val) {
#if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedAnd64((LONGLONG volatile *) atomic, (LONGLONG) val);
#else
  return (size_t) InterlockedAnd((LONG volatile *) atomic, (LONG) val);
#endif
}

size_t
u_atomic_pointer_or(volatile void *atomic,
  size_t val) {
#if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedOr64((LONGLONG volatile *) atomic, (LONGLONG) val);
#else
  return (size_t) InterlockedOr((LONG volatile *) atomic, (LONG) val);
#endif
}

size_t
u_atomic_pointer_xor(volatile void *atomic,
  size_t val) {
#if UNIC_SIZEOF_VOID_P == 8
  return (size_t) InterlockedXor64((LONGLONG volatile *) atomic, (LONGLONG) val);
#else
  return (size_t) InterlockedXor((LONG volatile *) atomic, (LONG) val);
#endif
}

bool
u_atomic_is_lock_free(void) {
  return true;
}

void
u_atomic_thread_init(void) {
}

void
u_atomic_thread_shutdown(void) {
}
