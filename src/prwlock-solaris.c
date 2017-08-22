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
#include "p/rwlock.h"
#include <thread.h>

typedef rwlock_t rwlock_hdl;

struct rwlock {
  rwlock_hdl hdl;
};

static bool
pp_rwlock_unlock_any(rwlock_t *lock);

static bool
pp_rwlock_unlock_any(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_LIKELY (rw_unlock(&lock->hdl) == 0)) {
    return true;
  } else {
    P_ERROR ("rwlock_t::pp_rwlock_unlock_any: rw_unlock() failed");
    return false;
  }
}

rwlock_t *
p_rwlock_new(void) {
  rwlock_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(rwlock_t))) == NULL)) {
    P_ERROR ("rwlock_t::p_rwlock_new: failed to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY (rwlock_init(&ret->hdl, USYNC_THREAD, NULL) != 0)) {
    P_ERROR ("rwlock_t::p_rwlock_new: rwlock_init() failed");
    p_free(ret);
    return NULL;
  }
  return ret;
}

bool
p_rwlock_reader_lock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (rw_rdlock(&lock->hdl) == 0)) {
    return true;
  } else {
    P_ERROR ("rwlock_t::p_rwlock_reader_lock: rw_rdlock() failed");
    return false;
  }
}

bool
p_rwlock_reader_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return (rw_tryrdlock(&lock->hdl) == 0) ? true : false;
}

bool
p_rwlock_reader_unlock(rwlock_t *lock) {
  return pp_rwlock_unlock_any(lock);
}

bool
p_rwlock_writer_lock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (rw_wrlock(&lock->hdl) == 0)) {
    return true;
  } else {
    P_ERROR ("rwlock_t::p_rwlock_writer_lock: rw_wrlock() failed");
    return false;
  }
}

bool
p_rwlock_writer_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return (rw_trywrlock(&lock->hdl) == 0) ? true : false;
}

bool
p_rwlock_writer_unlock(rwlock_t *lock) {
  return pp_rwlock_unlock_any(lock);
}

void
p_rwlock_free(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return;
  }
  if (P_UNLIKELY (rwlock_destroy(&lock->hdl) != 0))
    P_ERROR ("rwlock_t::p_rwlock_free: rwlock_destroy() failed");
  p_free(lock);
}

void
p_rwlock_init(void) {
}

void
p_rwlock_shutdown(void) {
}
