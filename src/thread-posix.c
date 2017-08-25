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

#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "unic/mem.h"
#include "unic/atomic.h"
#include "unic/thread.h"
#include "unic/string.h"
#include "thread-private.h"

#ifdef U_OS_SCO
# include "unic/pmutex.h"
#endif

#ifdef UNIC_HAS_POSIX_SCHEDULING
# ifndef U_OS_VMS
# endif
#endif

/* Some systems without native pthreads may lack some of the constants,
 * leave them zero as we are not going to use them anyway */

#ifndef PTHREAD_CREATE_JOINABLE
# define PTHREAD_CREATE_JOINABLE  0
#endif
#ifndef PTHREAD_CREATE_DETACHED
# define PTHREAD_CREATE_DETACHED  0
#endif
#ifdef UNIC_HAS_POSIX_SCHEDULING
# ifndef PTHREAD_INHERIT_SCHED
#   define PTHREAD_INHERIT_SCHED 0
# endif

# ifndef PTHREAD_EXPLICIT_SCHED
#   define PTHREAD_EXPLICIT_SCHED 0
# endif

/* Old Linux kernels may lack a definition */
# if defined (U_OS_LINUX) && !defined (SCHED_IDLE)
#   define SCHED_IDLE 5
# endif
#endif

typedef pthread_t thread_hdl_t;

struct thread {
  thread_base_t base;
  thread_hdl_t hdl;
};

struct thread_key {
  pthread_key_t *key;
  destroy_fn_t free_func;
};
#ifdef U_OS_SCO
static mutex_t *pp_thread_tls_mutex = NULL;
#endif
#ifdef UNIC_HAS_POSIX_SCHEDULING
static bool pp_thread_get_unix_priority(thread_prio_t prio,
  int *sched_policy, int *sched_priority);
#endif

static pthread_key_t *
pp_thread_get_tls_key(thread_key_t *key);

#ifdef UNIC_HAS_POSIX_SCHEDULING
static bool
pp_thread_get_unix_priority(thread_prio_t prio, int *sched_policy,
  int *sched_priority) {
  int lowBound, upperBound;
  int prio_min, prio_max;
  int native_prio;

#ifdef SCHED_IDLE
  if (prio == U_thread_PRIORITY_IDLE) {
    *sched_policy = SCHED_IDLE;
    *sched_priority = 0;
    return true;
  }

  lowBound = (int) U_thread_PRIORITY_LOWEST;
#else
  lowBound = (int) U_thread_PRIORITY_IDLE;
#endif
  upperBound = (int) U_thread_PRIORITY_TIMECRITICAL;

  prio_min = sched_get_priority_min(*sched_policy);
  prio_max = sched_get_priority_max(*sched_policy);

  if (U_UNLIKELY (prio_min == -1 || prio_max == -1))
    return false;

  native_prio =
    ((int) prio - lowBound) * (prio_max - prio_min) / upperBound + prio_min;

  if (U_UNLIKELY (native_prio > prio_max))
    native_prio = prio_max;

  if (U_UNLIKELY (native_prio < prio_min))
    native_prio = prio_min;

  *sched_priority = native_prio;

  return true;
}
#endif

static pthread_key_t *
pp_thread_get_tls_key(thread_key_t *key) {
  pthread_key_t *thread_key;
  thread_key = (pthread_key_t *) u_atomic_pointer_get((ptr_t) &key->key);
  if (U_LIKELY (thread_key != NULL)) {
    return thread_key;
  }
#ifdef U_OS_SCO
  u_mutex_lock (pp_thread_tls_mutex);

  thread_key = key->key;

  if (U_LIKELY (thread_key == NULL)) {
#endif
  if (U_UNLIKELY ((thread_key = u_malloc0(sizeof(pthread_key_t))) == NULL)) {
    U_ERROR ("thread_t::pp_thread_get_tls_key: failed to allocate memory");
#ifdef U_OS_SCO
    u_mutex_unlock (pp_thread_tls_mutex);
#endif
    return NULL;
  }
  if (U_UNLIKELY (pthread_key_create(thread_key, key->free_func) != 0)) {
    U_ERROR ("thread_t::pp_thread_get_tls_key: pthread_key_create() failed");
    u_free(thread_key);
#ifdef U_OS_SCO
    u_mutex_unlock (pp_thread_tls_mutex);
#endif
    return NULL;
  }
#ifndef U_OS_SCO
  if (U_UNLIKELY (u_atomic_pointer_compare_and_exchange((ptr_t) &key->key,
    NULL,
    (ptr_t) thread_key
  ) == false)) {
    if (U_UNLIKELY (pthread_key_delete(*thread_key) != 0)) {
      U_ERROR ("thread_t::pp_thread_get_tls_key: pthread_key_delete() failed");
      u_free(thread_key);
      return NULL;
    }
    u_free(thread_key);
    thread_key = key->key;
  }
#else
  key->key = thread_key;
}

u_mutex_unlock (pp_thread_tls_mutex);
#endif
  return thread_key;
}

