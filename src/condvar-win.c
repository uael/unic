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

#include "unic/atomic.h"
#include "unic/mem.h"
#include "unic/condvar.h"

typedef VOID (WINAPI
  *InitializeConditionVariableFunc)(
  ptr_t cv
);

typedef BOOL (WINAPI
  *SleepConditionVariableCSFunc)(
  ptr_t cv, PCRITICAL_SECTION
cs,
  DWORD ms
);

typedef VOID (WINAPI
  *WakeConditionVariableFunc)(
  ptr_t cv
);

typedef VOID (WINAPI
  *WakeAllConditionVariableFunc)(
  ptr_t cv
);

typedef bool (*PWin32CondInit)(condvar_t *cond);

typedef void     (*PWin32CondClose)(condvar_t *cond);

typedef bool (*PWin32CondWait)(condvar_t *cond, mutex_t *mutex);

typedef bool (*PWin32CondSignal)(condvar_t *cond);

typedef bool (*PWin32CondBrdcast)(condvar_t *cond);

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
  int waiters_count;
} PCondVariableXP;

struct condvar {
  ptr_t cv;
};

static PCondVariableVistaTable
  pp_condvar_vista_table = {NULL, NULL, NULL, NULL};

/* CONDITION_VARIABLE routines */
static bool
pp_condvar_init_vista(condvar_t *cond);

static void
pp_condvar_close_vista(condvar_t *cond);

static bool
pp_condvar_wait_vista(condvar_t *cond, mutex_t *mutex);

static bool
pp_condvar_signal_vista(condvar_t *cond);

static bool
pp_condvar_broadcast_vista(condvar_t *cond);

/* Windows XP emulation routines */
static bool
pp_condvar_init_xp(condvar_t *cond);

static void
pp_condvar_close_xp(condvar_t *cond);

static bool
pp_condvar_wait_xp(condvar_t *cond, mutex_t *mutex);

static bool
pp_condvar_signal_xp(condvar_t *cond);

static bool
pp_condvar_broadcast_xp(condvar_t *cond);

/* CONDITION_VARIABLE routines */

static bool
pp_condvar_init_vista(condvar_t *cond) {
  pp_condvar_vista_table.cv_init(cond);
  return true;
}

static void
pp_condvar_close_vista(condvar_t *cond) {
  U_UNUSED (cond);
}

static bool
pp_condvar_wait_vista(condvar_t *cond, mutex_t *mutex) {
  return pp_condvar_vista_table.cv_wait(
    cond,
    (PCRITICAL_SECTION)
      mutex,
    INFINITE
  ) != 0 ? true : false;
}

static bool
pp_condvar_signal_vista(condvar_t *cond) {
  pp_condvar_vista_table.cv_wake(cond);
  return true;
}

static bool
pp_condvar_broadcast_vista(condvar_t *cond) {
  pp_condvar_vista_table.cv_brdcast(cond);
  return true;
}

/* Windows XP emulation routines */

static bool
pp_condvar_init_xp(condvar_t *cond) {
  PCondVariableXP *cv_xp;
  if ((cond->cv = u_malloc0(sizeof(PCondVariableXP))) == NULL) {
    U_ERROR (
      "condvar_t::pp_condvar_init_xp: failed to allocate memory (internal)");
    return false;
  }
  cv_xp = ((PCondVariableXP *) cond->cv);
  cv_xp->waiters_count = 0;
  cv_xp->waiters_sema = CreateSemaphoreA(NULL, 0, MAXLONG, NULL);
  if (U_UNLIKELY (cv_xp->waiters_sema == NULL)) {
    U_ERROR (
      "condvar_t::pp_condvar_init_xp: failed to initialize semaphore");
    u_free(cond->cv);
    cond->cv = NULL;
    return false;
  }
  return true;
}

static void
pp_condvar_close_xp(condvar_t *cond) {
  CloseHandle(((PCondVariableXP *) cond->cv)->waiters_sema);
  u_free(cond->cv);
}

static bool
pp_condvar_wait_xp(condvar_t *cond, mutex_t *mutex) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);
  DWORD wait;
  u_atomic_int_inc(&cv_xp->waiters_count);
  u_mutex_unlock(mutex);
  wait = WaitForSingleObjectEx(cv_xp->waiters_sema, INFINITE, false);
  u_mutex_lock(mutex);
  if (wait != WAIT_OBJECT_0) {
    u_atomic_int_add(&cv_xp->waiters_count, -1);
  }
  return wait == WAIT_OBJECT_0 ? true : false;
}

static bool
pp_condvar_signal_xp(condvar_t *cond) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);
  if (u_atomic_int_get(&cv_xp->waiters_count) > 0) {
    u_atomic_int_add(&cv_xp->waiters_count, -1);
    return ReleaseSemaphore(cv_xp->waiters_sema, 1, 0) != 0 ? true : false;
  }
  return true;
}

static bool
pp_condvar_broadcast_xp(condvar_t *cond) {
  PCondVariableXP *cv_xp = ((PCondVariableXP *) cond->cv);
  int waiters;
  waiters = u_atomic_int_get(&cv_xp->waiters_count);
  if (waiters > 0) {
    u_atomic_int_set(&cv_xp->waiters_count, 0);
    return ReleaseSemaphore(cv_xp->waiters_sema, waiters, 0) != 0 ? true
      : false;
  }
  return true;
}

condvar_t *
u_condvar_new(void) {
  condvar_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(condvar_t))) == NULL)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY (pp_condvar_init_func(ret) != true)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to initialize");
    u_free(ret);
    return NULL;
  }
  return ret;
}

void
u_condvar_free(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return;
  }
  pp_condvar_close_func(cond);
  u_free(cond);
}

bool
u_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  if (U_UNLIKELY (cond == NULL || mutex == NULL)) {
    return false;
  }
  return pp_condvar_wait_func(cond, mutex);
}

bool
u_condvar_signal(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  return pp_condvar_signal_func(cond);
}

bool
u_condvar_broadcast(condvar_t *cond) {
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  return pp_condvar_brdcast_func(cond);
}

void
u_condvar_init(void) {
  HMODULE hmodule;
  hmodule = GetModuleHandleA("kernel32.dll");
  if (U_UNLIKELY (hmodule == NULL)) {
    U_ERROR (
      "condvar_t::u_condvar_init: failed to load kernel32.dll module");
    return;
  }
  pp_condvar_vista_table.cv_init =
    (InitializeConditionVariableFunc) GetProcAddress(
      hmodule,
      "InitializeConditionVariable"
    );
  if (U_LIKELY (pp_condvar_vista_table.cv_init != NULL)) {
    pp_condvar_vista_table.cv_wait =
      (SleepConditionVariableCSFunc) GetProcAddress(
        hmodule,
        "SleepConditionVariableCS"
      );
    pp_condvar_vista_table.cv_wake =
      (WakeConditionVariableFunc) GetProcAddress(
        hmodule,
        "WakeConditionVariable"
      );
    pp_condvar_vista_table.cv_brdcast =
      (WakeAllConditionVariableFunc) GetProcAddress(
        hmodule,
        "WakeAllConditionVariable"
      );
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
u_condvar_shutdown(void) {
  memset(
    &pp_condvar_vista_table, 0,
    sizeof(pp_condvar_vista_table));
  pp_condvar_init_func = NULL;
  pp_condvar_close_func = NULL;
  pp_condvar_wait_func = NULL;
  pp_condvar_signal_func = NULL;
  pp_condvar_brdcast_func = NULL;
}
