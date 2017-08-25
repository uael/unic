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

#include <atheos/threads.h>
#include <atheos/tld.h>

#include "unic/mem.h"
#include "unic/atomic.h"
#include "unic/thread.h"
#include "unic/string.h"
#include "thread-private.h"

typedef thread_id thread_hdl_t;

struct thread {
  thread_base_t base;
  thread_hdl_t hdl;
};

struct thread_key {
  int key;
  destroy_fn_t free_func;
};

static int
pp_thread_get_atheos_priority(thread_prio_t prio);

static int
pp_thread_get_tls_key(thread_key_t *key);

static int
pp_thread_get_atheos_priority(thread_prio_t prio) {
  switch (prio) {
    case U_thread_PRIORITY_INHERIT: {
      thread_info thr_info;
      memset(&thr_info, 0, sizeof(thr_info));
      if (U_UNLIKELY (get_thread_info(get_thread_id(NULL), &thr_info) != 0)) {
        U_WARNING (
          "thread_t::pp_thread_get_atheos_priority: failed to get thread info");
        return NORMAL_PRIORITY;
      } else {
        return thr_info.ti_priority;
      }
    }
    case U_thread_PRIORITY_IDLE:
      return IDLE_PRIORITY;
    case U_thread_PRIORITY_LOWEST:
      return LOW_PRIORITY / 2;
    case U_thread_PRIORITY_LOW:
      return LOW_PRIORITY;
    case U_thread_PRIORITY_NORMAL:
      return NORMAL_PRIORITY;
    case U_thread_PRIORITY_HIGH:
      return DISPLAY_PRIORITY;
    case U_thread_PRIORITY_HIGHEST:
      return URGENT_DISPLAY_PRIORITY;
    case U_thread_PRIORITY_TIMECRITICAL:
      return REALTIME_PRIORITY;
  }
}

static int
pp_thread_get_tls_key(thread_key_t *key) {
  int thread_key;
  thread_key = u_atomic_int_get((const volatile int *) &key->key);
  if (U_LIKELY (thread_key >= 0)) {
    return thread_key;
  }
  if (U_UNLIKELY ((thread_key = alloc_tld(key->free_func)) < 0)) {
    U_ERROR ("thread_t::pp_thread_get_tls_key: alloc_tld() failed");
    return -1;
  }
  if (U_UNLIKELY (
    u_atomic_int_compare_and_exchange((volatile int *) &key->key,
      -1,
      thread_key
    ) == false)) {
    if (U_UNLIKELY (free_tld(thread_key) != 0)) {
      U_ERROR ("thread_t::pp_thread_get_tls_key: free_tld() failed");
      return -1;
    }
    thread_key = key->key;
  }
  return thread_key;
}

void
u_thread_init_internal(void) {
}

void
u_thread_shutdown_internal(void) {
}

void
u_thread_win32_thread_detach(void) {
}

thread_t *
u_thread_create_internal(thread_fn_t func,
  bool joinable,
  thread_prio_t prio,
  size_t stack_size) {
  thread_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_create_internal: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY ((
    ret->hdl = spawn_thread(
      "",
      func,
      pp_thread_get_atheos_priority(prio),
      stack_size,
      ret
    )) < 0)) {
    U_ERROR ("thread_t::u_thread_create_internal: spawn_thread() failed");
    u_free(ret);
    return NULL;
  }
  if (U_UNLIKELY (resume_thread(ret->hdl) != 0)) {
    U_ERROR ("thread_t::u_thread_create_internal: resume_thread() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  return ret;
}

void
u_thread_exit_internal(void) {
  exit_thread(0);
}

void
u_thread_wait_internal(thread_t *thread) {
  wait_for_thread(thread->hdl);
}

void
u_thread_free_internal(thread_t *thread) {
  u_free(thread);
}

void
u_thread_yield(void) {
  sched_yield();
}

bool
u_thread_set_priority(thread_t *thread,
  thread_prio_t prio) {
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  set_thread_priority(thread->hdl, pp_thread_get_atheos_priority(prio));
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_thread_current_id(void) {
  return (U_HANDLE) ((size_t) get_thread_id(NULL));
}

thread_key_t *
u_thread_local_new(destroy_fn_t free_func) {
  thread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_key_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->key = -1;
  ret->free_func = free_func;
  return ret;
}

void
u_thread_local_free(thread_key_t *key) {
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  u_free(key);
}

ptr_t
u_thread_get_local(thread_key_t *key) {
  int tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_LIKELY (tls_key >= 0)) {
    return get_tld(tls_key);
  }
  return NULL;
}

void
u_thread_set_local(thread_key_t *key,
  ptr_t value) {
  int tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (tls_key >= 0) {
    set_tld(tls_key, value);
  }
}

void
u_thread_replace_local(thread_key_t *key,
  ptr_t value) {
  int tls_key;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_UNLIKELY (tls_key < 0)) {
    return;
  }
  old_value = get_tld(tls_key);
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  set_tld(tls_key, value);
}
