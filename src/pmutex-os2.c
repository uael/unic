/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#include "p/mem.h"
#include "p/mutex.h"

typedef HMTX mutex_hdl;

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
  if (P_UNLIKELY (
    DosCreateMutexSem(NULL, (PHMTX) & ret->hdl, 0, false) != NO_ERROR)) {
    P_ERROR ("mutex_t::p_mutex_new: DosCreateMutexSem() failed");
    p_free(ret);
    return NULL;
  }
  return ret;
}

bool
p_mutex_lock(mutex_t *mutex) {
  APIRET ulrc;
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  while ((ulrc = DosRequestMutexSem(mutex->hdl, SEM_INDEFINITE_WAIT))
    == ERROR_INTERRUPT) {}
  if (P_LIKELY (ulrc == NO_ERROR)) {
    return true;
  } else {
    P_ERROR ("mutex_t::p_mutex_lock: DosRequestMutexSem() failed");
    return false;
  }
}

bool
p_mutex_trylock(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  return (DosRequestMutexSem(mutex->hdl, SEM_IMMEDIATE_RETURN)) == NO_ERROR
    ? true : false;
}

bool
p_mutex_unlock(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return false;
  }
  if (P_LIKELY (DosReleaseMutexSem(mutex->hdl) == NO_ERROR)) {
    return true;
  } else {
    P_ERROR ("mutex_t::p_mutex_unlock: DosReleaseMutexSem() failed");
    return false;
  }
}

void
p_mutex_free(mutex_t *mutex) {
  if (P_UNLIKELY (mutex == NULL)) {
    return;
  }
  if (P_UNLIKELY (DosCloseMutexSem(mutex->hdl) != NO_ERROR))
    P_ERROR ("mutex_t::p_mutex_free: DosCloseMutexSem() failed");
  p_free(mutex);
}
