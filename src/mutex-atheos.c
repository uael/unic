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

#include <errno.h>
#include <atheos/semaphore.h>

#include "unic/mem.h"
#include "unic/mutex.h"

typedef sem_id mutex_hdl;

struct mutex {
  mutex_hdl hdl;
};

mutex_t *
u_mutex_new(void) {
  mutex_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(mutex_t))) == NULL)) {
    U_ERROR ("mutex_t::u_mutex_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY ((ret->hdl = create_semaphore("", 1, 0)) < 0)) {
    U_ERROR ("mutex_t::u_mutex_new: create_semaphore() failed");
    u_free(ret);
    return NULL;
  }
  return ret;
}

bool
u_mutex_lock(mutex_t *mutex) {
  status_t ret_status;
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  while ((ret_status = lock_semaphore(mutex->hdl)) == EINTR);
  if (U_LIKELY (ret_status == 0)) {
    return true;
  } else {
    U_ERROR ("mutex_t::u_mutex_lock: lock_semaphore() failed");
    return false;
  }
}

bool
u_mutex_trylock(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  return (lock_sema_x(mutex->hdl, 1, 0, 0)) == 0 ? true : false;
}

bool
u_mutex_unlock(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  if (U_LIKELY (unlock_semaphore(mutex->hdl) == 0)) {
    return true;
  } else {
    U_ERROR ("mutex_t::u_mutex_unlock: unlock_semaphore() failed");
    return false;
  }
}

void
u_mutex_free(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return;
  }
  if (U_UNLIKELY (delete_semaphore(mutex->hdl) != 0))
    U_ERROR ("mutex_t::u_mutex_free: delete_semaphore() failed");
  u_free(mutex);
}
