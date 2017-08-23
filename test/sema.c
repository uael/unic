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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PSEMAPHORE_MAX_VAL 10

static int sema_test_val = 0;

static int is_thread_exit = 0;

static void
clean_error(err_t **error) {
  if (error == NULL || *error == NULL) {
    return;
  }

  p_err_free(*error);
  *error = NULL;
}

static void *
sema_test_thread(void) {
  sema_t *sem;
  int i;

  sem = p_sema_new("p_sema_test_object", 1, P_SEMA_OPEN, NULL);

  if (sem == NULL) {
    p_uthread_exit(1);
  }

  for (i = 0; i < 1000; ++i) {
    if (!p_sema_acquire(sem, NULL)) {
      if (is_thread_exit > 0) {
        sema_test_val = PSEMAPHORE_MAX_VAL;
        break;
      }

      p_uthread_exit(1);
    }

    if (sema_test_val == PSEMAPHORE_MAX_VAL) {
      --sema_test_val;
    } else {
      p_uthread_sleep(1);
      ++sema_test_val;
    }

    if (!p_sema_release(sem, NULL)) {
      if (is_thread_exit > 0) {
        sema_test_val = PSEMAPHORE_MAX_VAL;
        break;
      }

      p_uthread_exit(1);
    }
  }

  ++is_thread_exit;

  p_sema_free(sem);
  p_uthread_exit(0);

  return NULL;
}

ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED (block);
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  P_UNUSED (block);
}

CUTEST(sema, nomem) {

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(
    p_sema_new("p_sema_test_object", 1, P_SEMA_CREATE, NULL) == NULL);

  p_mem_restore_vtable();

  return CUTE_SUCCESS;
}

CUTEST(sema, general) {
  sema_t *sem = NULL;
  err_t *error = NULL;
  int i;

  ASSERT(p_sema_new(NULL, 0, P_SEMA_CREATE, &error) == NULL);
  ASSERT(error != NULL);
  clean_error(&error);

  ASSERT(p_sema_acquire(sem, &error) == false);
  ASSERT(error != NULL);
  clean_error(&error);

  ASSERT(p_sema_release(sem, &error) == false);
  ASSERT(error != NULL);
  clean_error(&error);

  p_sema_take_ownership(sem);
  p_sema_free(NULL);

  sem = p_sema_new("p_sema_test_object", 10, P_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);
  p_sema_take_ownership(sem);
  p_sema_free(sem);

  sem = p_sema_new("p_sema_test_object", 10, P_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);

  for (i = 0; i < 10; ++i)
    ASSERT(p_sema_acquire(sem, NULL));

  for (i = 0; i < 10; ++i)
    ASSERT(p_sema_release(sem, NULL));

  for (i = 0; i < 1000; ++i) {
    ASSERT(p_sema_acquire(sem, NULL));
    ASSERT(p_sema_release(sem, NULL));
  }

  p_sema_free(sem);

  return CUTE_SUCCESS;
}

CUTEST(sema, thread) {
  uthread_t *thr1, *thr2;
  sema_t *sem = NULL;

  sem = p_sema_new("p_sema_test_object", 10, P_SEMA_CREATE, NULL);
  ASSERT(sem != NULL);
  p_sema_take_ownership(sem);
  p_sema_free(sem);

  sem = NULL;
  is_thread_exit = 0;
  sema_test_val = PSEMAPHORE_MAX_VAL;

  thr1 = p_uthread_create((uthread_fn_t) sema_test_thread, NULL, true);
  ASSERT(thr1 != NULL);

  thr2 = p_uthread_create((uthread_fn_t) sema_test_thread, NULL, true);
  ASSERT(thr2 != NULL);

  ASSERT(p_uthread_join(thr1) == 0);
  ASSERT(p_uthread_join(thr2) == 0);

  ASSERT(sema_test_val == PSEMAPHORE_MAX_VAL);

  ASSERT(p_sema_acquire(sem, NULL) == false);
  ASSERT(p_sema_release(sem, NULL) == false);
  p_sema_free(sem);
  p_sema_take_ownership(sem);

  sem = p_sema_new("p_sema_test_object", 1, P_SEMA_OPEN, NULL);
  ASSERT(sem != NULL);
  p_sema_take_ownership(sem);
  p_sema_free(sem);

  p_uthread_unref(thr1);
  p_uthread_unref(thr2);

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
