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

struct spinlock {
  volatile int spin;
};

spinlock_t *
p_spinlock_new(void) {
  spinlock_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(spinlock_t))) == NULL)) {
    P_ERROR ("spinlock_t::p_spinlock_new: failed to allocate memory");
    return NULL;
  }
  return ret;
}

bool
p_spinlock_lock(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  while ((bool) __sync_bool_compare_and_swap(&(spinlock->spin), 0, 1)
    == false) {}
  return true;
}

bool
p_spinlock_trylock(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  return (bool) __sync_bool_compare_and_swap(&(spinlock->spin), 0, 1);
}

bool
p_spinlock_unlock(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  spinlock->spin = 0;
  __sync_synchronize();
  return true;
}

void
p_spinlock_free(spinlock_t *spinlock) {
  p_free(spinlock);
}
