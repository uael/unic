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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pspinlock_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PSPINLOCK_MAX_VAL 10

static int_t spinlock_test_val = 0;
static spinlock_t *global_spinlock = NULL;

extern "C" ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

extern "C" ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED (block);
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

extern "C" void
pmem_free(ptr_t block) {
  P_UNUSED (block);
}

static void *
spinlock_test_thread(void *) {
  int_t i;

  for (i = 0; i < 1000; ++i) {
    if (!p_spinlock_trylock(global_spinlock)) {
      if (!p_spinlock_lock(global_spinlock))
        p_uthread_exit(1);
    }

    if (spinlock_test_val == PSPINLOCK_MAX_VAL)
      --spinlock_test_val;
    else {
      p_uthread_sleep(1);
      ++spinlock_test_val;
    }

    if (!p_spinlock_unlock(global_spinlock))
      p_uthread_exit(1);
  }

  p_uthread_exit(0);

  return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pspinlock_nomem_test) {
  p_libsys_init();

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  BOOST_CHECK (p_mem_set_vtable(&vtable) == true);
  BOOST_CHECK (p_spinlock_new() == NULL);

  p_mem_restore_vtable();

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pspinlock_bad_input_test) {
  p_libsys_init();

  BOOST_REQUIRE (p_spinlock_lock(NULL) == false);
  BOOST_REQUIRE (p_spinlock_unlock(NULL) == false);
  BOOST_REQUIRE (p_spinlock_trylock(NULL) == false);
  p_spinlock_free(NULL);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pspinlock_general_test) {
  uthread_t *thr1, *thr2;

  p_libsys_init();

  spinlock_test_val = PSPINLOCK_MAX_VAL;
  global_spinlock = p_spinlock_new();

  BOOST_REQUIRE (global_spinlock != NULL);

  thr1 = p_uthread_create((uthread_fn_t) spinlock_test_thread, NULL, true);
  BOOST_REQUIRE (thr1 != NULL);

  thr2 = p_uthread_create((uthread_fn_t) spinlock_test_thread, NULL, true);
  BOOST_REQUIRE (thr2 != NULL);

  BOOST_CHECK (p_uthread_join(thr1) == 0);
  BOOST_CHECK (p_uthread_join(thr2) == 0);

  BOOST_REQUIRE (spinlock_test_val == PSPINLOCK_MAX_VAL);

  p_uthread_unref(thr1);
  p_uthread_unref(thr2);
  p_spinlock_free(global_spinlock);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