void
u_thread_init_internal(void) {
#ifdef U_OS_SCO
  if (U_LIKELY (pp_thread_tls_mutex == NULL))
    pp_thread_tls_mutex = u_mutex_new ();
#endif
}

void
u_thread_shutdown_internal(void) {
#ifdef U_OS_SCO
  if (U_LIKELY (pp_thread_tls_mutex != NULL)) {
    u_mutex_free (pp_thread_tls_mutex);
    pp_thread_tls_mutex = NULL;
  }
#endif
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
  pthread_attr_t attr;
  int create_code;
#ifdef UNIC_HAS_POSIX_SCHEDULING
  struct sched_param sched;
  int native_prio;
  int sched_policy;
#endif
#if defined (UNIC_HAS_POSIX_STACKSIZE) && defined (_SC_THREAD_STACK_MIN)
  long min_stack;
#endif
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_create_internal: failed to allocate memory");
    return NULL;
  }
  ret->base.joinable = joinable;
  if (U_UNLIKELY (pthread_attr_init(&attr) != 0)) {
    U_ERROR ("thread_t::u_thread_create_internal: pthread_attr_init() failed");
    u_free(ret);
    return NULL;
  }
  if (U_UNLIKELY (pthread_attr_setdetachstate(
    &attr,
    joinable ? PTHREAD_CREATE_JOINABLE
      : PTHREAD_CREATE_DETACHED
  ) != 0)) {
    U_ERROR (
      "thread_t::u_thread_create_internal: pthread_attr_setdetachstate() failed");
    pthread_attr_destroy(&attr);
    u_free(ret);
    return NULL;
  }
#ifdef UNIC_HAS_POSIX_SCHEDULING
  if (prio == U_thread_PRIORITY_INHERIT) {
    if (U_UNLIKELY (
      pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED) != 0))
      U_WARNING (
        "thread_t::u_thread_create_internal: pthread_attr_setinheritsched() failed");
  } else {
    if (U_LIKELY (pthread_attr_getschedpolicy(&attr, &sched_policy) == 0)) {
      if (U_LIKELY (pp_thread_get_unix_priority(prio,
        &sched_policy,
        &native_prio) == true)) {
        memset(&sched, 0, sizeof(sched));
        sched.sched_priority = native_prio;

        if (U_LIKELY (
          pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0 ||
            pthread_attr_setschedpolicy(&attr, sched_policy) != 0 ||
            pthread_attr_setschedparam(&attr, &sched) != 0))
          U_WARNING (
            "thread_t::u_thread_create_internal: failed to set priority");
      } else
        U_WARNING (
          "thread_t::u_thread_create_internal: pp_thread_get_unix_priority() failed");
    } else
      U_WARNING (
        "thread_t::u_thread_create_internal: pthread_attr_getschedpolicy() failed");
  }
#endif
#ifdef UNIC_HAS_POSIX_STACKSIZE
# ifdef _SC_THREAD_STACK_MIN
  if (stack_size > 0) {
    min_stack = (long) sysconf(_SC_THREAD_STACK_MIN);

    if (U_LIKELY (min_stack > 0)) {
      if (U_UNLIKELY (stack_size < (size_t) min_stack))
        stack_size = (size_t) min_stack;
    } else
      U_WARNING (
        "thread_t::u_thread_create_internal: sysconf() with _SC_THREAD_STACK_MIN failed");

    if (U_UNLIKELY (pthread_attr_setstacksize(&attr, stack_size) != 0))
      U_WARNING (
        "thread_t::u_thread_create_internal: pthread_attr_setstacksize() failed");
  }
