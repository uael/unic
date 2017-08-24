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
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

#define PCONDTEST_MAX_QUEUE 10

static int thread_wakeups = 0;
static int thread_queue = 0;
static condvar_t *queue_empty_cond = NULL;
static condvar_t *queue_full_cond = NULL;
static mutex_t *cond_mutex = NULL;
volatile static bool is_working = true;

ptr_t
pmem_alloc(size_t nbytes) {
  U_UNUSED(nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  U_UNUSED(block);
  U_UNUSED(nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  U_UNUSED(block);
}

static void *
producer_test_thread(void *a) {
  U_UNUSED(a);
  while (is_working == true) {
    if (!u_mutex_lock(cond_mutex)) {
      is_working = false;
      u_condvar_broadcast(queue_full_cond);
      u_uthread_exit(1);
    }
    while (thread_queue >= PCONDTEST_MAX_QUEUE && is_working == true) {
      if (!u_condvar_wait(queue_empty_cond, cond_mutex)) {
        is_working = false;
        u_condvar_broadcast(queue_full_cond);
        u_mutex_unlock(cond_mutex);
        u_uthread_exit(1);
      }
    }
    if (is_working) {
      ++thread_queue;
      ++thread_wakeups;
    }
    if (!u_condvar_broadcast(queue_full_cond)) {
      is_working = false;
      u_mutex_unlock(cond_mutex);
      u_uthread_exit(1);
    }
    if (!u_mutex_unlock(cond_mutex)) {
      is_working = false;
      u_condvar_broadcast(queue_full_cond);
      u_uthread_exit(1);
    }
  }
  u_condvar_broadcast(queue_full_cond);
  u_uthread_exit(0);
  return NULL;
}

static void *
consumer_test_thread(void *a) {
  U_UNUSED(a);
  while (is_working == true) {
    if (!u_mutex_lock(cond_mutex)) {
      is_working = false;
      u_condvar_signal(queue_empty_cond);
      u_uthread_exit(1);
    }
    while (thread_queue <= 0 && is_working == true) {
      if (!u_condvar_wait(queue_full_cond, cond_mutex)) {
        is_working = false;
        u_condvar_signal(queue_empty_cond);
        u_mutex_unlock(cond_mutex);
        u_uthread_exit(1);
      }
    }
    if (is_working) {
      --thread_queue;
      ++thread_wakeups;
    }
    if (!u_condvar_signal(queue_empty_cond)) {
      is_working = false;
      u_mutex_unlock(cond_mutex);
      u_uthread_exit(1);
    }
    if (!u_mutex_unlock(cond_mutex)) {
      is_working = false;
      u_condvar_signal(queue_empty_cond);
      u_uthread_exit(1);
    }
  }
  u_condvar_signal(queue_empty_cond);
  u_uthread_exit(0);
  return NULL;
}

CUTEST(condvar, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_condvar_new() == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(condvar, bad_input) {
  ASSERT(u_condvar_broadcast(NULL) == false);
  ASSERT(u_condvar_signal(NULL) == false);
  ASSERT(u_condvar_wait(NULL, NULL) == false);
  u_condvar_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(condvar, general) {
  uthread_t *thr1, *thr2, *thr3;

  queue_empty_cond = u_condvar_new();
  ASSERT(queue_empty_cond != NULL);
  queue_full_cond = u_condvar_new();
  ASSERT(queue_full_cond != NULL);
  cond_mutex = u_mutex_new();
  ASSERT(cond_mutex != NULL);
  is_working = true;
  thread_wakeups = 0;
  thread_queue = 0;
  thr1 = u_uthread_create((uthread_fn_t) producer_test_thread, NULL, true);
  ASSERT(thr1 != NULL);
  thr2 = u_uthread_create((uthread_fn_t) consumer_test_thread, NULL, true);
  ASSERT(thr2 != NULL);
  thr3 = u_uthread_create((uthread_fn_t) consumer_test_thread, NULL, true);
  ASSERT(thr3 != NULL);
  ASSERT(u_condvar_broadcast(queue_empty_cond) == true);
  ASSERT(u_condvar_broadcast(queue_full_cond) == true);
  u_uthread_sleep(40);
  is_working = false;
  ASSERT(u_uthread_join(thr1) == 0);
  ASSERT(u_uthread_join(thr2) == 0);
  ASSERT(u_uthread_join(thr3) == 0);
  ASSERT(thread_wakeups > 0 && thread_queue >= 0 && thread_queue <= 10);
  u_uthread_unref(thr1);
  u_uthread_unref(thr2);
  u_uthread_unref(thr3);
  u_condvar_free(queue_empty_cond);
  u_condvar_free(queue_full_cond);
  u_mutex_free(cond_mutex);
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
