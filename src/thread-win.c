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
#include "unic/thread.h"
#include "thread-private.h"

typedef HANDLE thread_hdl_t;

struct thread {
  thread_base_t base;
  thread_hdl_t hdl;
  thread_fn_t proxy;
};

struct thread_key {
  DWORD key_idx;
  destroy_fn_t free_func;
};

typedef struct PthreadDestructor_ PthreadDestructor;

struct PthreadDestructor_ {
  DWORD key_idx;
  destroy_fn_t free_func;
  PthreadDestructor *next;
};

static PthreadDestructor *volatile pp_thread_tls_destructors = NULL;

static mutex_t *pp_thread_tls_mutex = NULL;

static DWORD
pp_thread_get_tls_key(thread_key_t *key);

static uint_t __stdcall
pp_thread_win32_proxy(ptr_t data);

static DWORD
pp_thread_get_tls_key(thread_key_t *key) {
  DWORD tls_key = key->key_idx;
  if (U_LIKELY (tls_key != TLS_OUT_OF_INDEXES)) {
    return tls_key;
  }
  u_mutex_lock(pp_thread_tls_mutex);
  tls_key = key->key_idx;
  if (U_LIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
    PthreadDestructor *destr = NULL;
    tls_key = TlsAlloc();
    if (U_UNLIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
      U_ERROR ("thread_t::pp_thread_get_tls_key: TlsAlloc() failed");
      u_mutex_unlock(pp_thread_tls_mutex);
      return TLS_OUT_OF_INDEXES;
    }
    if (key->free_func != NULL) {
      if (U_UNLIKELY (
        (destr = u_malloc0(sizeof(PthreadDestructor))) == NULL)) {
        U_ERROR ("thread_t::pp_thread_get_tls_key: failed to allocate memory");
        if (U_UNLIKELY (TlsFree(tls_key) == 0))
          U_ERROR ("thread_t::pp_thread_get_tls_key: TlsFree() failed(1)");
        u_mutex_unlock(pp_thread_tls_mutex);
        return TLS_OUT_OF_INDEXES;
      }
      destr->key_idx = tls_key;
      destr->free_func = key->free_func;
      destr->next = pp_thread_tls_destructors;

      /* At the same time thread exit could be performed at there is no
       * lock for the global destructor list */
      if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange(
        (PVOID volatile *) &pp_thread_tls_destructors,
        (PVOID) destr->next,
        (PVOID) destr
      ) == false)) {
        U_ERROR (
          "thread_t::pp_thread_get_tls_key: u_atomic_pointer_compare_and_exchange() failed");
        if (U_UNLIKELY (TlsFree(tls_key) == 0))
          U_ERROR ("thread_t::pp_thread_get_tls_key: TlsFree() failed(2)");
        u_free(destr);
        u_mutex_unlock(pp_thread_tls_mutex);
        return TLS_OUT_OF_INDEXES;
      }
    }
    key->key_idx = tls_key;
  }
  u_mutex_unlock(pp_thread_tls_mutex);
  return tls_key;
}

static uint_t __stdcall
pp_thread_win32_proxy(ptr_t data) {
  thread_t *thread = data;
  thread->proxy(thread);
  _endthreadex(0);
  return 0;
}

