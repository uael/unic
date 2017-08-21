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
#include "p/condvar.h"

#include <stdlib.h>
#include <thread.h>
#include <synch.h>

struct p_condvar {
  cond_t hdl;
};

P_API p_condvar_t *
p_condvar_new(void) {
  p_condvar_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_condvar_t))) == NULL)) {
    P_ERROR ("p_condvar_t::p_condvar_new: failed to allocate memory");
    return NULL;
  }

  if (P_UNLIKELY (cond_init(&ret->hdl, NULL, NULL) != 0)) {
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

  if (P_UNLIKELY (cond_destroy(&cond->hdl) != 0))
    P_WARNING ("p_condvar_t::p_condvar_free: cond_destroy() failed");

  p_free(cond);
}

P_API bool
p_condvar_wait(p_condvar_t *cond,
  PMutex *mutex) {
  if (P_UNLIKELY (cond == NULL || mutex == NULL))
    return false;

  /* Cast is eligible since there is only one field in the PMutex structure */
  if (P_UNLIKELY (cond_wait(&cond->hdl, (mutex_t *) mutex) != 0)) {
    P_ERROR ("p_condvar_t::p_condvar_wait: cond_wait() failed");
    return false;
  }

  return true;
}

P_API bool
p_condvar_signal(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return false;

  if (P_UNLIKELY (cond_signal(&cond->hdl) != 0)) {
    P_ERROR ("p_condvar_t::p_condvar_signal: cond_signal() failed");
    return false;
  }

  return true;
}

P_API bool
p_condvar_broadcast(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return false;

  if (P_UNLIKELY (cond_broadcast(&cond->hdl) != 0)) {
    P_ERROR (
      "p_condvar_t::p_condvar_broadcast: cond_broadcast() failed");
    return false;
  }

  return true;
}

void
p_condvar_init(void) {
}

void
p_condvar_shutdown(void) {
}
