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

CUTEST(mem, bad_input) {
  mem_vtable_t vtable;

  vtable.free = NULL;
  vtable.malloc = NULL;
  vtable.realloc = NULL;
  ASSERT(u_malloc(0) == NULL);
  ASSERT(u_malloc0(0) == NULL);
  ASSERT(u_realloc(NULL, 0) == NULL);
  ASSERT(u_mem_set_vtable(NULL) == false);
  ASSERT(u_mem_set_vtable(&vtable) == false);
  u_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(mem, general) {
  mem_vtable_t vtable;
  ptr_t ptr = NULL;
  int i;

  alloc_counter = 0;
  realloc_counter = 0;
  free_counter = 0;
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);

  /* Test memory allocation using system functions */
  ptr = u_malloc(1024);
  ASSERT(ptr != NULL);
  for (i = 0; i < 1024; ++i) {
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);
  }
  for (i = 0; i < 1024; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == (byte_t) (i % 127));
  u_free(ptr);
  ptr = u_malloc0(2048);
  ASSERT(ptr != NULL);
  for (i = 0; i < 2048; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == 0);
  for (i = 0; i < 2048; ++i) {
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);
  }
  for (i = 0; i < 2048; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == (byte_t) (i % 127));
  u_free(ptr);
  ptr = u_realloc(NULL, 1024);
  ASSERT(ptr != NULL);
  for (i = 0; i < 1024; ++i) {
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);
  }
  ptr = u_realloc(ptr, 2048);
  for (i = 1024; i < 2048; ++i) {
    *(((byte_t *) ptr) + i) = (byte_t) ((i - 1) % 127);
  }
  for (i = 0; i < 1024; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == (byte_t) (i % 127));
  for (i = 1024; i < 2048; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == (byte_t) ((i - 1) % 127));
  u_free(ptr);
  ASSERT(alloc_counter > 0);
  ASSERT(realloc_counter > 0);
  ASSERT(free_counter > 0);
  u_mem_restore_vtable();

  /* Test memory mapping */
  ptr = u_mem_mmap(0, NULL);
  ASSERT(ptr == NULL);
  ptr = u_mem_mmap(1024, NULL);
  ASSERT(ptr != NULL);
  for (i = 0; i < 1024; ++i) {
    *(((byte_t *) ptr) + i) = (byte_t) (i % 127);
  }
  for (i = 0; i < 1024; ++i)
    ASSERT(*(((byte_t *) ptr) + i) == i % 127);
  ASSERT(u_mem_munmap(NULL, 1024, NULL) == false);
  ASSERT(u_mem_munmap(ptr, 1024, NULL) == true);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(mem, bad_input);
  CUTEST_PASS(mem, general);
  return EXIT_SUCCESS;
}
