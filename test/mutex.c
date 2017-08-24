/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

static int mutex_test_val = 0;

static mutex_t *global_mutex = NULL;

ptr_t
pmem_alloc(size_t nbytes) {
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  U_UNUSED (block);
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  U_UNUSED (block);
}

static void *
mutex_test_thread(void) {
  int i;

  for (i = 0; i < 100; ++i) {
    if (!u_mutex_trylock(global_mutex)) {
      if (!u_mutex_lock(global_mutex)) {
        u_uthread_exit(1);
      }
    }
    if (mutex_test_val == 10) {
      --mutex_test_val;
    } else {
      u_uthread_sleep(1);
      ++mutex_test_val;
    }
    if (!u_mutex_unlock(global_mutex)) {
      u_uthread_exit(1);
    }
  }
  u_uthread_exit(0);
  return NULL;
}

CUTEST(mutex, nomem) {

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_mutex_new() == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(mutex, bad_input) {

  ASSERT(u_mutex_lock(NULL) == false);
  ASSERT(u_mutex_unlock(NULL) == false);
  ASSERT(u_mutex_trylock(NULL) == false);
  u_mutex_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(mutex, general) {
  uthread_t *thr1, *thr2;

  global_mutex = u_mutex_new();
  ASSERT(global_mutex != NULL);
  mutex_test_val = 10;
  thr1 = u_uthread_create((uthread_fn_t) mutex_test_thread, NULL, true);
  ASSERT(thr1 != NULL);
  thr2 = u_uthread_create((uthread_fn_t) mutex_test_thread, NULL, true);
  ASSERT(thr2 != NULL);
  ASSERT(u_uthread_join(thr1) == 0);
  ASSERT(u_uthread_join(thr2) == 0);
  ASSERT(mutex_test_val == 10);
  u_uthread_unref(thr1);
  u_uthread_unref(thr2);
  u_mutex_free(global_mutex);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(mutex, nomem);
  CUTEST_PASS(mutex, bad_input);
  CUTEST_PASS(mutex, general);
  return EXIT_SUCCESS;
}
