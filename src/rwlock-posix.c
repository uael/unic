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

#include <pthread.h>

#include "unic/mem.h"
#include "unic/rwlock.h"

typedef pthread_rwlock_t rwlock_hdl;

struct rwlock {
  rwlock_hdl hdl;
};

static bool
pp_rwlock_unlock_any(rwlock_t *lock);

static bool
pp_rwlock_unlock_any(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_LIKELY (pthread_rwlock_unlock(&lock->hdl) == 0)) {
    return true;
  } else {
    U_ERROR ("rwlock_t::pp_rwlock_unlock_any: pthread_rwlock_unlock() failed");
    return false;
  }
}

rwlock_t *
u_rwlock_new(void) {
  rwlock_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(rwlock_t))) == NULL)) {
    U_ERROR ("rwlock_t::u_rwlock_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY (pthread_rwlock_init(&ret->hdl, NULL) != 0)) {
    U_ERROR ("rwlock_t::u_rwlock_new: pthread_rwlock_init() failed");
    u_free(ret);
    return NULL;
  }
  return ret;
}

bool
u_rwlock_reader_lock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (pthread_rwlock_rdlock(&lock->hdl) == 0)) {
    return true;
  } else {
    U_ERROR ("rwlock_t::u_rwlock_reader_lock: pthread_rwlock_rdlock() failed");
    return false;
  }
}

bool
u_rwlock_reader_trylock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  return (pthread_rwlock_tryrdlock(&lock->hdl) == 0) ? true : false;
}

bool
u_rwlock_reader_unlock(rwlock_t *lock) {
  return pp_rwlock_unlock_any(lock);
}

bool
u_rwlock_writer_lock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (pthread_rwlock_wrlock(&lock->hdl) == 0)) {
    return true;
  } else {
    U_ERROR ("rwlock_t::u_rwlock_writer_lock: pthread_rwlock_wrlock() failed");
    return false;
  }
}

bool
u_rwlock_writer_trylock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  return (pthread_rwlock_trywrlock(&lock->hdl) == 0) ? true : false;
}

bool
u_rwlock_writer_unlock(rwlock_t *lock) {
  return pp_rwlock_unlock_any(lock);
}

void
u_rwlock_free(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return;
  }
  if (U_UNLIKELY (pthread_rwlock_destroy(&lock->hdl) != 0))
    U_ERROR ("rwlock_t::u_rwlock_free: pthread_rwlock_destroy() failed");
  u_free(lock);
}

void
u_rwlock_init(void) {
}

void
u_rwlock_shutdown(void) {
}
