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

#include "p/mem.h"
#include "p/spinlock.h"

#ifdef P_CC_SUN
#  define PSPINLOCK_INT_CAST(x) (int_t *) (x)
#else
#  define PSPINLOCK_INT_CAST(x) x
#endif

struct PSpinLock_ {
  volatile int_t spin;
};

P_API PSpinLock *
p_spinlock_new(void) {
  PSpinLock *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PSpinLock))) == NULL)) {
    P_ERROR ("PSpinLock::p_spinlock_new: failed to allocate memory");
    return NULL;
  }

  return ret;
}

P_API bool
p_spinlock_lock(PSpinLock *spinlock) {
  int_t tmp_int;

  if (P_UNLIKELY (spinlock == NULL))
    return false;

  do {
    tmp_int = 0;
  } while ((bool) __atomic_compare_exchange_n(
    PSPINLOCK_INT_CAST (&(spinlock->spin)),
    &tmp_int,
    1,
    0,
    __ATOMIC_ACQUIRE,
    __ATOMIC_RELAXED) == false);

  return true;
}

P_API bool
p_spinlock_trylock(PSpinLock *spinlock) {
  int_t tmp_int = 0;

  if (P_UNLIKELY (spinlock == NULL))
    return false;

  return (bool) __atomic_compare_exchange_n(
    PSPINLOCK_INT_CAST (&(spinlock->spin)),
    &tmp_int,
    1,
    0,
    __ATOMIC_ACQUIRE,
    __ATOMIC_RELAXED);
}

P_API bool
p_spinlock_unlock(PSpinLock *spinlock) {
  if (P_UNLIKELY (spinlock == NULL))
    return false;

  __atomic_store_4(PSPINLOCK_INT_CAST (&(spinlock->spin)), 0, __ATOMIC_RELEASE);

  return true;
}

P_API void
p_spinlock_free(PSpinLock *spinlock) {
  p_free(spinlock);
}
