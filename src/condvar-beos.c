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

#include "unic/condvar.h"
#include "unic/spinlock.h"
#include "unic/atomic.h"
#include "unic/string.h"
#include "unic/mem.h"

typedef struct _PCondThread {
  thread_id thread;
  struct _PCondThread *next;
} PCondThread;

struct condvar {
  spinlock_t *lock;
  PCondThread *wait_head;
  int wait_count;
};

condvar_t *
u_condvar_new(void) {
  condvar_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(condvar_t))) == NULL)) {
    U_ERROR ("condvar_t::u_condvar_new: failed to allocate memory");
    return NULL;
  }
  if ((ret->lock = u_spinlock_new()) == NULL) {
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
  if ((cond->wait_count > 0) || (cond->wait_head != NULL))
    U_WARNING (
      "condvar_t::u_condvar_free: destroying while threads are waiting");
  u_spinlock_free(cond->lock);
  u_free(cond);
}

bool
u_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  PCondThread *wait_thread;
  if (U_UNLIKELY (cond == NULL || mutex == NULL)) {
    return false;
  }
  if ((wait_thread = u_malloc0(sizeof(PCondThread))) == NULL) {
    U_ERROR ("condvar_t::u_condvar_wait: failed to allocate memory");
    return false;
  }
  wait_thread->thread = find_thread(NULL);
  wait_thread->next = NULL;
  if (u_spinlock_lock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_wait: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head != NULL) {
    cond->wait_head->next = wait_thread;
  } else {
    cond->wait_head = wait_thread;
  }
  u_atomic_int_inc((volatile int *) &cond->wait_count);
  if (u_spinlock_unlock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_wait: failed to unlock internal spinlock");
    return false;
  }
  if (u_mutex_unlock(mutex) != true) {
    U_ERROR ("condvar_t::u_condvar_wait: failed to unlock mutex");
    return false;
  }
  suspend_thread(wait_thread->thread);
  if (u_mutex_lock(mutex) != true) {
    U_ERROR ("condvar_t::u_condvar_wait: failed to lock mutex");
    return false;
  }
  return true;
}

bool
u_condvar_signal(condvar_t *cond) {
  PCondThread *wait_thread;
  thread_info thr_info;
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (u_spinlock_lock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_signal: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head == NULL) {
    if (u_spinlock_unlock(cond->lock) != true) {
      U_ERROR (
        "condvar_t::u_condvar_signal(1): failed to unlock internal spinlock");
      return false;
    } else {
      return true;
    }
  }
  wait_thread = cond->wait_head;
  cond->wait_head = wait_thread->next;
  u_atomic_int_add((volatile int *) &cond->wait_count, -1);
  if (u_spinlock_unlock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_signal(2): failed to unlock internal spinlock");
    return false;
  }
  memset(&thr_info, 0, sizeof(thr_info));
  while (get_thread_info(wait_thread->thread, &thr_info) == B_OK) {
    if (thr_info.state != B_THREAD_READY) {
      break;
    }
  }
  resume_thread(wait_thread->thread);
  u_free(wait_thread);
  return true;
}

bool
u_condvar_broadcast(condvar_t *cond) {
  PCondThread *cur_thread;
  PCondThread *next_thread;
  thread_info thr_info;
  if (U_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (u_spinlock_lock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_broadcast: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head == NULL) {
    if (u_spinlock_unlock(cond->lock) != true) {
      U_ERROR (
        "condvar_t::u_condvar_broadcast(1): failed to unlock internal spinlock");
      return false;
    } else {
      return true;
    }
  }
  cur_thread = cond->wait_head;
  do {
    memset(&thr_info, 0, sizeof(thr_info));
    while (get_thread_info(cur_thread->thread, &thr_info) == B_OK) {
      if (thr_info.state != B_THREAD_READY) {
        break;
      }
    }
    resume_thread(cur_thread->thread);
    next_thread = cur_thread->next;
    u_free(cur_thread);
    cur_thread = next_thread;
  } while (cur_thread != NULL);
  cond->wait_head = NULL;
  cond->wait_count = 0;
  if (u_spinlock_unlock(cond->lock) != true) {
    U_ERROR (
      "condvar_t::u_condvar_broadcast(2): failed to unlock internal spinlock");
    return false;
  }
  return true;
}

void
u_condvar_init(void) {
}

void
u_condvar_shutdown(void) {
}
