/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

/* Taken from "Strategies for Implementing POSIX Condition Variables on Win32"
 * by Douglas C. Schmidt and Irfan Pyarali.
 * See: http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
 * See: https://github.com/python/cpython/blob/master/Python/condvar.h
 */

#include "p/atomic.h"
#include "p/mem.h"
#include "p/condvar.h"

#include <stdlib.h>

typedef VOID (WINAPI
* InitializeConditionVariableFunc) (
ptr_t cv
);
typedef BOOL (WINAPI
* SleepConditionVariableCSFunc)    (
ptr_t cv, PCRITICAL_SECTION
cs,
DWORD ms
);
typedef VOID (WINAPI
* WakeConditionVariableFunc)       (
ptr_t cv
);
typedef VOID (WINAPI
* WakeAllConditionVariableFunc)    (
ptr_t cv
);

typedef bool (*PWin32CondInit)(p_condvar_t *cond);
typedef void     (*PWin32CondClose)(p_condvar_t *cond);
typedef bool (*PWin32CondWait)(p_condvar_t *cond, PMutex *mutex);
typedef bool (*PWin32CondSignal)(p_condvar_t *cond);
typedef bool (*PWin32CondBrdcast)(p_condvar_t *cond);

static PWin32CondInit pp_condvar_init_func = NULL;
static PWin32CondClose pp_condvar_close_func = NULL;
static PWin32CondWait pp_condvar_wait_func = NULL;
static PWin32CondSignal pp_condvar_signal_func = NULL;
static PWin32CondBrdcast pp_condvar_brdcast_func = NULL;

typedef struct PCondVariableVistaTable_ {
  InitializeConditionVariableFunc cv_init;
  SleepConditionVariableCSFunc cv_wait;
  WakeConditionVariableFunc cv_wake;
  WakeAllConditionVariableFunc cv_brdcast;
} PCondVariableVistaTable;

typedef struct PCondVariableXP_ {
  HANDLE waiters_sema;
  int_t waiters_count;
} PCondVariableXP;

struct p_condvar {
  ptr_t cv;
};

static PCondVariableVistaTable
  pp_condvar_vista_table = {NULL, NULL, NULL, NULL};

/* CONDITION_VARIABLE routines */
static bool pp_condvar_init_vista(p_condvar_t *cond);
static void pp_condvar_close_vista(p_condvar_t *cond);
static bool pp_condvar_wait_vista(p_condvar_t *cond, PMutex *mutex);
static bool pp_condvar_signal_vista(p_condvar_t *cond);
static bool pp_condvar_broadcast_vista(p_condvar_t *cond);

/* Windows XP emulation routines */
static bool pp_condvar_init_xp(p_condvar_t *cond);
static void pp_condvar_close_xp(p_condvar_t *cond);
static bool pp_condvar_wait_xp(p_condvar_t *cond, PMutex *mutex);
static bool pp_condvar_signal_xp(p_condvar_t *cond);
static bool pp_condvar_broadcast_xp(p_condvar_t *cond);

/* CONDITION_VARIABLE routines */

static bool
pp_condvar_init_vista(p_condvar_t *cond) {
  pp_condvar_vista_table.cv_init(cond);

  return true;
}

static void
pp_condvar_close_vista(p_condvar_t *cond) {
  P_UNUSED (cond);
}

static bool
pp_condvar_wait_vista(p_condvar_t *cond, PMutex *mutex) {
  return pp_condvar_vista_table.cv_wait(cond,
    (PCRITICAL_SECTION)
  mutex,
    INFINITE) != 0 ? true : false;
}

static bool
pp_condvar_signal_vista(p_condvar_t *cond) {
  pp_condvar_vista_table.cv_wake(cond);

  return true;
}

static bool
pp_condvar_broadcast_vista(p_condvar_t *cond) {
  pp_condvar_vista_table.cv_brdcast(cond);

  return true;
}

/* Windows XP emulation routines */

static bool
pp_condvar_init_xp(p_condvar_t *cond) {
  PCondVariableXP *cv_xp;

  if ((cond->cv = p_malloc0(sizeof(PCondVariableXP))) == NULL) {
    P_ERROR (
      "p_condvar_t::pp_condvar_init_xp: failed to allocate memory (internal)");
    return false;
  }

  cv_xp = ((PCondVariableXP *) cond->cv);

  cv_xp->waiters_count = 0;
  cv_xp->waiters_sema = CreateSemaphoreA(NULL, 0, MAXLONG, NULL);

  if (P_UNLIKELY (cv_xp->waiters_sema == NULL)) {
    P_ERROR (
      "p_condvar_t::pp_condvar_init_xp: failed to initialize semaphore");
    p_free(cond->cv);
    cond->cv = NULL;
    return false;
  }

  return true;
}

