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

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>
#include <process.h>

#ifdef U_DEBUG
# undef U_DEBUG
#endif

#include "unic/mem.h"
#include "unic/atomic.h"
#include "unic/mutex.h"
#include "unic/thread.h"
#include "thread-private.h"

typedef TID thread_hdl_t;

struct thread {
  thread_base_t base;
  thread_hdl_t hdl;
  thread_fn_t proxy;
};

struct thread_key {
  PULONG key;
  destroy_fn_t free_func;
};

typedef struct PthreadDestructor_ PthreadDestructor;

struct PthreadDestructor_ {
  PULONG key;
  destroy_fn_t free_func;
  PthreadDestructor *next;
};

static PthreadDestructor *volatile pp_thread_tls_destructors = NULL;

static mutex_t *pp_thread_tls_mutex = NULL;

static void
pp_thread_get_os2_priority(thread_prio_t prio, PULONG thr_class,
  PLONG thr_level);

static PULONG
pp_thread_get_tls_key(thread_key_t *key);

static void
pp_thread_clean_destructors(void);

static void
pp_thread_os2_proxy(ptr_t data);

static void
pp_thread_get_os2_priority(thread_prio_t prio, PULONG thr_class,
  PLONG thr_level) {
  switch (prio) {
    case U_thread_PRIORITY_INHERIT: {
      APIRET ulrc;
      PTIB ptib = NULL;
      if (U_UNLIKELY (DosGetInfoBlocks(&ptib, NULL) != NO_ERROR)) {
        U_WARNING (
          "thread_t::pp_thread_get_os2_priority: DosGetInfoBlocks() failed");
        *thr_class = PRTYC_REGULAR;
        *thr_level = 0;
      } else {
        *thr_class = ((ptib->tib_ptib2->tib2_ulpri) >> 8) & 0x00FF;
        *thr_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;
      }
      return;
    }
    case U_thread_PRIORITY_IDLE: {
      *thr_class = PRTYC_IDLETIME;
      *thr_level = 0;
      return;
    }
    case U_thread_PRIORITY_LOWEST: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MINIMUM;
      return;
    }
    case U_thread_PRIORITY_LOW: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MINIMUM / 2;
      return;
    }
    case U_thread_PRIORITY_NORMAL: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = 0;
      return;
    }
    case U_thread_PRIORITY_HIGH: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MAXIMUM / 2;
      return;
    }
    case U_thread_PRIORITY_HIGHEST: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MAXIMUM;
      return;
    }
    case U_thread_PRIORITY_TIMECRITICAL: {
      *thr_class = PRTYC_TIMECRITICAL;
      *thr_level = 0;
      return;
    }
  }
}

static PULONG
pp_thread_get_tls_key(thread_key_t *key) {
  PULONG thread_key;
  thread_key = (PULONG) u_atomic_pointer_get((ptr_t) &key->key);
  if (U_LIKELY (thread_key != NULL)) {
    return thread_key;
  }
  u_mutex_lock(pp_thread_tls_mutex);
  thread_key = key->key;
  if (U_LIKELY (thread_key == NULL)) {
    PthreadDestructor *destr = NULL;
    if (key->free_func != NULL) {
      if (U_UNLIKELY (
        (destr = u_malloc0(sizeof(PthreadDestructor))) == NULL)) {
        U_ERROR ("thread_t::pp_thread_get_tls_key: failed to allocate memory");
        u_mutex_unlock(pp_thread_tls_mutex);
        return NULL;
      }
    }
    if (U_UNLIKELY (DosAllocThreadLocalMemory(1, &thread_key) != NO_ERROR)) {
      U_ERROR (
        "thread_t::pp_thread_get_tls_key: DosAllocThreadLocalMemory() failed");
      u_free(destr);
      u_mutex_unlock(pp_thread_tls_mutex);
      return NULL;
    }
    if (destr != NULL) {
      destr->key = thread_key;
      destr->free_func = key->free_func;
      destr->next = pp_thread_tls_destructors;

      /* At the same time thread exit could be performed at there is no
       * lock for the global destructor list */
      if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange(
        (void *volatile *) &pp_thread_tls_destructors,
        (void *) destr->next,
        (void *) destr
      ) == false)) {
        U_ERROR (
          "thread_t::pp_thread_get_tls_key: u_atomic_pointer_compare_and_exchange() failed");
        if (U_UNLIKELY (DosFreeThreadLocalMemory(thread_key) != NO_ERROR))
          U_ERROR (
            "thread_t::pp_thread_get_tls_key: DosFreeThreadLocalMemory() failed");
        u_free(destr);
        u_mutex_unlock(pp_thread_tls_mutex);
        return NULL;
      }
    }
    key->key = thread_key;
  }
  u_mutex_unlock(pp_thread_tls_mutex);
  return thread_key;
}

