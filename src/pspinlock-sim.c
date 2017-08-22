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
#include "p/mutex.h"
#include "p/spinlock.h"

struct spinlock {
  mutex_t *mutex;
};

spinlock_t *
p_spinlock_new(void) {
  spinlock_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(spinlock_t))) == NULL)) {
    P_ERROR ("spinlock_t::p_spinlock_new: failed to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY ((ret->mutex = p_mutex_new()) == NULL)) {
    P_ERROR ("spinlock_t::p_spinlock_new: p_mutex_new() failed");
    p_free(ret);
    return NULL;
  }
  return ret;
}

bool
p_spinlock_lock(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  return p_mutex_lock(spinlock->mutex);
}

bool
p_spinlock_trylock(spinlock_t *spinlock) {
  if (spinlock == NULL) {
    return false;
  }
  return p_mutex_trylock(spinlock->mutex);
}

bool
p_spinlock_unlock(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  return p_mutex_unlock(spinlock->mutex);
}

void
p_spinlock_free(spinlock_t *spinlock) {
  if (P_UNLIKELY (spinlock == NULL)) {
    return;
  }
  p_mutex_free(spinlock->mutex);
  p_free(spinlock);
}
