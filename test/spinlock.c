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

#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

#define PSPINLOCK_MAX_VAL 10

static int spinlock_test_val = 0;
static spinlock_t *global_spinlock = NULL;

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
spinlock_test_thread(void) {
  int i;

  for (i = 0; i < 1000; ++i) {
    if (!u_spinlock_trylock(global_spinlock)) {
      if (!u_spinlock_lock(global_spinlock)) {
        u_thread_exit(1);
      }
    }
    if (spinlock_test_val == PSPINLOCK_MAX_VAL) {
      --spinlock_test_val;
    } else {
      u_thread_sleep(1);
      ++spinlock_test_val;
    }
    if (!u_spinlock_unlock(global_spinlock)) {
      u_thread_exit(1);
    }
  }
  u_thread_exit(0);
  return NULL;
}

CUTEST(spinlock, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_spinlock_new() == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(spinlock, bad_input) {
  ASSERT(u_spinlock_lock(NULL) == false);
  ASSERT(u_spinlock_unlock(NULL) == false);
  ASSERT(u_spinlock_trylock(NULL) == false);
  u_spinlock_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(spinlock, general) {
  thread_t *thr1, *thr2;

  spinlock_test_val = PSPINLOCK_MAX_VAL;
  global_spinlock = u_spinlock_new();
  ASSERT(global_spinlock != NULL);
  thr1 = u_thread_create((thread_fn_t) spinlock_test_thread, NULL, true);
  ASSERT(thr1 != NULL);
  thr2 = u_thread_create((thread_fn_t) spinlock_test_thread, NULL, true);
  ASSERT(thr2 != NULL);
  ASSERT(u_thread_join(thr1) == 0);
  ASSERT(u_thread_join(thr2) == 0);
  ASSERT(spinlock_test_val == PSPINLOCK_MAX_VAL);
  u_thread_unref(thr1);
  u_thread_unref(thr2);
  u_spinlock_free(global_spinlock);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(spinlock, nomem);
  CUTEST_PASS(spinlock, bad_input);
  CUTEST_PASS(spinlock, general);
  return EXIT_SUCCESS;
}
