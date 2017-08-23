/*
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

#include "p/mem.h"
#include "p/mutex.h"

typedef CRITICAL_SECTION mutex_hdl;

struct mutex {
  mutex_hdl hdl;
};

mutex_t *
p_mutex_new(void) {
  mutex_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(mutex_t))) == NULL)) {
    P_ERROR ("mutex_t::p_mutex_new: failed to allocate memory");
    return NULL;
  }
  InitializeCriticalSection(&ret->hdl);
  return ret;
}

bool
p_mutex_lock(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  EnterCriticalSection(&mutex->hdl);
  return true;
}

bool
p_mutex_trylock(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  return TryEnterCriticalSection(&mutex->hdl) != 0 ? true : false;
}

bool
p_mutex_unlock(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  LeaveCriticalSection(&mutex->hdl);
  return true;
}

void
p_mutex_free(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return;
  }
  DeleteCriticalSection(&mutex->hdl);
  p_free(mutex);
}
