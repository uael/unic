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

#include "unic/mem.h"
#include "unic/mutex.h"

typedef HMTX mutex_hdl;

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
  if (U_UNLIKELY (
    DosCreateMutexSem(NULL, (PHMTX) & ret->hdl, 0, false) != NO_ERROR)) {
    U_ERROR ("mutex_t::u_mutex_new: DosCreateMutexSem() failed");
    u_free(ret);
    return NULL;
  }
  return ret;
}

bool
u_mutex_lock(mutex_t *mutex) {
  APIRET ulrc;
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  while ((ulrc = DosRequestMutexSem(mutex->hdl, SEM_INDEFINITE_WAIT))
    == ERROR_INTERRUPT) {}
  if (U_LIKELY (ulrc == NO_ERROR)) {
    return true;
  } else {
    U_ERROR ("mutex_t::u_mutex_lock: DosRequestMutexSem() failed");
    return false;
  }
}

bool
u_mutex_trylock(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  return (DosRequestMutexSem(mutex->hdl, SEM_IMMEDIATE_RETURN)) == NO_ERROR
    ? true : false;
}

bool
u_mutex_unlock(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return false;
  }
  if (U_LIKELY (DosReleaseMutexSem(mutex->hdl) == NO_ERROR)) {
    return true;
  } else {
    U_ERROR ("mutex_t::u_mutex_unlock: DosReleaseMutexSem() failed");
    return false;
  }
}

void
u_mutex_free(mutex_t *mutex) {
  if (U_UNLIKELY (mutex == NULL)) {
    return;
  }
  if (U_UNLIKELY (DosCloseMutexSem(mutex->hdl) != NO_ERROR))
    U_ERROR ("mutex_t::u_mutex_free: DosCloseMutexSem() failed");
  u_free(mutex);
}
