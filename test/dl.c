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
#include "unic.h"

CUTEST_DATA {
  int ac;
  char **av;

  void (*st_fn)(void);
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN {
#ifdef U_OS_BEOS
  u_libsys_shutdown();
#else
  /* We have already loaded reference to ourself library, it's OK */
  if (self->st_fn) {
    self->st_fn();
  } else {
    u_libsys_shutdown();
  }
#endif
}

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

CUTEST(dl, nomem) {
  FILE *file;
  mem_vtable_t vtable;

  if (U_UNLIKELY (u_dl_is_ref_counted() == false)) {
    return CUTE_SUCCESS;
  }
  /* Cleanup from previous run */
  u_file_remove("." U_DIR_SEP "u_empty_file.txt", NULL);
  file = fopen("." U_DIR_SEP "u_empty_file.txt", "w");
  ASSERT(file != NULL);
  ASSERT(fclose(file) == 0);
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
#ifdef U_OS_WIN
  SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
  ASSERT(u_dl_new("."
    U_DIR_SEP
    "u_empty_file.txt") == NULL);
  ASSERT(u_dl_new(self->av[self->ac - 1]) == NULL);
#ifdef U_OS_WIN
  SetErrorMode(0);
#endif
  u_mem_restore_vtable();
  ASSERT(u_file_remove("."
    U_DIR_SEP
    "u_empty_file.txt", NULL) == true);
  return CUTE_SUCCESS;
}

CUTEST(dl, general) {
  dl_t *loader;
  byte_t *err_msg;

  /* We assume that 3rd argument is ourself library path */
  ASSERT(self->ac > 1);

  /* Invalid usage */
  ASSERT(u_dl_new(NULL) == NULL);
  ASSERT(u_dl_new("./unexistent_file.nofile") == NULL);
  ASSERT(u_dl_get_symbol(NULL, NULL) == NULL);
  ASSERT(u_dl_get_symbol(NULL, "unexistent_symbol") == NULL);
  u_dl_free(NULL);

  /* General tests */

  /* At least not on HP-UX it should be true */
#if !defined (U_OS_HPUX)
  ASSERT(u_dl_is_ref_counted() == true);
#else
  u_dl_is_ref_counted();
#endif
  err_msg = u_dl_get_last_error(NULL);
  u_free(err_msg);
  if (U_UNLIKELY (u_dl_is_ref_counted() == false)) {
    return CUTE_SUCCESS;
  }
  loader = u_dl_new(
    self->av[self->ac - 1]
  );
  ASSERT(loader != NULL);
  ASSERT(u_dl_get_symbol(
    loader,
    "there_is_no_such_a_symbol"
  ) == (fn_addr_t) NULL);
  err_msg = u_dl_get_last_error(loader);
  ASSERT(err_msg != NULL);
  u_free(err_msg);
  self->st_fn = (void (*)(void)) u_dl_get_symbol(loader, "u_libsys_shutdown");
  if (self->st_fn == NULL) {
    self->st_fn =
      (void (*)(void)) u_dl_get_symbol(loader, "_p_libsys_shutdown");
  }
#ifdef U_CC_WATCOM
  ASSERT(self->st_fn == NULL);
#else
  ASSERT(self->st_fn != NULL);
#endif
  err_msg = u_dl_get_last_error(loader);
  u_free(err_msg);
  u_dl_free(loader);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test;

  test.ac = ac;
  test.av = av;
  test.st_fn = NULL;
  CUTEST_PASS(dl, nomem);
  CUTEST_PASS(dl, general);
  return EXIT_SUCCESS;
}
