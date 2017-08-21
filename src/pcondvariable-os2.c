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

#include "p/atomic.h"
#include "p/mem.h"
#include "p/condvar.h"

#include <stdlib.h>

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

struct p_condvar {
  HEV waiters_sema;
  int_t waiters_count;
  int_t signaled;
};

P_API p_condvar_t *
p_condvar_new(void) {
  p_condvar_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_condvar_t))) == NULL)) {
    P_ERROR ("p_condvar_t::p_condvar_new: failed to allocate memory");
    return NULL;
  }

  if (P_UNLIKELY (DosCreateEventSem(NULL,
    (PHEV) & ret->waiters_sema,
    0,
    false) != NO_ERROR)) {
    P_ERROR ("p_condvar_t::p_condvar_new: failed to initialize");
    p_free(ret);
    return NULL;
  }

  return ret;
}

P_API void
p_condvar_free(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return;

  if (P_UNLIKELY (DosCloseEventSem(cond->waiters_sema) != NO_ERROR))
    P_WARNING (
      "p_condvar_t::p_condvar_free: DosCloseEventSem() failed");

  p_free(cond);
}

P_API bool
p_condvar_wait(p_condvar_t *cond,
  PMutex *mutex) {
  APIRET ulrc;
  APIRET reset_ulrc;

  if (P_UNLIKELY (cond == NULL || mutex == NULL))
    return false;

  do {
    p_atomic_int_inc(&cond->waiters_count);
    p_mutex_unlock(mutex);

    do {
      ULONG post_count;

      ulrc = DosWaitEventSem(cond->waiters_sema, SEM_INDEFINITE_WAIT);

      if (ulrc == NO_ERROR) {
        reset_ulrc = DosResetEventSem(cond->waiters_sema, &post_count);

        if (P_UNLIKELY (reset_ulrc != NO_ERROR &&
          reset_ulrc != ERROR_ALREADY_RESET))
          P_WARNING (
            "p_condvar_t::p_condvar_wait: DosResetEventSem() failed");
      }
    } while (ulrc == NO_ERROR &&
      p_atomic_int_compare_and_exchange(&cond->signaled, 1, 0) == false);

    p_atomic_int_add(&cond->waiters_count, -1);
    p_mutex_lock(mutex);
  } while (ulrc == ERROR_INTERRUPT);

  return (ulrc == NO_ERROR) ? true : false;
}

P_API bool
p_condvar_signal(p_condvar_t *cond) {
  bool result = true;

  if (P_UNLIKELY (cond == NULL))
    return false;

  if (p_atomic_int_get(&cond->waiters_count) > 0) {
    ULONG post_count;
    APIRET ulrc;

    p_atomic_int_set(&cond->signaled, 1);

    ulrc = DosPostEventSem(cond->waiters_sema);

    if (P_UNLIKELY (ulrc != NO_ERROR &&
      ulrc != ERROR_ALREADY_POSTED &&
      ulrc != ERROR_TOO_MANY_POSTS)) {
      P_WARNING (
        "p_condvar_t::p_condvar_signal: DosPostEventSem() failed");
      result = false;
    }
  }

  return result;
}

P_API bool
p_condvar_broadcast(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return false;

  bool result = true;

  while (p_atomic_int_get(&cond->waiters_count) != 0) {
    if (P_UNLIKELY (p_condvar_signal(cond) == false))
      result = false;
  }

  return result;
}

void
p_condvar_init(void) {
}

void
p_condvar_shutdown(void) {
}
