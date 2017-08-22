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

#include <atheos/semaphore.h>
#include <atheos/threads.h>

#include "p/condvar.h"
#include "p/spinlock.h"
#include "p/atomic.h"
#include "p/mem.h"
#include "p/string.h"

typedef struct _PCondThread {
  thread_id thread;
  struct _PCondThread *next;
} PCondThread;

struct condvar {
  spinlock_t *lock;
  PCondThread *wait_head;
  int_t wait_count;
};

condvar_t *
p_condvar_new(void) {
  condvar_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(condvar_t))) == NULL)) {
    P_ERROR ("condvar_t::p_condvar_new: failed to allocate memory");
    return NULL;
  }
  if ((ret->lock = p_spinlock_new()) == NULL) {
    P_ERROR ("condvar_t::p_condvar_new: failed to initialize");
    p_free(ret);
    return NULL;
  }
  return ret;
}

void
p_condvar_free(condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL)) {
    return;
  }
  if ((cond->wait_count > 0) || (cond->wait_head != NULL))
    P_WARNING (
      "condvar_t::p_condvar_free: destroying while threads are waiting");
  p_spinlock_free(cond->lock);
  p_free(cond);
}

bool
p_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  PCondThread *wait_thread;
  if (P_UNLIKELY (cond == NULL || mutex == NULL)) {
    return false;
  }
  if ((wait_thread = p_malloc0(sizeof(PCondThread))) == NULL) {
    P_ERROR ("condvar_t::p_condvar_wait: failed to allocate memory");
    return false;
  }
  wait_thread->thread = get_thread_id(NULL);
  wait_thread->next = NULL;
  if (p_spinlock_lock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_wait: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head != NULL) {
    cond->wait_head->next = wait_thread;
  } else {
    cond->wait_head = wait_thread;
  }
  p_atomic_int_inc((volatile int_t *) &cond->wait_count);
  if (p_spinlock_unlock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_wait: failed to unlock internal spinlock");
    return false;
  }
  if (p_mutex_unlock(mutex) != true) {
    P_ERROR ("condvar_t::p_condvar_wait: failed to unlock mutex");
    return false;
  }
  suspend_thread(wait_thread->thread);
  if (p_mutex_lock(mutex) != true) {
    P_ERROR ("condvar_t::p_condvar_wait: failed to lock mutex");
    return false;
  }
  return true;
}

bool
p_condvar_signal(condvar_t *cond) {
  PCondThread *wait_thread;
  thread_info thr_info;
  if (P_UNLIKELY (cond == NULL)) {
    return false;
  }
  if (p_spinlock_lock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_signal: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head == NULL) {
    if (p_spinlock_unlock(cond->lock) != true) {
      P_ERROR (
        "condvar_t::p_condvar_signal(1): failed to unlock internal spinlock");
      return false;
    } else {
      return true;
    }
  }
  wait_thread = cond->wait_head;
  cond->wait_head = wait_thread->next;
  p_atomic_int_add((volatile int_t *) &cond->wait_count, -1);
  if (p_spinlock_unlock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_signal(2): failed to unlock internal spinlock");
    return false;
  }
  memset(&thr_info, 0, sizeof(thr_info));
  while (get_thread_info(wait_thread->thread, &thr_info) == 0) {
    if (thr_info.ti_state != TS_READY) {
      break;
    }
  }
  resume_thread(wait_thread->thread);
  p_free(wait_thread);
  return true;
}

bool
p_condvar_broadcast(condvar_t *cond) {
  if (P_UNLIKELY (cond == NULL)) {
    return false;
  }
  PCondThread *cur_thread;
  PCondThread *next_thread;
  thread_info thr_info;
  if (p_spinlock_lock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_broadcast: failed to lock internal spinlock");
    return false;
  }
  if (cond->wait_head == NULL) {
    if (p_spinlock_unlock(cond->lock) != true) {
      P_ERROR (
        "condvar_t::p_condvar_broadcast(1): failed to unlock internal spinlock");
      return false;
    } else {
      return true;
    }
  }
  cur_thread = cond->wait_head;
  do {
    memset(&thr_info, 0, sizeof(thr_info));
    while (get_thread_info(cur_thread->thread, &thr_info) == 0) {
      if (thr_info.ti_state != TS_READY) {
        break;
      }
    }
    resume_thread(cur_thread->thread);
    next_thread = cur_thread->next;
    p_free(cur_thread);
    cur_thread = next_thread;
  } while (cur_thread != NULL);
  cond->wait_head = NULL;
  cond->wait_count = 0;
  if (p_spinlock_unlock(cond->lock) != true) {
    P_ERROR (
      "condvar_t::p_condvar_broadcast(2): failed to unlock internal spinlock");
    return false;
  }
  return true;
}

void
p_condvar_init(void) {
}

void
p_condvar_shutdown(void) {
}
