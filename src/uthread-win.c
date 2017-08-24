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

#include <process.h>

#include "unic/mem.h"
#include "unic/mutex.h"
#include "unic/atomic.h"
#include "unic/uthread.h"
#include "uthread-private.h"

typedef HANDLE puthread_hdl;

struct uthread {
  PUThreadBase base;
  puthread_hdl hdl;
  uthread_fn_t proxy;
};

struct uthread_key {
  DWORD key_idx;
  destroy_fn_t free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
  DWORD key_idx;
  destroy_fn_t free_func;
  PUThreadDestructor *next;
};

static PUThreadDestructor *volatile pp_uthread_tls_destructors = NULL;

static mutex_t *pp_uthread_tls_mutex = NULL;

static DWORD
pp_uthread_get_tls_key(uthread_key_t *key);

static uint_t __stdcall
pp_uthread_win32_proxy(ptr_t data);

static DWORD
pp_uthread_get_tls_key(uthread_key_t *key) {
  DWORD tls_key = key->key_idx;
  if (U_LIKELY (tls_key != TLS_OUT_OF_INDEXES)) {
    return tls_key;
  }
  u_mutex_lock(pp_uthread_tls_mutex);
  tls_key = key->key_idx;
  if (U_LIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
    PUThreadDestructor *destr = NULL;
    tls_key = TlsAlloc();
    if (U_UNLIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
      U_ERROR ("uthread_t::pp_uthread_get_tls_key: TlsAlloc() failed");
      u_mutex_unlock(pp_uthread_tls_mutex);
      return TLS_OUT_OF_INDEXES;
    }
    if (key->free_func != NULL) {
      if (U_UNLIKELY (
        (destr = u_malloc0(sizeof(PUThreadDestructor))) == NULL)) {
        U_ERROR ("uthread_t::pp_uthread_get_tls_key: failed to allocate memory");
        if (U_UNLIKELY (TlsFree(tls_key) == 0))
          U_ERROR ("uthread_t::pp_uthread_get_tls_key: TlsFree() failed(1)");
        u_mutex_unlock(pp_uthread_tls_mutex);
        return TLS_OUT_OF_INDEXES;
      }
      destr->key_idx = tls_key;
      destr->free_func = key->free_func;
      destr->next = pp_uthread_tls_destructors;

      /* At the same time thread exit could be performed at there is no
       * lock for the global destructor list */
      if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange(
        (PVOID volatile *) &pp_uthread_tls_destructors,
        (PVOID) destr->next,
        (PVOID) destr
      ) == false)) {
        U_ERROR (
          "uthread_t::pp_uthread_get_tls_key: u_atomic_pointer_compare_and_exchange() failed");
        if (U_UNLIKELY (TlsFree(tls_key) == 0))
          U_ERROR ("uthread_t::pp_uthread_get_tls_key: TlsFree() failed(2)");
        u_free(destr);
        u_mutex_unlock(pp_uthread_tls_mutex);
        return TLS_OUT_OF_INDEXES;
      }
    }
    key->key_idx = tls_key;
  }
  u_mutex_unlock(pp_uthread_tls_mutex);
  return tls_key;
}

static uint_t __stdcall
pp_uthread_win32_proxy(ptr_t data) {
  uthread_t *thread = data;
  thread->proxy(thread);
  _endthreadex(0);
  return 0;
}

void
u_uthread_win32_thread_detach(void) {
  bool was_called;
  do {
    PUThreadDestructor *destr;
    was_called = false;
    destr = (PUThreadDestructor *) u_atomic_pointer_get(
      (const PVOID volatile *) &pp_uthread_tls_destructors
    );
    while (destr != NULL) {
      ptr_t value;
      value = TlsGetValue(destr->key_idx);
      if (value != NULL && destr->free_func != NULL) {
        TlsSetValue(destr->key_idx, NULL);
        destr->free_func(value);
        was_called = true;
      }
      destr = destr->next;
    }
  } while (was_called);
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
  u_uthread_win32_thread_detach();
  destr = pp_uthread_tls_destructors;
  while (destr != NULL) {
    PUThreadDestructor *next_destr = destr->next;
    TlsFree(destr->key_idx);
    u_free(destr);
    destr = next_destr;
  }
  pp_uthread_tls_destructors = NULL;
  if (U_LIKELY (pp_uthread_tls_mutex != NULL)) {
    u_mutex_free(pp_uthread_tls_mutex);
    pp_uthread_tls_mutex = NULL;
  }
}

