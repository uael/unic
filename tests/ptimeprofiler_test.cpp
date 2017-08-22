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

#define BOOST_TEST_MODULE ptimeprofiler_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

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

BOOST_AUTO_TEST_CASE (ptimeprofiler_nomem_test) {
  p_libsys_init();

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  BOOST_CHECK (p_mem_set_vtable(&vtable) == true);

  BOOST_CHECK (p_profiler_new() == NULL);

  p_mem_restore_vtable();

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptimeprofiler_bad_input_test) {
  p_libsys_init();

  BOOST_CHECK (p_profiler_elapsed_usecs(NULL) == 0);
  p_profiler_reset(NULL);
  p_profiler_free(NULL);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptimeprofiler_general_test) {
  p_profiler_t *profiler = NULL;
  uint64_t prev_val, val;

  p_libsys_init();

  profiler = p_profiler_new();
  BOOST_REQUIRE (profiler != NULL);

  p_uthread_sleep(50);
  prev_val = p_profiler_elapsed_usecs(profiler);
  BOOST_CHECK (prev_val > 0);

  p_uthread_sleep(100);
  val = p_profiler_elapsed_usecs(profiler);
  BOOST_CHECK (val > prev_val);
  prev_val = val;

  p_uthread_sleep(1000);
  val = p_profiler_elapsed_usecs(profiler);
  BOOST_CHECK (val > prev_val);

  p_profiler_reset(profiler);

  p_uthread_sleep(15);
  prev_val = p_profiler_elapsed_usecs(profiler);
  BOOST_CHECK (prev_val > 0);

  p_uthread_sleep(178);
  val = p_profiler_elapsed_usecs(profiler);
  BOOST_CHECK (val > prev_val);

  p_profiler_free(profiler);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
