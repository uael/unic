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

#include "p/mem.h"
#include "p/atomic.h"
#include "p/uthread.h"
#include "puthread-private.h"
#include <atheos/threads.h>
#include <atheos/tld.h>

typedef thread_id puthread_hdl;

struct uthread {
  PUThreadBase base;
  puthread_hdl hdl;
};

struct uthread_key {
  int_t key;
  destroy_fn_t free_func;
};

static int_t
pp_uthread_get_atheos_priority(uthread_prio_t prio);

static int_t
pp_uthread_get_tls_key(uthread_key_t *key);

static int_t
pp_uthread_get_atheos_priority(uthread_prio_t prio) {
  switch (prio) {
    case P_UTHREAD_PRIORITY_INHERIT: {
      thread_info thr_info;
      memset(&thr_info, 0, sizeof(thr_info));
      if (P_UNLIKELY (get_thread_info(get_thread_id(NULL), &thr_info) != 0)) {
        P_WARNING (
          "uthread_t::pp_uthread_get_atheos_priority: failed to get thread info");
        return NORMAL_PRIORITY;
      } else {
        return thr_info.ti_priority;
      }
    }
    case P_UTHREAD_PRIORITY_IDLE:
      return IDLE_PRIORITY;
    case P_UTHREAD_PRIORITY_LOWEST:
      return LOW_PRIORITY / 2;
    case P_UTHREAD_PRIORITY_LOW:
      return LOW_PRIORITY;
    case P_UTHREAD_PRIORITY_NORMAL:
      return NORMAL_PRIORITY;
    case P_UTHREAD_PRIORITY_HIGH:
      return DISPLAY_PRIORITY;
    case P_UTHREAD_PRIORITY_HIGHEST:
      return URGENT_DISPLAY_PRIORITY;
    case P_UTHREAD_PRIORITY_TIMECRITICAL:
      return REALTIME_PRIORITY;
  }
}

static int_t
pp_uthread_get_tls_key(uthread_key_t *key) {
  int_t thread_key;
  thread_key = p_atomic_int_get((const volatile int_t *) &key->key);
  if (P_LIKELY (thread_key >= 0)) {
    return thread_key;
  }
  if (P_UNLIKELY ((thread_key = alloc_tld(key->free_func)) < 0)) {
    P_ERROR ("uthread_t::pp_uthread_get_tls_key: alloc_tld() failed");
    return -1;
  }
  if (P_UNLIKELY (
    p_atomic_int_compare_and_exchange((volatile int_t *) &key->key,
      -1,
      thread_key
    ) == false)) {
    if (P_UNLIKELY (free_tld(thread_key) != 0)) {
      P_ERROR ("uthread_t::pp_uthread_get_tls_key: free_tld() failed");
      return -1;
    }
    thread_key = key->key;
  }
  return thread_key;
}

void
p_uthread_init_internal(void) {
}

void
p_uthread_shutdown_internal(void) {
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
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(uthread_t))) == NULL)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY ((
    ret->hdl = spawn_thread(
      "",
      func,
      pp_uthread_get_atheos_priority(prio),
      stack_size,
      ret
    )) < 0)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: spawn_thread() failed");
    p_free(ret);
    return NULL;
  }
  if (P_UNLIKELY (resume_thread(ret->hdl) != 0)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: resume_thread() failed");
    p_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  return ret;
}

void
p_uthread_exit_internal(void) {
  exit_thread(0);
}

void
p_uthread_wait_internal(uthread_t *thread) {
  wait_for_thread(thread->hdl);
}

void
p_uthread_free_internal(uthread_t *thread) {
  p_free(thread);
}

void
p_uthread_yield(void) {
  sched_yield();
}

bool
p_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  if (P_UNLIKELY (thread == NULL)) {
    return false;
  }
  set_thread_priority(thread->hdl, pp_uthread_get_atheos_priority(prio));
  thread->base.prio = prio;
  return true;
}

P_HANDLE
p_uthread_current_id(void) {
  return (P_HANDLE) ((size_t) get_thread_id(NULL));
}

uthread_key_t *
p_uthread_local_new(destroy_fn_t free_func) {
  uthread_key_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(uthread_key_t))) == NULL)) {
    P_ERROR ("uthread_t::p_uthread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->key = -1;
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
  int_t tls_key;
  if (P_UNLIKELY (key == NULL)) {
    return NULL;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_LIKELY (tls_key >= 0)) {
    return get_tld(tls_key);
  }
  return NULL;
}

void
p_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  int_t tls_key;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (tls_key >= 0) {
    set_tld(tls_key, value);
  }
}

void
p_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  int_t tls_key;
  ptr_t old_value;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_UNLIKELY (tls_key < 0)) {
    return;
  }
  old_value = get_tld(tls_key);
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  set_tld(tls_key, value);
}