void
u_thread_win32_thread_detach(void) {
  bool was_called;
  do {
    PthreadDestructor *destr;
    was_called = false;
    destr = (PthreadDestructor *) u_atomic_pointer_get(
      (const PVOID volatile *) &pp_thread_tls_destructors
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
u_thread_init_internal(void) {
  if (U_LIKELY (pp_thread_tls_mutex == NULL)) {
    pp_thread_tls_mutex = u_mutex_new();
  }
}

void
u_thread_shutdown_internal(void) {
  PthreadDestructor *destr;
  u_thread_win32_thread_detach();
  destr = pp_thread_tls_destructors;
  while (destr != NULL) {
    PthreadDestructor *next_destr = destr->next;
    TlsFree(destr->key_idx);
    u_free(destr);
    destr = next_destr;
  }
  pp_thread_tls_destructors = NULL;
  if (U_LIKELY (pp_thread_tls_mutex != NULL)) {
    u_mutex_free(pp_thread_tls_mutex);
    pp_thread_tls_mutex = NULL;
  }
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
  ret->proxy = func;
  if (U_UNLIKELY ((
    ret->hdl = (HANDLE) _beginthreadex(
      NULL,
      (uint_t) stack_size,
      pp_thread_win32_proxy,
      ret,
      CREATE_SUSPENDED,
      NULL
    )) == NULL)) {
    U_ERROR ("thread_t::u_thread_create_internal: _beginthreadex() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.joinable = joinable;
  u_thread_set_priority(ret, prio);
  if (U_UNLIKELY (ResumeThread(ret->hdl) == (DWORD) -1)) {
    U_ERROR ("thread_t::u_thread_create_internal: ResumeThread() failed");
    CloseHandle(ret->hdl);
    u_free(ret);
  }
  return ret;
}

void
u_thread_exit_internal(void) {
  _endthreadex(0);
}

void
u_thread_wait_internal(thread_t *thread) {
  if (U_UNLIKELY (
    (WaitForSingleObject(thread->hdl, INFINITE)) != WAIT_OBJECT_0))
    U_ERROR ("thread_t::u_thread_wait_internal: WaitForSingleObject() failed");
}

void
u_thread_free_internal(thread_t *thread) {
  CloseHandle(thread->hdl);
  u_free(thread);
}

void
u_thread_yield(void) {
  Sleep(0);
}

bool
u_thread_set_priority(thread_t *thread,
  thread_prio_t prio) {
  int native_prio;
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  switch (prio) {
    case U_THREAD_PRIORITY_IDLE:
      native_prio = THREAD_PRIORITY_IDLE;
      break;
    case U_THREAD_PRIORITY_LOWEST:
      native_prio = THREAD_PRIORITY_LOWEST;
      break;
    case U_THREAD_PRIORITY_LOW:
      native_prio = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case U_THREAD_PRIORITY_NORMAL:
      native_prio = THREAD_PRIORITY_NORMAL;
      break;
    case U_THREAD_PRIORITY_HIGH:
      native_prio = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case U_THREAD_PRIORITY_HIGHEST:
      native_prio = THREAD_PRIORITY_HIGHEST;
      break;
    case U_THREAD_PRIORITY_TIMECRITICAL:
      native_prio = THREAD_PRIORITY_TIME_CRITICAL;
      break;
    case U_THREAD_PRIORITY_INHERIT:
    default:
      native_prio = GetThreadPriority(GetCurrentThread());
      break;
  }
  if (U_UNLIKELY (SetThreadPriority(thread->hdl, native_prio) == 0)) {
    U_ERROR ("thread_t::u_thread_set_priority: SetThreadPriority() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_thread_current_id(void) {
  return (U_HANDLE) ((size_t) GetCurrentThreadId());
}

thread_key_t *
u_thread_local_new(destroy_fn_t free_func) {
  thread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_key_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_local_new: failed to allocate memory");
    return NULL;
  }
  ret->key_idx = TLS_OUT_OF_INDEXES;
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
  DWORD tls_idx;
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  tls_idx = pp_thread_get_tls_key(key);
  return tls_idx == TLS_OUT_OF_INDEXES ? NULL : TlsGetValue(tls_idx);
}

void
u_thread_set_local(thread_key_t *key,
  ptr_t value) {
  DWORD tls_idx;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_idx = pp_thread_get_tls_key(key);
  if (U_LIKELY (tls_idx != TLS_OUT_OF_INDEXES)) {
    if (U_UNLIKELY (TlsSetValue(tls_idx, value) == 0))
      U_ERROR ("thread_t::u_thread_set_local: TlsSetValue() failed");
  }
}

void
u_thread_replace_local(thread_key_t *key,
  ptr_t value) {
  DWORD tls_idx;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_idx = pp_thread_get_tls_key(key);
  if (U_UNLIKELY (tls_idx == TLS_OUT_OF_INDEXES)) {
    return;
  }
  old_value = TlsGetValue(tls_idx);
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  if (U_UNLIKELY (TlsSetValue(tls_idx, value) == 0))
    U_ERROR ("thread_t::u_thread_replace_local: TlsSetValue() failed");
}
