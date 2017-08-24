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

#include <thread.h>

#include "unic/mem.h"
#include "unic/atomic.h"
#include "unic/uthread.h"
#include "uthread-private.h"

#ifndef U_OS_UNIXWARE
# include "unic/mutex.h"
#endif

#ifdef U_OS_UNIXWARE
# define UNIC_THREAD_MIN_PRIO 0
# define UNIC_THREAD_MAX_PRIO 126
#else
# define UNIC_THREAD_MIN_PRIO 0
# define UNIC_THREAD_MAX_PRIO 127
#endif

typedef thread_t puthread_hdl;

struct uthread {
  PUThreadBase base;
  puthread_hdl hdl;
};

struct uthread_key {
  thread_key_t *key;
  destroy_fn_t free_func;
};
#ifndef U_OS_UNIXWARE

static mutex_t *pp_uthread_tls_mutex = NULL;

#endif

static int
pp_uthread_get_unix_priority(uthread_prio_t prio);

static thread_key_t *
pp_uthread_get_tls_key(uthread_key_t *key);

static int
pp_uthread_get_unix_priority(uthread_prio_t prio) {
  int lowBound, upperBound;
  lowBound = (int) U_UTHREAD_PRIORITY_IDLE;
  upperBound = (int) U_UTHREAD_PRIORITY_TIMECRITICAL;
  return ((int) prio - lowBound) *
    (UNIC_THREAD_MAX_PRIO - UNIC_THREAD_MIN_PRIO) / upperBound +
    UNIC_THREAD_MIN_PRIO;
}

static thread_key_t *
pp_uthread_get_tls_key(uthread_key_t *key) {
  thread_key_t *thread_key;
  thread_key = (thread_key_t *) u_atomic_pointer_get((ptr_t) &key->key);
  if (U_LIKELY (thread_key != NULL)) {
    return thread_key;
  }
#ifndef U_OS_UNIXWARE
  u_mutex_lock(pp_uthread_tls_mutex);
  thread_key = key->key;
  if (U_LIKELY (thread_key == NULL)) {
#endif
    if (U_UNLIKELY ((thread_key = u_malloc0(sizeof(thread_key_t))) == NULL)) {
      U_ERROR ("uthread_t::pp_uthread_get_tls_key: failed to allocate memory");
#ifndef U_OS_UNIXWARE
      u_mutex_unlock(pp_uthread_tls_mutex);
#endif
      return NULL;
    }
    if (U_UNLIKELY (thr_keycreate(thread_key, key->free_func) != 0)) {
      U_ERROR ("uthread_t::pp_uthread_get_tls_key: thr_keycreate() failed");
      u_free(thread_key);
#ifndef U_OS_UNIXWARE
      u_mutex_unlock(pp_uthread_tls_mutex);
#endif
      return NULL;
    }
#ifdef U_OS_UNIXWARE
    if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange ((ptr_t) &key->key,
                       NULL,
                       (ptr_t) thread_key) == false)) {
      if (U_UNLIKELY (thr_keydelete (*thread_key) != 0)) {
        U_ERROR ("uthread_t::pp_uthread_get_tls_key: thr_keydelete() failed");
        u_free (thread_key);
        return NULL;
      }

      u_free (thread_key);

      thread_key = key->key;
    }
#else
    key->key = thread_key;
  }
  u_mutex_unlock(pp_uthread_tls_mutex);
#endif
  return thread_key;
}

void
u_uthread_init_internal(void) {
#ifndef U_OS_UNIXWARE
  if (U_LIKELY (pp_uthread_tls_mutex == NULL)) {
    pp_uthread_tls_mutex = u_mutex_new();
  }
#endif
}

void
u_uthread_shutdown_internal(void) {
#ifndef U_OS_UNIXWARE
  if (U_LIKELY (pp_uthread_tls_mutex != NULL)) {
    u_mutex_free(pp_uthread_tls_mutex);
    pp_uthread_tls_mutex = NULL;
  }
#endif
}

void
u_uthread_win32_thread_detach(void) {
}

uthread_t *
u_uthread_create_internal(uthread_fn_t func,
  bool joinable,
  uthread_prio_t prio,
  size_t stack_size) {
  uthread_t *ret;
  i32_t flags;
  size_t min_stack;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  if (stack_size > 0) {
#ifdef U_OS_UNIXWARE
    min_stack = thr_minstack ();
#else
    min_stack = thr_min_stack();
#endif
    if (U_UNLIKELY (stack_size < min_stack)) {
      stack_size = min_stack;
    }
  }
  flags = THR_SUSPENDED;
  flags |= joinable ? 0 : THR_DETACHED;
  if (U_UNLIKELY (
    thr_create(NULL, stack_size, func, ret, flags, &ret->hdl) != 0)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: thr_create() failed");
    u_free(ret);
    return NULL;
  }
  if (U_UNLIKELY (
    thr_setprio(ret->hdl, pp_uthread_get_unix_priority(prio)) != 0))
    U_WARNING ("uthread_t::u_uthread_create_internal: thr_setprio() failed");
  if (U_UNLIKELY (thr_continue(ret->hdl) != 0)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: thr_continue() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  return ret;
}

void
u_uthread_exit_internal(void) {
  thr_exit(U_INT_TO_POINTER (0));
}

void
u_uthread_wait_internal(uthread_t *thread) {
  if (U_UNLIKELY (thr_join(thread->hdl, NULL, NULL) != 0))
    U_ERROR ("uthread_t::u_uthread_wait_internal: thr_join() failed");
}

void
u_uthread_free_internal(uthread_t *thread) {
  u_free(thread);
}

void
u_uthread_yield(void) {
  thr_yield();
}

bool
u_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  if (U_UNLIKELY (
    thr_setprio(thread->hdl, pp_uthread_get_unix_priority(prio)) != 0)) {
    U_WARNING ("uthread_t::u_uthread_set_priority: thr_setprio() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_uthread_current_id(void) {
  return (U_HANDLE) ((size_t) thr_self());
}

uthread_key_t *
u_uthread_local_new(destroy_fn_t free_func) {
  uthread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_key_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->free_func = free_func;
  return ret;
}

void
u_uthread_local_free(uthread_key_t *key) {
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  u_free(key);
}

ptr_t
u_uthread_get_local(uthread_key_t *key) {
  thread_key_t *tls_key;
  ptr_t ret = NULL;
  if (U_UNLIKELY (key == NULL)) {
    return ret;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (U_LIKELY (tls_key != NULL)) {
    if (U_UNLIKELY (thr_getspecific(*tls_key, &ret) != 0))
      U_ERROR ("uthread_t::u_uthread_get_local: thr_getspecific() failed");
  }
  return ret;
}

void
u_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  thread_key_t *tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (U_LIKELY (tls_key != NULL)) {
    if (U_UNLIKELY (thr_setspecific(*tls_key, value) != 0))
      U_ERROR ("uthread_t::u_uthread_set_local: thr_setspecific() failed");
  }
}

void
u_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  thread_key_t *tls_key;
  ptr_t old_value = NULL;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (U_UNLIKELY (tls_key == NULL)) {
    return;
  }
  if (U_UNLIKELY (thr_getspecific(*tls_key, &old_value) != 0)) {
    U_ERROR ("uthread_t::u_uthread_replace_local: thr_getspecific() failed");
    return;
  }
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  if (U_UNLIKELY (thr_setspecific(*tls_key, value) != 0))
    U_ERROR ("uthread_t::u_uthread_replace_local: thr_setspecific() failed");
}