uthread_t *
u_uthread_create_internal(uthread_fn_t func,
  bool joinable,
  uthread_prio_t prio,
  size_t stack_size) {
  uthread_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  ret->proxy = func;
  if (U_UNLIKELY ((
    ret->hdl = (HANDLE) _beginthreadex(
      NULL,
      (uint_t) stack_size,
      pp_uthread_win32_proxy,
      ret,
      CREATE_SUSPENDED,
      NULL
    )) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: _beginthreadex() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  u_uthread_set_priority(ret, prio);
  if (U_UNLIKELY (ResumeThread(ret->hdl) == (DWORD) -1)) {
    U_ERROR ("uthread_t::u_uthread_create_internal: ResumeThread() failed");
    CloseHandle(ret->hdl);
    u_free(ret);
  }
  return ret;
}

void
u_uthread_exit_internal(void) {
  _endthreadex(0);
}

void
u_uthread_wait_internal(uthread_t *thread) {
  if (U_UNLIKELY (
    (WaitForSingleObject(thread->hdl, INFINITE)) != WAIT_OBJECT_0))
    U_ERROR ("uthread_t::u_uthread_wait_internal: WaitForSingleObject() failed");
}

void
u_uthread_free_internal(uthread_t *thread) {
  CloseHandle(thread->hdl);
  u_free(thread);
}

void
u_uthread_yield(void) {
  Sleep(0);
}

bool
u_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  int native_prio;
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  switch (prio) {
    case U_UTHREAD_PRIORITY_IDLE:
      native_prio = THREAD_PRIORITY_IDLE;
      break;
    case U_UTHREAD_PRIORITY_LOWEST:
      native_prio = THREAD_PRIORITY_LOWEST;
      break;
    case U_UTHREAD_PRIORITY_LOW:
      native_prio = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case U_UTHREAD_PRIORITY_NORMAL:
      native_prio = THREAD_PRIORITY_NORMAL;
      break;
    case U_UTHREAD_PRIORITY_HIGH:
      native_prio = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case U_UTHREAD_PRIORITY_HIGHEST:
      native_prio = THREAD_PRIORITY_HIGHEST;
      break;
    case U_UTHREAD_PRIORITY_TIMECRITICAL:
      native_prio = THREAD_PRIORITY_TIME_CRITICAL;
      break;
    case U_UTHREAD_PRIORITY_INHERIT:
    default:
      native_prio = GetThreadPriority(GetCurrentThread());
      break;
  }
  if (U_UNLIKELY (SetThreadPriority(thread->hdl, native_prio) == 0)) {
    U_ERROR ("uthread_t::u_uthread_set_priority: SetThreadPriority() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_uthread_current_id(void) {
  return (U_HANDLE) ((size_t) GetCurrentThreadId());
}

uthread_key_t *
u_uthread_local_new(destroy_fn_t free_func) {
  uthread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(uthread_key_t))) == NULL)) {
    U_ERROR ("uthread_t::u_uthread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->key_idx = TLS_OUT_OF_INDEXES;
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
  DWORD tls_idx;
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  tls_idx = pp_uthread_get_tls_key(key);
  return tls_idx == TLS_OUT_OF_INDEXES ? NULL : TlsGetValue(tls_idx);
}

void
u_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  DWORD tls_idx;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_idx = pp_uthread_get_tls_key(key);
  if (U_LIKELY (tls_idx != TLS_OUT_OF_INDEXES)) {
    if (U_UNLIKELY (TlsSetValue(tls_idx, value) == 0))
      U_ERROR ("uthread_t::u_uthread_set_local: TlsSetValue() failed");
  }
}

void
u_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  DWORD tls_idx;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_idx = pp_uthread_get_tls_key(key);
  if (U_UNLIKELY (tls_idx == TLS_OUT_OF_INDEXES)) {
    return;
  }
  old_value = TlsGetValue(tls_idx);
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  if (U_UNLIKELY (TlsSetValue(tls_idx, value) == 0))
    U_ERROR ("uthread_t::u_uthread_replace_local: TlsSetValue() failed");
}
