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

#include <kernel/OS.h>
#include <kernel/scheduler.h>
#include <support/TLS.h>

#include "unic/mem.h"
#include "unic/atomic.h"
#include "unic/mutex.h"
#include "unic/uthread.h"
#include "unic/string.h"
#include "uthread-private.h"

typedef thread_id puthread_hdl;

struct uthread {
  PUThreadBase base;
  puthread_hdl hdl;
  uthread_fn_t proxy;
};

struct uthread_key {
  int key;
  destroy_fn_t free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
  int key_idx;
  destroy_fn_t free_func;
  PUThreadDestructor *next;
};

static PUThreadDestructor *volatile pp_uthread_tls_destructors = NULL;

static mutex_t *pp_uthread_tls_mutex = NULL;

static int
pp_uthread_get_beos_priority(uthread_prio_t prio);

static int
pp_uthread_get_tls_key(uthread_key_t *key);

static void
pp_uthread_clean_destructors(void);

static int
pp_uthread_beos_proxy(ptr_t data);

static int
pp_uthread_get_beos_priority(uthread_prio_t prio) {
  switch (prio) {
    case U_UTHREAD_PRIORITY_INHERIT: {
      thread_info thr_info;
      memset(&thr_info, 0, sizeof(thr_info));
      if (U_UNLIKELY (get_thread_info(find_thread(NULL), &thr_info) != B_OK)) {
        U_WARNING (
          "uthread_t::pp_uthread_get_beos_priority: failed to get thread info");
        return B_NORMAL_PRIORITY;
      } else {
        return thr_info.priority;
      }
    }
    case U_UTHREAD_PRIORITY_IDLE:
      return B_LOW_PRIORITY;
    case U_UTHREAD_PRIORITY_LOWEST:
      return B_NORMAL_PRIORITY / 4;
    case U_UTHREAD_PRIORITY_LOW:
      return B_NORMAL_PRIORITY / 2;
    case U_UTHREAD_PRIORITY_NORMAL:
      return B_NORMAL_PRIORITY;
    case U_UTHREAD_PRIORITY_HIGH:
      return B_DISPLAY_PRIORITY;
    case U_UTHREAD_PRIORITY_HIGHEST:
      return B_URGENT_DISPLAY_PRIORITY;
    case U_UTHREAD_PRIORITY_TIMECRITICAL:
      return B_REAL_TIME_PRIORITY;
  }
}

static int
pp_uthread_get_tls_key(uthread_key_t *key) {
  int thread_key;
  thread_key = u_atomic_int_get((const volatile int *) &key->key);
  if (U_LIKELY (thread_key >= 0)) {
    return thread_key;
  }
  u_mutex_lock(pp_uthread_tls_mutex);
  thread_key = key->key;
  if (U_LIKELY (thread_key == -1)) {
    PUThreadDestructor *destr = NULL;
    if (key->free_func != NULL) {
      if (U_UNLIKELY (
        (destr = u_malloc0(sizeof(PUThreadDestructor))) == NULL)) {
        U_ERROR ("uthread_t::pp_uthread_get_tls_key: failed to allocate memory");
        u_mutex_unlock(pp_uthread_tls_mutex);
        return -1;
      }
    }
    if (U_UNLIKELY ((thread_key = tls_allocate()) < 0)) {
      U_ERROR ("uthread_t::pp_uthread_get_tls_key: tls_allocate() failed");
      u_free(destr);
      u_mutex_unlock(pp_uthread_tls_mutex);
      return -1;
    }
    if (destr != NULL) {
      destr->key_idx = thread_key;
      destr->free_func = key->free_func;
      destr->next = pp_uthread_tls_destructors;

      /* At the same time thread exit could be performed at there is no
       * lock for the global destructor list */
      if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange(
        (void *volatile *) &pp_uthread_tls_destructors,
        (void *) destr->next,
        (void *) destr
      ) == false)) {
        U_ERROR (
          "uthread_t::pp_uthread_get_tls_key: u_atomic_pointer_compare_and_exchange() failed");
        u_free(destr);
        u_mutex_unlock(pp_uthread_tls_mutex);
        return -1;
      }
    }
    key->key = thread_key;
  }
  u_mutex_unlock(pp_uthread_tls_mutex);
  return thread_key;
}

static void
pp_uthread_clean_destructors(void) {
  bool was_called;
  do {
    PUThreadDestructor *destr;
    was_called = false;
    destr = (PUThreadDestructor *) u_atomic_pointer_get(
      (const void *volatile *) &pp_uthread_tls_destructors
    );
    while (destr != NULL) {
      ptr_t value;
      value = tls_get(destr->key_idx);
      if (value != NULL && destr->free_func != NULL) {
        tls_set(destr->key_idx, NULL);
        destr->free_func(value);
        was_called = true;
      }
      destr = destr->next;
    }
  } while (was_called);
}

static int
pp_uthread_beos_proxy(ptr_t data) {
  uthread_t *thread = data;
  thread->proxy(thread);
  pp_uthread_clean_destructors();
  return 0;
}

void
u_uthread_init_internal(void) {
  if (U_LIKELY (pp_uthread_tls_mutex == NULL)) {
    pp_uthread_tls_mutex = u_mutex_new();
  }
}

void
u_uthread_shutdown_internal(void) {
  PUThreadDestructor *destr;
  pp_uthread_clean_destructors();
  destr = pp_uthread_tls_destructors;
  while (destr != NULL) {
    PUThreadDestructor *next_destr = destr->next;
    u_free(destr);
    destr = next_destr;
  }
  pp_uthread_tls_destructors = NULL;
  if (U_LIKELY (pp_uthread_tls_mutex != NULL)) {
    u_mutex_free(pp_uthread_tls_mutex);
    pp_uthread_tls_mutex = NULL;
  }
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
  U_UNUSED (stack_size);
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  ret->proxy = func;
  if (U_UNLIKELY ((
    ret->hdl = spawn_thread((thread_func) pp_uthread_beos_proxy,
      "",
      pp_uthread_get_beos_priority(prio),
      ret
    )) < B_OK)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: spawn_thread() failed");
    u_free(ret);
    return NULL;
  }
  if (U_UNLIKELY (resume_thread(ret->hdl) != B_OK)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: resume_thread() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  return ret;
}

void
u_uthread_exit_internal(void) {
  pp_uthread_clean_destructors();
  exit_thread(0);
}

void
u_uthread_wait_internal(uthread_t *thread) {
  status_t exit_value;
  wait_for_thread(thread->hdl, &exit_value);
}

void
u_uthread_free_internal(uthread_t *thread) {
  u_free(thread);
}

void
u_uthread_yield(void) {
  snooze((bigtime_t) 0);
}

bool
u_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  if (set_thread_priority(thread->hdl, pp_uthread_get_beos_priority(prio))
    < B_OK) {
    U_ERROR (
      "uthread_t::u_uthread_create_internal: set_thread_priority() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_uthread_current_id(void) {
  return (U_HANDLE) ((size_t) find_thread(NULL));
}

uthread_key_t *
u_uthread_local_new(destroy_fn_t free_func) {
  uthread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_key_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->key = -1;
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
  int tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (U_LIKELY (tls_key >= 0)) {
    return tls_get(tls_key);
  }
  return NULL;
}

void
u_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  int tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (tls_key >= 0) {
    tls_set(tls_key, value);
  }
}

void
u_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  int tls_key;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (U_UNLIKELY (tls_key < 0)) {
    return;
  }
  old_value = tls_get(tls_key);
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  tls_set(tls_key, value);
}