static void
pp_condvar_close_xp(p_condvar_t *cond) {
  CloseHandle(((PCondVariableXP *) cond->cv)->waiters_sema);
  p_free(cond->cv);
}

static bool
pp_condvar_wait_xp(p_condvar_t *cond, PMutex *mutex) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);
  DWORD wait;

  p_atomic_int_inc(&cv_xp->waiters_count);

  p_mutex_unlock(mutex);
  wait = WaitForSingleObjectEx(cv_xp->waiters_sema, INFINITE, false);
  p_mutex_lock(mutex);

  if (wait != WAIT_OBJECT_0)
    p_atomic_int_add(&cv_xp->waiters_count, -1);

  return wait == WAIT_OBJECT_0 ? true : false;
}

static bool
pp_condvar_signal_xp(p_condvar_t *cond) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);

  if (p_atomic_int_get(&cv_xp->waiters_count) > 0) {
    p_atomic_int_add(&cv_xp->waiters_count, -1);
    return ReleaseSemaphore(cv_xp->waiters_sema, 1, 0) != 0 ? true : false;
  }

  return true;
}

static bool
pp_condvar_broadcast_xp(p_condvar_t *cond) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);
  int_t waiters;

  waiters = p_atomic_int_get(&cv_xp->waiters_count);

  if (waiters > 0) {
    p_atomic_int_set(&cv_xp->waiters_count, 0);
    return ReleaseSemaphore(cv_xp->waiters_sema, waiters, 0) != 0 ? true
      : false;
  }

  return true;
}

P_API p_condvar_t *
p_condvar_new(void) {
  p_condvar_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_condvar_t))) == NULL)) {
    P_ERROR ("p_condvar_t::p_condvar_new: failed to allocate memory");
    return NULL;
  }

  if (P_UNLIKELY (pp_condvar_init_func(ret) != true)) {
    P_ERROR ("p_condvar_t::p_condvar_new: failed to initialize");
    p_free(ret);
    return NULL;
  }

  return ret;
}

P_API void
p_condvar_free(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return;

  pp_condvar_close_func(cond);
  p_free(cond);
}

P_API bool
p_condvar_wait(p_condvar_t *cond,
  PMutex *mutex) {
  if (P_UNLIKELY (cond == NULL || mutex == NULL))
    return false;

  return pp_condvar_wait_func(cond, mutex);
}

P_API bool
p_condvar_signal(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return false;

  return pp_condvar_signal_func(cond);
}

P_API bool
p_condvar_broadcast(p_condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL))
    return false;

  return pp_condvar_brdcast_func(cond);
}

void
p_condvar_init(void) {
  HMODULE hmodule;

  hmodule = GetModuleHandleA("kernel32.dll");

  if (P_UNLIKELY (hmodule == NULL)) {
    P_ERROR (
      "p_condvar_t::p_condvar_init: failed to load kernel32.dll module");
    return;
  }

  pp_condvar_vista_table.cv_init =
    (InitializeConditionVariableFunc) GetProcAddress(hmodule,
      "InitializeConditionVariable");

  if (P_LIKELY (pp_condvar_vista_table.cv_init != NULL)) {
    pp_condvar_vista_table.cv_wait =
      (SleepConditionVariableCSFunc) GetProcAddress(hmodule,
        "SleepConditionVariableCS");
    pp_condvar_vista_table.cv_wake =
      (WakeConditionVariableFunc) GetProcAddress(hmodule,
        "WakeConditionVariable");
    pp_condvar_vista_table.cv_brdcast =
      (WakeAllConditionVariableFunc) GetProcAddress(hmodule,
        "WakeAllConditionVariable");

    pp_condvar_init_func = pp_condvar_init_vista;
    pp_condvar_close_func = pp_condvar_close_vista;
    pp_condvar_wait_func = pp_condvar_wait_vista;
    pp_condvar_signal_func = pp_condvar_signal_vista;
    pp_condvar_brdcast_func = pp_condvar_broadcast_vista;
  } else {
    pp_condvar_init_func = pp_condvar_init_xp;
    pp_condvar_close_func = pp_condvar_close_xp;
    pp_condvar_wait_func = pp_condvar_wait_xp;
    pp_condvar_signal_func = pp_condvar_signal_xp;
    pp_condvar_brdcast_func = pp_condvar_broadcast_xp;
  }
}

void
p_condvar_shutdown(void) {
  memset(&pp_condvar_vista_table, 0,
    sizeof(pp_condvar_vista_table));

  pp_condvar_init_func = NULL;
  pp_condvar_close_func = NULL;
  pp_condvar_wait_func = NULL;
  pp_condvar_signal_func = NULL;
  pp_condvar_brdcast_func = NULL;
}