static void
pp_thread_clean_destructors(void) {
  bool was_called;
  do {
    PthreadDestructor *destr;
    was_called = false;
    destr = (PthreadDestructor *) u_atomic_pointer_get(
      (const void *volatile *) &pp_thread_tls_destructors
    );
    while (destr != NULL) {
      PULONG value;
      value = destr->key;
      if (value != NULL && ((ptr_t) *value) != NULL
        && destr->free_func != NULL) {
        *destr->key = (ULONG) NULL;
        destr->free_func((ptr_t) *value);
        was_called = true;
      }
      destr = destr->next;
    }
  } while (was_called);
}

static void
pp_thread_os2_proxy(ptr_t data) {
  thread_t *thread = data;
  thread->proxy(thread);
  pp_thread_clean_destructors();
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
  pp_thread_clean_destructors();
  destr = pp_thread_tls_destructors;
  while (destr != NULL) {
    PthreadDestructor *next_destr = destr->next;
    u_free(destr);
    destr = next_destr;
  }
  pp_thread_tls_destructors = NULL;
  if (U_LIKELY (pp_thread_tls_mutex != NULL)) {
    u_mutex_free(pp_thread_tls_mutex);
    pp_thread_tls_mutex = NULL;
  }
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
  ret->base.joinable = joinable;
  ret->proxy = func;
  if (U_UNLIKELY (
    (
      ret->hdl = _beginthread((void (*)(void *)) pp_thread_os2_proxy,
        NULL,
        (uint_t) stack_size,
        ret
      )) <= 0)) {
    U_ERROR ("thread_t::u_thread_create_internal: _beginthread() failed");
    u_free(ret);
    return NULL;
  }
  ret->base.prio = U_thread_PRIORITY_INHERIT;
  u_thread_set_priority(ret, prio);
  return ret;
}

void
u_thread_exit_internal(void) {
  pp_thread_clean_destructors();
  _endthread();
}

void
u_thread_wait_internal(thread_t *thread) {
  APIRET ulrc;
  while ((ulrc = DosWaitThread(&thread->hdl, DCWW_WAIT)) == ERROR_INTERRUPT);
  if (U_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_INVALID_THREADID))
    U_ERROR ("thread_t::u_thread_wait_internal: DosWaitThread() failed");
}

void
u_thread_free_internal(thread_t *thread) {
  u_free(thread);
}

void
u_thread_yield(void) {
  DosSleep(0);
}

bool
u_thread_set_priority(thread_t *thread,
  thread_prio_t prio) {
  APIRET ulrc;
  PTIB ptib = NULL;
  LONG cur_level;
  LONG new_level;
  ULONG new_class;
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  if (U_UNLIKELY (DosGetInfoBlocks(&ptib, NULL) != NO_ERROR)) {
    U_WARNING ("thread_t::u_thread_set_priority: DosGetInfoBlocks() failed");
    return false;
  }
  cur_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;
  pp_thread_get_os2_priority(prio, &new_class, &new_level);
  if (U_UNLIKELY (
    DosSetPriority(PRTYS_THREAD, new_class, new_level - cur_level, 0)
      != NO_ERROR)) {
    U_WARNING ("thread_t::u_thread_set_priority: DosSetPriority() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_thread_current_id(void) {
  return (U_HANDLE) (_gettid());
}

thread_key_t *
u_thread_local_new(destroy_fn_t free_func) {
  thread_key_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_key_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_local_new: failed to allocate memory");
    return NULL;
  }
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
  PULONG tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((tls_key = pp_thread_get_tls_key(key)) == NULL)) {
    return NULL;
  }
  return (ptr_t) *tls_key;
}

void
u_thread_set_local(thread_key_t *key,
  ptr_t value) {
  PULONG tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_LIKELY (tls_key != NULL)) {
    *tls_key = (ULONG) value;
  }
}

void
u_thread_replace_local(thread_key_t *key,
  ptr_t value) {
  PULONG tls_key;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_UNLIKELY (tls_key == NULL)) {
    return;
  }
  old_value = (ptr_t) *tls_key;
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  *tls_key = (ULONG) value;
}