# endif
#endif
  create_code = pthread_create(&ret->hdl, &attr, func, ret);
#ifdef EPERM
  if (create_code == EPERM) {
# ifdef UNIC_HAS_POSIX_SCHEDULING
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
# endif
    create_code = pthread_create(&ret->hdl, &attr, func, ret);
  }
#endif
  if (U_UNLIKELY (create_code != 0)) {
    U_ERROR ("thread_t::u_thread_create_internal: pthread_create() failed");
    pthread_attr_destroy(&attr);
    u_free(ret);
    return NULL;
  }
  ret->base.prio = prio;
  pthread_attr_destroy(&attr);
  return ret;
}

void
u_thread_exit_internal(void) {
  pthread_exit(U_INT_TO_POINTER (0));
}

void
u_thread_wait_internal(thread_t *thread) {
  if (U_UNLIKELY (pthread_join(thread->hdl, NULL) != 0))
    U_ERROR ("thread_t::u_thread_wait_internal: pthread_join() failed");
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
#ifdef UNIC_HAS_POSIX_SCHEDULING
  struct sched_param sched;
  int policy;
  int native_prio;
#endif
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
#ifdef UNIC_HAS_POSIX_SCHEDULING
  if (U_UNLIKELY (pthread_getschedparam(thread->hdl, &policy, &sched) != 0)) {
    U_ERROR (
      "thread_t::u_thread_set_priority: pthread_getschedparam() failed");
    return false;
  }

  if (U_UNLIKELY (
    pp_thread_get_unix_priority(prio, &policy, &native_prio) == false)) {
    U_ERROR (
      "thread_t::u_thread_set_priority: pp_thread_get_unix_priority() failed");
    return false;
  }

  memset(&sched, 0, sizeof(sched));
  sched.sched_priority = native_prio;

  if (U_UNLIKELY (pthread_setschedparam(thread->hdl, policy, &sched) != 0)) {
    U_ERROR (
      "thread_t::u_thread_set_priority: pthread_setschedparam() failed");
    return false;
  }
#endif
  thread->base.prio = prio;
  return true;
}

U_HANDLE
u_thread_current_id(void) {
  return (U_HANDLE) ((size_t) pthread_self());
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
  pthread_key_t *tls_key;
#ifdef U_OS_SCO
  ptr_t value;
#endif
  if (U_UNLIKELY (key == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((tls_key = pp_thread_get_tls_key(key)) == NULL)) {
    return NULL;
  }
#ifdef U_OS_SCO
  if (U_UNLIKELY (pthread_getspecific (*tls_key, &value) != 0))
    return NULL;

  return value;
#else
  return pthread_getspecific(*tls_key);
#endif
}

void
u_thread_set_local(thread_key_t *key,
  ptr_t value) {
  pthread_key_t *tls_key;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_LIKELY (tls_key != NULL)) {
    if (U_UNLIKELY (pthread_setspecific(*tls_key, value) != 0))
      U_ERROR ("thread_t::u_thread_set_local: pthread_setspecific() failed");
  }
}

void
u_thread_replace_local(thread_key_t *key,
  ptr_t value) {
  pthread_key_t *tls_key;
  ptr_t old_value;
  if (U_UNLIKELY (key == NULL)) {
    return;
  }
  tls_key = pp_thread_get_tls_key(key);
  if (U_UNLIKELY (tls_key == NULL)) {
    return;
  }
#ifdef U_OS_SCO
  if (U_UNLIKELY (pthread_getspecific (*tls_key, &old_value) != 0))
    return;
#else
  old_value = pthread_getspecific(*tls_key);
#endif
  if (old_value != NULL && key->free_func != NULL) {
    key->free_func(old_value);
  }
  if (U_UNLIKELY (pthread_setspecific(*tls_key, value) != 0))
    U_ERROR ("thread_t::u_thread_replace_local: pthread_setspecific() failed");
}
