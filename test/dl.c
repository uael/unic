/*
 * Copyright (C) 2015-2017 Alexander Saprykin <xelfium@gmail.com>
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
  int ac;
  char **av;

  void (*st_fn)(void);
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN {
#ifdef P_OS_BEOS
  p_libsys_shutdown();
#else
  /* We have already loaded reference to ourself library, it's OK */
  if (self->st_fn) {
    self->st_fn();
  } else {
    p_libsys_shutdown();
  }
#endif
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

CUTEST(dl, nomem) {
  FILE *file;
  mem_vtable_t vtable;

  if (P_UNLIKELY (p_dl_is_ref_counted() == false)) {
    return CUTE_SUCCESS;
  }
  /* Cleanup from previous run */
  p_file_remove("." P_DIR_SEP "p_empty_file.txt", NULL);
  file = fopen("." P_DIR_SEP "p_empty_file.txt", "w");

  ASSERT(file != NULL);
  ASSERT(fclose(file) == 0);

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

#ifdef P_OS_WIN
  SetErrorMode(SEM_FAILCRITICALERRORS);
#endif

  ASSERT(p_dl_new("."P_DIR_SEP"p_empty_file.txt") == NULL);
  ASSERT(p_dl_new(self->av[self->ac - 1]) == NULL);

#ifdef P_OS_WIN
  SetErrorMode(0);
#endif

  p_mem_restore_vtable();
  ASSERT(p_file_remove("."P_DIR_SEP"p_empty_file.txt", NULL) == true);

  return CUTE_SUCCESS;
}

CUTEST(dl, general) {
  dl_t *loader;
  byte_t *err_msg;
  void (*shutdown_func)(void);


  /* We assume that 3rd argument is ourself library path */
  ASSERT(self->ac > 1);

  /* Invalid usage */
  ASSERT(p_dl_new(NULL) == NULL);
  ASSERT(p_dl_new("./unexistent_file.nofile") == NULL);
  ASSERT(p_dl_get_symbol(NULL, NULL) == NULL);
  ASSERT(p_dl_get_symbol(NULL, "unexistent_symbol") == NULL);

  p_dl_free(NULL);

  /* General tests */

  /* At least not on HP-UX it should be true */
#if !defined (P_OS_HPUX)
  ASSERT(p_dl_is_ref_counted() == true);
#else
  p_dl_is_ref_counted();
#endif

  err_msg = p_dl_get_last_error(NULL);
  p_free(err_msg);

  if (P_UNLIKELY (p_dl_is_ref_counted() == false)) {
    return CUTE_SUCCESS;
  }

  loader = p_dl_new(
    self->av[self->ac - 1]
  );
  ASSERT(loader != NULL);

  ASSERT(
    p_dl_get_symbol(loader, "there_is_no_such_a_symbol") == (PFuncAddr) NULL);

  err_msg = p_dl_get_last_error(loader);
  ASSERT(err_msg != NULL);
  p_free(err_msg);

  shutdown_func = (void (*)(void)) p_dl_get_symbol(loader, "p_libsys_shutdown");

  if (shutdown_func == NULL) {
    shutdown_func =
      (void (*)(void)) p_dl_get_symbol(loader, "_p_libsys_shutdown");
  }

  ASSERT(shutdown_func != NULL);

  err_msg = p_dl_get_last_error(loader);
  p_free(err_msg);

  p_dl_free(loader);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {ac, av, NULL};

  CUTEST_PASS(dl, nomem);
  CUTEST_PASS(dl, general);
  return EXIT_SUCCESS;
}
