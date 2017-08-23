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

CUTEST(profiler, nomem) {

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(p_profiler_new() == NULL);

  p_mem_restore_vtable();

  return CUTE_SUCCESS;
}

CUTEST(profiler, bad_input) {

  ASSERT(p_profiler_elapsed_usecs(NULL) == 0);
  p_profiler_reset(NULL);
  p_profiler_free(NULL);

  return CUTE_SUCCESS;
}

CUTEST(profiler, general) {
  profiler_t *profiler = NULL;
  u64_t prev_val, val;

  profiler = p_profiler_new();
  ASSERT(profiler != NULL);

  p_uthread_sleep(50);
  prev_val = p_profiler_elapsed_usecs(profiler);
  ASSERT(prev_val > 0);

  p_uthread_sleep(100);
  val = p_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);
  prev_val = val;

  p_uthread_sleep(1000);
  val = p_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);

  p_profiler_reset(profiler);

  p_uthread_sleep(15);
  prev_val = p_profiler_elapsed_usecs(profiler);
  ASSERT(prev_val > 0);

  p_uthread_sleep(178);
  val = p_profiler_elapsed_usecs(profiler);
  ASSERT(val > prev_val);

  p_profiler_free(profiler);

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
