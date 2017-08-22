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

#ifdef P_DEBUG
# undef P_DEBUG
#endif

#include "p/mem.h"
#include "p/atomic.h"
#include "p/mutex.h"
#include "p/uthread.h"
#include "puthread-private.h"

typedef TID puthread_hdl;

struct uthread {
  PUThreadBase base;
  puthread_hdl hdl;
  uthread_fn_t proxy;
};

struct uthread_key {
  PULONG key;
  destroy_fn_t free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
  PULONG key;
  destroy_fn_t free_func;
  PUThreadDestructor *next;
};

static PUThreadDestructor *volatile pp_uthread_tls_destructors = NULL;

static mutex_t *pp_uthread_tls_mutex = NULL;

static void
pp_uthread_get_os2_priority(uthread_prio_t prio, PULONG thr_class,
  PLONG thr_level);

static PULONG
pp_uthread_get_tls_key(uthread_key_t *key);

static void
pp_uthread_clean_destructors(void);

static void
pp_uthread_os2_proxy(ptr_t data);

static void
pp_uthread_get_os2_priority(uthread_prio_t prio, PULONG thr_class,
  PLONG thr_level) {
  switch (prio) {
    case P_UTHREAD_PRIORITY_INHERIT: {
      APIRET ulrc;
      PTIB ptib = NULL;
      if (P_UNLIKELY (DosGetInfoBlocks(&ptib, NULL) != NO_ERROR)) {
        P_WARNING (
          "uthread_t::pp_uthread_get_os2_priority: DosGetInfoBlocks() failed");
        *thr_class = PRTYC_REGULAR;
        *thr_level = 0;
      } else {
        *thr_class = ((ptib->tib_ptib2->tib2_ulpri) >> 8) & 0x00FF;
        *thr_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;
      }
      return;
    }
    case P_UTHREAD_PRIORITY_IDLE: {
      *thr_class = PRTYC_IDLETIME;
      *thr_level = 0;
      return;
    }
    case P_UTHREAD_PRIORITY_LOWEST: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MINIMUM;
      return;
    }
    case P_UTHREAD_PRIORITY_LOW: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MINIMUM / 2;
      return;
    }
    case P_UTHREAD_PRIORITY_NORMAL: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = 0;
      return;
    }
    case P_UTHREAD_PRIORITY_HIGH: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MAXIMUM / 2;
      return;
    }
    case P_UTHREAD_PRIORITY_HIGHEST: {
      *thr_class = PRTYC_REGULAR;
      *thr_level = PRTYD_MAXIMUM;
      return;
    }
    case P_UTHREAD_PRIORITY_TIMECRITICAL: {
      *thr_class = PRTYC_TIMECRITICAL;
      *thr_level = 0;
      return;
    }
  }
}

static PULONG
pp_uthread_get_tls_key(uthread_key_t *key) {
  PULONG thread_key;
  thread_key = (PULONG) p_atomic_pointer_get((ptr_t) &key->key);
  if (P_LIKELY (thread_key != NULL)) {
    return thread_key;
  }
  p_mutex_lock(pp_uthread_tls_mutex);
  thread_key = key->key;
  if (P_LIKELY (thread_key == NULL)) {
    PUThreadDestructor *destr = NULL;
    if (key->free_func != NULL) {
      if (P_UNLIKELY (
        (destr = p_malloc0(sizeof(PUThreadDestructor))) == NULL)) {
        P_ERROR ("uthread_t::pp_uthread_get_tls_key: failed to allocate memory");
        p_mutex_unlock(pp_uthread_tls_mutex);
        return NULL;
      }
    }
    if (P_UNLIKELY (DosAllocThreadLocalMemory(1, &thread_key) != NO_ERROR)) {
      P_ERROR (
        "uthread_t::pp_uthread_get_tls_key: DosAllocThreadLocalMemory() failed");
      p_free(destr);
      p_mutex_unlock(pp_uthread_tls_mutex);
      return NULL;
    }
    if (destr != NULL) {
      destr->key = thread_key;
      destr->free_func = key->free_func;
      destr->next = pp_uthread_tls_destructors;

      /* At the same time thread exit could be performed at there is no
       * lock for the global destructor list */
      if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange(
        (void *volatile *) &pp_uthread_tls_destructors,
        (void *) destr->next,
        (void *) destr
      ) == false)) {
        P_ERROR (
          "uthread_t::pp_uthread_get_tls_key: p_atomic_pointer_compare_and_exchange() failed");
        if (P_UNLIKELY (DosFreeThreadLocalMemory(thread_key) != NO_ERROR))
          P_ERROR (
            "uthread_t::pp_uthread_get_tls_key: DosFreeThreadLocalMemory() failed");
        p_free(destr);
        p_mutex_unlock(pp_uthread_tls_mutex);
        return NULL;
      }
    }
    key->key = thread_key;
  }
  p_mutex_unlock(pp_uthread_tls_mutex);
  return thread_key;
}

