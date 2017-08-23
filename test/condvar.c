/*
 * Copyright (C) 2013-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "cute.h"
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PCONDTEST_MAX_QUEUE 10

static int thread_wakeups = 0;
static int thread_queue = 0;
static condvar_t *queue_empty_cond = NULL;
static condvar_t *queue_full_cond = NULL;
static mutex_t *cond_mutex = NULL;
volatile static bool is_working = true;

ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED(nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED(block);
  P_UNUSED(nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  P_UNUSED(block);
}

static void *
producer_test_thread(void *a) {
  P_UNUSED(a);
  while (is_working == true) {
    if (!p_mutex_lock(cond_mutex)) {
      is_working = false;
      p_condvar_broadcast(queue_full_cond);
      p_uthread_exit(1);
    }
    while (thread_queue >= PCONDTEST_MAX_QUEUE && is_working == true) {
      if (!p_condvar_wait(queue_empty_cond, cond_mutex)) {
        is_working = false;
        p_condvar_broadcast(queue_full_cond);
        p_mutex_unlock(cond_mutex);
        p_uthread_exit(1);
      }
    }
    if (is_working) {
      ++thread_queue;
      ++thread_wakeups;
    }
    if (!p_condvar_broadcast(queue_full_cond)) {
      is_working = false;
      p_mutex_unlock(cond_mutex);
      p_uthread_exit(1);
    }
    if (!p_mutex_unlock(cond_mutex)) {
      is_working = false;
      p_condvar_broadcast(queue_full_cond);
      p_uthread_exit(1);
    }
  }
  p_condvar_broadcast(queue_full_cond);
  p_uthread_exit(0);
  return NULL;
}

static void *
consumer_test_thread(void *a) {
  P_UNUSED(a);
  while (is_working == true) {
    if (!p_mutex_lock(cond_mutex)) {
      is_working = false;
      p_condvar_signal(queue_empty_cond);
      p_uthread_exit(1);
    }
    while (thread_queue <= 0 && is_working == true) {
      if (!p_condvar_wait(queue_full_cond, cond_mutex)) {
        is_working = false;
        p_condvar_signal(queue_empty_cond);
        p_mutex_unlock(cond_mutex);
        p_uthread_exit(1);
      }
    }
    if (is_working) {
      --thread_queue;
      ++thread_wakeups;
    }
    if (!p_condvar_signal(queue_empty_cond)) {
      is_working = false;
      p_mutex_unlock(cond_mutex);
      p_uthread_exit(1);
    }
    if (!p_mutex_unlock(cond_mutex)) {
      is_working = false;
      p_condvar_signal(queue_empty_cond);
      p_uthread_exit(1);
    }
  }
  p_condvar_signal(queue_empty_cond);
  p_uthread_exit(0);
  return NULL;
}

CUTEST(condvar, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(p_mem_set_vtable(&vtable) == true);
  ASSERT(p_condvar_new() == NULL);
  p_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(condvar, bad_input) {
  ASSERT(p_condvar_broadcast(NULL) == false);
  ASSERT(p_condvar_signal(NULL) == false);
  ASSERT(p_condvar_wait(NULL, NULL) == false);
  p_condvar_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(condvar, general) {
  uthread_t *thr1, *thr2, *thr3;

  queue_empty_cond = p_condvar_new();
  ASSERT(queue_empty_cond != NULL);
  queue_full_cond = p_condvar_new();
  ASSERT(queue_full_cond != NULL);
  cond_mutex = p_mutex_new();
  ASSERT(cond_mutex != NULL);
  is_working = true;
  thread_wakeups = 0;
  thread_queue = 0;
  thr1 = p_uthread_create((uthread_fn_t) producer_test_thread, NULL, true);
  ASSERT(thr1 != NULL);
  thr2 = p_uthread_create((uthread_fn_t) consumer_test_thread, NULL, true);
  ASSERT(thr2 != NULL);
  thr3 = p_uthread_create((uthread_fn_t) consumer_test_thread, NULL, true);
  ASSERT(thr3 != NULL);
  ASSERT(p_condvar_broadcast(queue_empty_cond) == true);
  ASSERT(p_condvar_broadcast(queue_full_cond) == true);
  p_uthread_sleep(40);
  is_working = false;
  ASSERT(p_uthread_join(thr1) == 0);
  ASSERT(p_uthread_join(thr2) == 0);
  ASSERT(p_uthread_join(thr3) == 0);
  ASSERT(thread_wakeups > 0 && thread_queue >= 0 && thread_queue <= 10);
  p_uthread_unref(thr1);
  p_uthread_unref(thr2);
  p_uthread_unref(thr3);
  p_condvar_free(queue_empty_cond);
  p_condvar_free(queue_full_cond);
  p_mutex_free(cond_mutex);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(condvar, nomem);
  CUTEST_PASS(condvar, bad_input);
  CUTEST_PASS(condvar, general);
  return EXIT_SUCCESS;
}
