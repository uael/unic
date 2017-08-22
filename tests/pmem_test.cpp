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

#define BOOST_TEST_MODULE pmem_test

#include "plib.h"

#include <stdlib.h>
#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

static int_t alloc_counter = 0;
static int_t realloc_counter = 0;
static int_t free_counter = 0;

extern "C" ptr_t
pmem_alloc(size_t nbytes) {
  ++alloc_counter;
  return (ptr_t) malloc(nbytes);
}

extern "C" ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  ++realloc_counter;
  return (ptr_t) realloc(block, nbytes);
}

extern "C" void
pmem_free(ptr_t block) {
  ++free_counter;
  free(block);
}

BOOST_AUTO_TEST_CASE (pmem_bad_input_test) {
  mem_vtable_t vtable;

  p_libsys_init();

  vtable.free = NULL;
  vtable.malloc = NULL;
  vtable.realloc = NULL;

  BOOST_CHECK (p_malloc(0) == NULL);
  BOOST_CHECK (p_malloc0(0) == NULL);
  BOOST_CHECK (p_realloc(NULL, 0) == NULL);
  BOOST_CHECK (p_mem_set_vtable(NULL) == false);
  BOOST_CHECK (p_mem_set_vtable(&vtable) == false);
  p_free(NULL);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (pmem_general_test) {
  mem_vtable_t vtable;
  ptr_t ptr = NULL;
  int_t i;

  p_libsys_init();

  alloc_counter = 0;
  realloc_counter = 0;
  free_counter = 0;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  BOOST_CHECK (p_mem_set_vtable(&vtable) == true);

  /* Test memory allocation using system functions */
  ptr = p_malloc(1024);
  BOOST_REQUIRE (ptr != NULL);

  for (int i = 0; i < 1024; ++i)
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);

  for (int i = 0; i < 1024; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == (byte_t) (i % 127));

  p_free(ptr);

  ptr = p_malloc0(2048);
  BOOST_REQUIRE (ptr != NULL);

  for (int i = 0; i < 2048; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == 0);

  for (int i = 0; i < 2048; ++i)
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);

  for (int i = 0; i < 2048; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == (byte_t) (i % 127));

  p_free(ptr);

  ptr = p_realloc(NULL, 1024);
  BOOST_REQUIRE (ptr != NULL);

  for (int i = 0; i < 1024; ++i)
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);

  ptr = p_realloc(ptr, 2048);

  for (int i = 1024; i < 2048; ++i)
    *(((byte_t *) ptr) + i) = (byte_t) ((i - 1) % 127);

  for (int i = 0; i < 1024; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == (byte_t) (i % 127));

  for (int i = 1024; i < 2048; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == (byte_t) ((i - 1) % 127));

  p_free(ptr);

  BOOST_CHECK (alloc_counter > 0);
  BOOST_CHECK (realloc_counter > 0);
  BOOST_CHECK (free_counter > 0);

  p_mem_restore_vtable();

  /* Test memory mapping */
  ptr = p_mem_mmap(0, NULL);
  BOOST_CHECK (ptr == NULL);

  ptr = p_mem_mmap(1024, NULL);
  BOOST_REQUIRE (ptr != NULL);

  for (i = 0; i < 1024; ++i)
    *(((byte_t *) ptr) + i) = i % 127;

  for (i = 0; i < 1024; ++i)
    BOOST_CHECK (*(((byte_t *) ptr) + i) == i % 127);

  BOOST_CHECK (p_mem_munmap(NULL, 1024, NULL) == false);
  BOOST_CHECK (p_mem_munmap(ptr, 1024, NULL) == true);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
