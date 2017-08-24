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

#include "unic/atomic.h"
#include "unic/mem.h"
#include "unic/condvar.h"

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#include <os2.h>

struct condvar {
  HEV waiters_sema;
  int waiters_count;
  int signaled;
};

condvar_t *
u_condvar_new(void) {
  condvar_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(condvar_t))) == NULL)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY (DosCreateEventSem(
    NULL,
    (PHEV) & ret->waiters_sema,
    0,
    false
  ) != NO_ERROR)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to initialize");
    u_free(ret);
    return NULL;
  }
  return ret;
}

void
u_condvar_free(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return;
  }
  if (U_UNLIKELY (DosCloseEventSem(cond->waiters_sema) != NO_ERROR))
    U_WARNING (
      "condvar_t::u_condvar_free: DosCloseEventSem() failed");
  u_free(cond);
}

bool
u_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  APIRET ulrc;
  APIRET reset_ulrc;
  if (U_UNLIKELY (cond == NULL || mutex == NULL)) {
    return false;
  }
  do {
    u_atomic_int_inc(&cond->waiters_count);
    u_mutex_unlock(mutex);
    do {
      ULONG post_count;
      ulrc = DosWaitEventSem(cond->waiters_sema, SEM_INDEFINITE_WAIT);
      if (ulrc == NO_ERROR) {
        reset_ulrc = DosResetEventSem(cond->waiters_sema, &post_count);
        if (U_UNLIKELY (reset_ulrc != NO_ERROR &&
          reset_ulrc != ERROR_ALREADY_RESET))
          U_WARNING (
            "condvar_t::u_condvar_wait: DosResetEventSem() failed");
      }
    } while (ulrc == NO_ERROR &&
      u_atomic_int_compare_and_exchange(&cond->signaled, 1, 0) == false);
    u_atomic_int_add(&cond->waiters_count, -1);
    u_mutex_lock(mutex);
  } while (ulrc == ERROR_INTERRUPT);
  return (ulrc == NO_ERROR) ? true : false;
}

bool
u_condvar_signal(condvar_t *cond) {
  bool result = true;
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (u_atomic_int_get(&cond->waiters_count) > 0) {
    ULONG post_count;
    APIRET ulrc;
    u_atomic_int_set(&cond->signaled, 1);
    ulrc = DosPostEventSem(cond->waiters_sema);
    if (U_UNLIKELY (ulrc != NO_ERROR &&
      ulrc != ERROR_ALREADY_POSTED &&
      ulrc != ERROR_TOO_MANY_POSTS)) {
      U_WARNING (
        "condvar_t::u_condvar_signal: DosPostEventSem() failed");
      result = false;
    }
  }
  return result;
}

bool
u_condvar_broadcast(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  bool result = true;
  while (u_atomic_int_get(&cond->waiters_count) != 0) {
    if (U_UNLIKELY (u_condvar_signal(cond) == false)) {
      result = false;
    }
  }
  return result;
}

void
u_condvar_init(void) {
}

void
u_condvar_shutdown(void) {
}
