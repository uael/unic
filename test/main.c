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

static int alloc_counter = 0;

static int realloc_counter = 0;

static int free_counter = 0;

ptr_t
pmem_alloc(size_t nbytes) {
  ++alloc_counter;
  return (ptr_t) malloc(nbytes);
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  ++realloc_counter;
  return (ptr_t) realloc(block, nbytes);
}

void
pmem_free(ptr_t block) {
  ++free_counter;
  free(block);
}

CUTEST(main, general) {
  return CUTE_SUCCESS;
}

CUTEST(main, double) {
  p_libsys_init_full(NULL);
  return CUTE_SUCCESS;
  return CUTE_SUCCESS;
}

CUTEST(main, vtable) {
  mem_vtable_t vtable;
  byte_t *buf;
  byte_t *new_buf;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  p_libsys_init_full(&vtable);

  alloc_counter = 0;
  realloc_counter = 0;
  free_counter = 0;
  buf = (byte_t *) p_malloc0(10);
  new_buf = (byte_t *) p_realloc((ptr_t) buf, 20);

  ASSERT(new_buf != NULL);

  buf = new_buf;

  p_free(buf);

  ASSERT(alloc_counter > 0);
  ASSERT(realloc_counter > 0);
  ASSERT(free_counter > 0);

  ASSERT(strcmp(p_libsys_version(), PLIBSYS_VERSION_STR) == 0);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(main, general);
  CUTEST_PASS(main, double);
  CUTEST_PASS(main, vtable);
  return EXIT_SUCCESS;
}
