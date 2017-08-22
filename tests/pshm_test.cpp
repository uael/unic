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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pshm_test

#include "plib.h"

#include <stdlib.h>
#include <time.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static void *
shm_test_thread(void *arg) {
  int_t rand_num;
  size_t shm_size;
  ptr_t addr;
  shm_t *shm;

  if (arg == NULL)
    p_uthread_exit(1);

  shm = (shm_t *) arg;
  rand_num = rand() % 127;
  shm_size = p_shm_get_size(shm);
  addr = p_shm_get_address(shm);

  if (shm_size == 0 || addr == NULL)
    p_uthread_exit(1);

  if (!p_shm_lock(shm, NULL))
    p_uthread_exit(1);

  for (uint_t i = 0; i < shm_size; ++i)
    *(((byte_t *) addr) + i) = (byte_t) rand_num;

  if (!p_shm_unlock(shm, NULL))
    p_uthread_exit(1);

  p_uthread_exit(0);

  return NULL;
}

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

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pshm_nomem_test) {
  p_libsys_init();

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  BOOST_CHECK (p_mem_set_vtable(&vtable) == true);

  BOOST_CHECK (
    p_shm_new("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL)
      == NULL);

  p_mem_restore_vtable();

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pshm_invalid_test) {
  p_libsys_init();

  BOOST_CHECK (p_shm_new(NULL, 0, P_SHM_ACCESS_READWRITE, NULL) == NULL);
  BOOST_CHECK (p_shm_lock(NULL, NULL) == false);
  BOOST_CHECK (p_shm_unlock(NULL, NULL) == false);
  BOOST_CHECK (p_shm_get_address(NULL) == NULL);
  BOOST_CHECK (p_shm_get_size(NULL) == 0);
  p_shm_take_ownership(NULL);

  shm_t *shm = p_shm_new("p_shm_invalid_test", 0, P_SHM_ACCESS_READWRITE, NULL);
  p_shm_take_ownership(shm);
  p_shm_free(shm);

  shm = p_shm_new("p_shm_invalid_test", 10, (shm_access_t) -1, NULL);
  p_shm_take_ownership(shm);
  p_shm_free(shm);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pshm_general_test) {
  shm_t *shm = NULL;
#ifndef P_OS_HPUX
  shm_t *shm2 = NULL;
#endif
  ptr_t addr, addr2;
  int_t i;

  p_libsys_init();

  shm =
    p_shm_new("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
  BOOST_REQUIRE (shm != NULL);
  p_shm_take_ownership(shm);
  p_shm_free(shm);

  shm =
    p_shm_new("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
  BOOST_REQUIRE (shm != NULL);
  BOOST_REQUIRE (p_shm_get_size(shm) == 1024);

  addr = p_shm_get_address(shm);
  BOOST_REQUIRE (addr != NULL);

#ifndef P_OS_HPUX
  shm2 =
    p_shm_new("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READONLY, NULL);

  if (shm2 == NULL) {
    /* OK, some systems may want exactly the same permissions */
    shm2 =
      p_shm_new("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
  }

  BOOST_REQUIRE (shm2 != NULL);
  BOOST_REQUIRE (p_shm_get_size(shm2) == 1024);

  addr2 = p_shm_get_address(shm2);
  BOOST_REQUIRE (shm2 != NULL);
#endif

  for (i = 0; i < 512; ++i) {
    BOOST_CHECK (p_shm_lock(shm, NULL));
    *(((byte_t *) addr) + i) = 'a';
    BOOST_CHECK (p_shm_unlock(shm, NULL));
  }

#ifndef P_OS_HPUX
  for (i = 0; i < 512; ++i) {
    BOOST_CHECK (p_shm_lock(shm2, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) == 'a');
    BOOST_CHECK (p_shm_unlock(shm2, NULL));
  }
#else
  for (i = 0; i < 512; ++i) {
    BOOST_CHECK (p_shm_lock (shm, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) == 'a');
    BOOST_CHECK (p_shm_unlock (shm, NULL));
  }
#endif

  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock(shm, NULL));
    *(((byte_t *) addr) + i) = 'b';
    BOOST_CHECK (p_shm_unlock(shm, NULL));
  }

#ifndef P_OS_HPUX
  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock(shm2, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) != 'c');
    BOOST_CHECK (p_shm_unlock(shm2, NULL));
  }

  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock(shm2, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) == 'b');
    BOOST_CHECK (p_shm_unlock(shm2, NULL));
  }
#else
  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock (shm, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) != 'c');
    BOOST_CHECK (p_shm_unlock (shm, NULL));
  }

  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock (shm, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) == 'b');
    BOOST_CHECK (p_shm_unlock (shm, NULL));
  }
#endif

  p_shm_free(shm);

  shm =
    p_shm_new("p_shm_test_memory_block_2", 1024, P_SHM_ACCESS_READWRITE, NULL);
  BOOST_REQUIRE (shm != NULL);
  BOOST_REQUIRE (p_shm_get_size(shm) == 1024);

  addr = p_shm_get_address(shm);
  BOOST_REQUIRE (addr != NULL);

  for (i = 0; i < 1024; ++i) {
    BOOST_CHECK (p_shm_lock(shm, NULL));
    BOOST_CHECK (*(((byte_t *) addr) + i) != 'b');
    BOOST_CHECK (p_shm_unlock(shm, NULL));
  }

  p_shm_free(shm);

#ifndef P_OS_HPUX
  p_shm_free(shm2);
#endif

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pshm_thread_test) {
  shm_t *shm;
  uthread_t *thr1, *thr2, *thr3;
  ptr_t addr;
  int_t i, val;
  bool test_ok;

  p_libsys_init();

  srand((uint_t) time(NULL));

  shm =
    p_shm_new("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE,
      NULL);
  BOOST_REQUIRE (shm != NULL);
  p_shm_take_ownership(shm);
  p_shm_free(shm);

  shm =
    p_shm_new("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE,
      NULL);
  BOOST_REQUIRE (shm != NULL);

  if (p_shm_get_size(shm) != 1024 * 1024) {
    p_shm_free(shm);
    shm =
      p_shm_new("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE,
        NULL);
    BOOST_REQUIRE (shm != NULL);
  }

  BOOST_REQUIRE (p_shm_get_size(shm) == 1024 * 1024);

  addr = p_shm_get_address(shm);
  BOOST_REQUIRE (addr != NULL);

  thr1 = p_uthread_create((uthread_fn_t) shm_test_thread, (ptr_t) shm, true);
  BOOST_REQUIRE (thr1 != NULL);

  thr2 = p_uthread_create((uthread_fn_t) shm_test_thread, (ptr_t) shm, true);
  BOOST_REQUIRE (thr2 != NULL);

  thr3 = p_uthread_create((uthread_fn_t) shm_test_thread, (ptr_t) shm, true);
  BOOST_REQUIRE (thr3 != NULL);

  BOOST_CHECK (p_uthread_join(thr1) == 0);
  BOOST_CHECK (p_uthread_join(thr2) == 0);
  BOOST_CHECK (p_uthread_join(thr3) == 0);

  test_ok = true;
  val = *((byte_t *) addr);

  for (i = 1; i < 1024 * 1024; ++i)
    if (*(((byte_t *) addr) + i) != val) {
      test_ok = false;
      break;
    }

  BOOST_REQUIRE (test_ok == true);

  p_uthread_unref(thr1);
  p_uthread_unref(thr2);
  p_uthread_unref(thr3);
  p_shm_free(shm);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
