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

CUTEST(profiler, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_profiler_new() == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(profiler, bad_input) {
  ASSERT(u_profiler_elapsed_usecs(NULL) == 0);
  u_profiler_reset(NULL);
  u_profiler_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(profiler, general) {
  profiler_t *profiler;
  u64_t prev_val, val;

  profiler = u_profiler_new();
  ASSERT(profiler != NULL);
  u_uthread_sleep(50);
  prev_val = u_profiler_elapsed_usecs(profiler);
  ASSERT(prev_val > 0);
  u_uthread_sleep(100);
  val = u_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);
  prev_val = val;
  u_uthread_sleep(1000);
  val = u_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);
  u_profiler_reset(profiler);
  u_uthread_sleep(15);
  prev_val = u_profiler_elapsed_usecs(profiler);
  ASSERT(prev_val > 0);
  u_uthread_sleep(178);
  val = u_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);
  u_profiler_free(profiler);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(profiler, nomem);
  CUTEST_PASS(profiler, bad_input);
  CUTEST_PASS(profiler, general);
  return EXIT_SUCCESS;
}