static void
pp_uthread_clean_destructors(void) {
  bool was_called;
  do {
    PUThreadDestructor *destr;
    was_called = false;
    destr = (PUThreadDestructor *) p_atomic_pointer_get(
      (const void *volatile *) &pp_uthread_tls_destructors
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
pp_uthread_os2_proxy(ptr_t data) {
  uthread_t *thread = data;
  thread->proxy(thread);
  pp_uthread_clean_destructors();
}

void
p_uthread_init_internal(void) {
  if (P_LIKELY (pp_uthread_tls_mutex == NULL)) {
    pp_uthread_tls_mutex = p_mutex_new();
  }
}

void
p_uthread_shutdown_internal(void) {
  PUThreadDestructor *destr;
  pp_uthread_clean_destructors();
  destr = pp_uthread_tls_destructors;
  while (destr != NULL) {
    PUThreadDestructor *next_destr = destr->next;
    p_free(destr);
    destr = next_destr;
  }
  pp_uthread_tls_destructors = NULL;
  if (P_LIKELY (pp_uthread_tls_mutex != NULL)) {
    p_mutex_free(pp_uthread_tls_mutex);
    pp_uthread_tls_mutex = NULL;
  }
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
  ret->base.joinable = joinable;
  ret->proxy = func;
  if (P_UNLIKELY (
    (
      ret->hdl = _beginthread((void (*)(void *)) pp_uthread_os2_proxy,
        NULL,
        (uint_t) stack_size,
        ret
      )) <= 0)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: _beginthread() failed");
    p_free(ret);
    return NULL;
  }
  ret->base.prio = P_UTHREAD_PRIORITY_INHERIT;
  p_uthread_set_priority(ret, prio);
  return ret;
}

void
p_uthread_exit_internal(void) {
  pp_uthread_clean_destructors();
  _endthread();
}

void
p_uthread_wait_internal(uthread_t *thread) {
  APIRET ulrc;
  while ((ulrc = DosWaitThread(&thread->hdl, DCWW_WAIT)) == ERROR_INTERRUPT);
  if (P_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_INVALID_THREADID))
    P_ERROR ("uthread_t::p_uthread_wait_internal: DosWaitThread() failed");
}

void
p_uthread_free_internal(uthread_t *thread) {
  p_free(thread);
}

void
p_uthread_yield(void) {
  DosSleep(0);
}

bool
p_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  APIRET ulrc;
  PTIB ptib = NULL;
  LONG cur_level;
  LONG new_level;
  ULONG new_class;
  if (P_UNLIKELY (thread == NULL)) {
    return false;
  }
  if (P_UNLIKELY (DosGetInfoBlocks(&ptib, NULL) != NO_ERROR)) {
    P_WARNING ("uthread_t::p_uthread_set_priority: DosGetInfoBlocks() failed");
    return false;
  }
  cur_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;
  pp_uthread_get_os2_priority(prio, &new_class, &new_level);
  if (P_UNLIKELY (
    DosSetPriority(PRTYS_THREAD, new_class, new_level - cur_level, 0)
      != NO_ERROR)) {
    P_WARNING ("uthread_t::p_uthread_set_priority: DosSetPriority() failed");
    return false;
  }
  thread->base.prio = prio;
  return true;
}

P_HANDLE
p_uthread_current_id(void) {
  return (P_HANDLE) (_gettid());
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
  PULONG tls_key;
  if (P_UNLIKELY (key == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((tls_key = pp_uthread_get_tls_key(key)) == NULL)) {
    return NULL;
  }
  return (ptr_t) *tls_key;
}

void
p_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  PULONG tls_key;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_LIKELY (tls_key != NULL)) {
    *tls_key = (ULONG) value;
  }
}

void
p_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  PULONG tls_key;
  ptr_t old_value;
  if (P_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_uthread_get_tls_key(key);
  if (P_UNLIKELY (tls_key == NULL)) {
    return;
  }
  old_value = (ptr_t) *tls_key;
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  *tls_key = (ULONG) value;
}
