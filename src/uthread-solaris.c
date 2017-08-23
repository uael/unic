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

#include "p/mem.h"
#include "p/atomic.h"
#include "p/uthread.h"
#include "uthread-private.h"

#ifndef P_OS_UNIXWARE
# include "p/mutex.h"
#endif

#ifdef P_OS_UNIXWARE
# define PLIBSYS_THREAD_MIN_PRIO 0
# define PLIBSYS_THREAD_MAX_PRIO 126
#else
# define PLIBSYS_THREAD_MIN_PRIO 0
# define PLIBSYS_THREAD_MAX_PRIO 127
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
#ifndef P_OS_UNIXWARE

static mutex_t *pp_uthread_tls_mutex = NULL;

#endif

static int
pp_uthread_get_unix_priority(uthread_prio_t prio);

static thread_key_t *
pp_uthread_get_tls_key(uthread_key_t *key);

static int
pp_uthread_get_unix_priority(uthread_prio_t prio) {
  int lowBound, upperBound;
  lowBound = (int) P_UTHREAD_PRIORITY_IDLE;
  upperBound = (int) P_UTHREAD_PRIORITY_TIMECRITICAL;
  return ((int) prio - lowBound) *
    (PLIBSYS_THREAD_MAX_PRIO - PLIBSYS_THREAD_MIN_PRIO) / upperBound +
    PLIBSYS_THREAD_MIN_PRIO;
}

static thread_key_t *
pp_uthread_get_tls_key(uthread_key_t *key) {
  thread_key_t *thread_key;
  thread_key = (thread_key_t *) p_atomic_pointer_get((ptr_t) &key->key);
  if (P_LIKELY (thread_key != NULL)) {
    return thread_key;
  }
#ifndef P_OS_UNIXWARE
  p_mutex_lock(pp_uthread_tls_mutex);
  thread_key = key->key;
  if (P_LIKELY (thread_key == NULL)) {
#endif
    if (P_UNLIKELY ((thread_key = p_malloc0(sizeof(thread_key_t))) == NULL)) {
      P_ERROR ("uthread_t::pp_uthread_get_tls_key: failed to allocate memory");
#ifndef P_OS_UNIXWARE
      p_mutex_unlock(pp_uthread_tls_mutex);
#endif
      return NULL;
    }
    if (P_UNLIKELY (thr_keycreate(thread_key, key->free_func) != 0)) {
      P_ERROR ("uthread_t::pp_uthread_get_tls_key: thr_keycreate() failed");
      p_free(thread_key);
#ifndef P_OS_UNIXWARE
      p_mutex_unlock(pp_uthread_tls_mutex);
#endif
      return NULL;
    }
#ifdef P_OS_UNIXWARE
    if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((ptr_t) &key->key,
                       NULL,
                       (ptr_t) thread_key) == false)) {
      if (P_UNLIKELY (thr_keydelete (*thread_key) != 0)) {
        P_ERROR ("uthread_t::pp_uthread_get_tls_key: thr_keydelete() failed");
        p_free (thread_key);
        return NULL;
      }

      p_free (thread_key);

      thread_key = key->key;
    }
#else
    key->key = thread_key;
  }
  p_mutex_unlock(pp_uthread_tls_mutex);
#endif
  return thread_key;
}

void
p_uthread_init_internal(void) {
#ifndef P_OS_UNIXWARE
  if (P_LIKELY (pp_uthread_tls_mutex == NULL)) {
    pp_uthread_tls_mutex = p_mutex_new();
  }
#endif
}

void
p_uthread_shutdown_internal(void) {
#ifndef P_OS_UNIXWARE
  if (P_LIKELY (pp_uthread_tls_mutex != NULL)) {
    p_mutex_free(pp_uthread_tls_mutex);
    pp_uthread_tls_mutex = NULL;
  }
#endif
}

void
p_uthread_win32_thread_detach(void) {
}

uthread_t *
p_uthread_create_internal(uthread_fn_t func,
  bool joinable,
  uthread_prio_t prio,
  size_t stack_size) {
  uthread_t *ret;
  int32_t flags;
  size_t min_stack;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(uthread_t))) == NULL)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  if (stack_size > 0) {
#ifdef P_OS_UNIXWARE
    min_stack = thr_minstack ();
#else
    min_stack = thr_min_stack();
#endif
    if (P_UNLIKELY (stack_size < min_stack)) {
      stack_size = min_stack;
    }
  }
  flags = THR_SUSPENDED;
  flags |= joinable ? 0 : THR_DETACHED;
  if (P_UNLIKELY (
    thr_create(NULL, stack_size, func, ret, flags, &ret->hdl) != 0)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: thr_create() failed");
    p_free(ret);
    return NULL;
  }
  if (P_UNLIKELY (
    thr_setprio(ret->hdl, pp_uthread_get_unix_priority(prio)) != 0))
    P_WARNING ("uthread_t::p_uthread_create_internal: thr_setprio() failed");
  if (P_UNLIKELY (thr_continue(ret->hdl) != 0)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: thr_continue() failed");
    p_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  return ret;
}

void
p_uthread_exit_internal(void) {
  thr_exit(P_INT_TO_POINTER (0));
}

void
p_uthread_wait_internal(uthread_t *thread) {
  if (P_UNLIKELY (thr_join(thread->hdl, NULL, NULL) != 0))
    P_ERROR ("uthread_t::p_uthread_wait_internal: thr_join() failed");
}

void
p_uthread_free_internal(uthread_t *thread) {
  p_free(thread);
}

void
p_uthread_yield(void) {
  thr_yield();
}

bool
p_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  if (P_UNLIKELY (thread == NULL)) {
    return false;
  }
  if (P_UNLIKELY (
    thr_setprio(thread->hdl, pp_uthread_get_unix_priority(prio)) != 0)) {
    P_WARNING ("uthread_t::p_uthread_set_priority: thr_setprio() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

P_HANDLE
p_uthread_current_id(void) {
  return (P_HANDLE) ((size_t) thr_self());
}

uthread_key_t *
p_uthread_local_new(destroy_fn_t free_func) {
  uthread_key_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(uthread_key_t))) == NULL)) {
    P_ERROR ("uthread_t::p_uthread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->free_func = free_func;
  return ret;
}

void
p_uthread_local_free(uthread_key_t *key) {
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  p_free(key);
}

ptr_t
p_uthread_get_local(uthread_key_t *key) {
  thread_key_t *tls_key;
  ptr_t ret = NULL;
  if (P_UNLIKELY (key == NULL)) {
    return ret;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_LIKELY (tls_key != NULL)) {
    if (P_UNLIKELY (thr_getspecific(*tls_key, &ret) != 0))
      P_ERROR ("uthread_t::p_uthread_get_local: thr_getspecific() failed");
  }
  return ret;
}

void
p_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  thread_key_t *tls_key;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_LIKELY (tls_key != NULL)) {
    if (P_UNLIKELY (thr_setspecific(*tls_key, value) != 0))
      P_ERROR ("uthread_t::p_uthread_set_local: thr_setspecific() failed");
  }
}

void
p_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  thread_key_t *tls_key;
  ptr_t old_value = NULL;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_UNLIKELY (tls_key == NULL)) {
    return;
  }
  if (P_UNLIKELY (thr_getspecific(*tls_key, &old_value) != 0)) {
    P_ERROR ("uthread_t::p_uthread_replace_local: thr_getspecific() failed");
    return;
  }
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  if (P_UNLIKELY (thr_setspecific(*tls_key, value) != 0))
    P_ERROR ("uthread_t::p_uthread_replace_local: thr_setspecific() failed");
}
