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

#define PSEMAPHORE_MAX_VAL 10

static int sema_test_val = 0;
static int is_thread_exit = 0;

static void
clean_error(err_t **error) {
  if (error == NULL || *error == NULL) {
    return;
  }
  u_err_free(*error);
  *error = NULL;
}

static void *
sema_test_thread(void) {
  sema_t *sem;
  int i;

  sem = u_sema_new("u_sema_test_object", 1, U_SEMA_OPEN, NULL);
  if (sem == NULL) {
    u_uthread_exit(1);
  }
  for (i = 0; i < 1000; ++i) {
    if (!u_sema_acquire(sem, NULL)) {
      if (is_thread_exit > 0) {
        sema_test_val = PSEMAPHORE_MAX_VAL;
        break;
      }
      u_uthread_exit(1);
    }
    if (sema_test_val == PSEMAPHORE_MAX_VAL) {
      --sema_test_val;
    } else {
      u_uthread_sleep(1);
      ++sema_test_val;
    }
    if (!u_sema_release(sem, NULL)) {
      if (is_thread_exit > 0) {
        sema_test_val = PSEMAPHORE_MAX_VAL;
        break;
      }
      u_uthread_exit(1);
    }
  }
  ++is_thread_exit;
  u_sema_free(sem);
  u_uthread_exit(0);
  return NULL;
}

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

CUTEST(sema, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_sema_new("u_sema_test_object", 1, U_SEMA_CREATE, NULL) == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(sema, general) {
  sema_t *sem = NULL;
  err_t *error = NULL;
  int i;

  ASSERT(u_sema_new(NULL, 0, U_SEMA_CREATE, &error) == NULL);
  ASSERT(error != NULL);
  clean_error(&error);
  ASSERT(u_sema_acquire(sem, &error) == false);
  ASSERT(error != NULL);
  clean_error(&error);
  ASSERT(u_sema_release(sem, &error) == false);
  ASSERT(error != NULL);
  clean_error(&error);
  u_sema_take_ownership(sem);
  u_sema_free(NULL);
  sem = u_sema_new("u_sema_test_object", 10, U_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);
  u_sema_take_ownership(sem);
  u_sema_free(sem);
  sem = u_sema_new("u_sema_test_object", 10, U_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);
  for (i = 0; i < 10; ++i)
    ASSERT(u_sema_acquire(sem, NULL));
  for (i = 0; i < 10; ++i)
    ASSERT(u_sema_release(sem, NULL));
  for (i = 0; i < 1000; ++i) {
    ASSERT(u_sema_acquire(sem, NULL));
    ASSERT(u_sema_release(sem, NULL));
  }
  u_sema_free(sem);
  return CUTE_SUCCESS;
}

CUTEST(sema, thread) {
  uthread_t *thr1, *thr2;
  sema_t *sem;

  sem = u_sema_new("u_sema_test_object", 10, U_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);
  u_sema_take_ownership(sem);
  u_sema_free(sem);
  sem = NULL;
  is_thread_exit = 0;
  sema_test_val = PSEMAPHORE_MAX_VAL;
  thr1 = u_uthread_create((uthread_fn_t) sema_test_thread, NULL, true);
  ASSERT(thr1 != NULL);
  thr2 = u_uthread_create((uthread_fn_t) sema_test_thread, NULL, true);
  ASSERT(thr2 != NULL);
  ASSERT(u_uthread_join(thr1) == 0);
  ASSERT(u_uthread_join(thr2) == 0);
  ASSERT(sema_test_val == PSEMAPHORE_MAX_VAL);
  ASSERT(u_sema_acquire(sem, NULL) == false);
  ASSERT(u_sema_release(sem, NULL) == false);
  u_sema_free(sem);
  u_sema_take_ownership(sem);
  sem = u_sema_new("u_sema_test_object", 1, U_SEMA_OPEN, NULL);
  ASSERT(sem != NULL);
  u_sema_take_ownership(sem);
  u_sema_free(sem);
  u_uthread_unref(thr1);
  u_uthread_unref(thr2);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(sema, nomem);
  CUTEST_PASS(sema, general);
  CUTEST_PASS(sema, thread);
  return EXIT_SUCCESS;
}
