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

#include <pthread.h>

#include "unic/mem.h"
#include "unic/condvar.h"

struct condvar {
  pthread_cond_t hdl;
};

condvar_t *
u_condvar_new(void) {
  condvar_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(condvar_t))) == NULL)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY (pthread_cond_init(&ret->hdl, NULL) != 0)) {
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
  if (U_UNLIKELY (pthread_cond_destroy(&cond->hdl) != 0))
    U_WARNING (
      "condvar_t::u_condvar_free: pthread_cond_destroy() failed");
  u_free(cond);
}

bool
u_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  if (U_UNLIKELY (cond == NULL || mutex == NULL)) {
    return false;
  }

  /* Cast is eligible since there is only one field in the mutex_t structure */
  if (U_UNLIKELY (
    pthread_cond_wait(&cond->hdl, (pthread_mutex_t *) mutex) != 0)) {
    U_ERROR ("condvar_t::u_condvar_wait: pthread_cond_wait() failed");
    return false;
  }
  return true;
}

bool
u_condvar_signal(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (U_UNLIKELY (pthread_cond_signal(&cond->hdl) != 0)) {
    U_ERROR (
      "condvar_t::u_condvar_signal: pthread_cond_signal() failed");
    return false;
  }
  return true;
}

bool
u_condvar_broadcast(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (U_UNLIKELY (pthread_cond_broadcast(&cond->hdl) != 0)) {
    U_ERROR (
      "condvar_t::u_condvar_broadcast: thread_cond_broadcast() failed");
    return false;
  }
  return true;
}

void
u_condvar_init(void) {
}

void
u_condvar_shutdown(void) {
}
