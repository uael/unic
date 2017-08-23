/*
 * Copyright (C) 2016-2017 Alexander Saprykin <xelfium@gmail.com>
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

#define PERROR_TEST_MESSAGE  "PError test error message"
#define PERROR_TEST_MESSAGE_2  "Another PError test error message"

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

CUTEST(error, nomem) {
  err_t *error;
  mem_vtable_t vtable;

  error = p_err_new_literal(0, 0, NULL);
  ASSERT(error != NULL);
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(p_mem_set_vtable(&vtable) == true);
  ASSERT(p_err_new() == NULL);
  ASSERT(p_err_new_literal(0, 0, NULL) == NULL);
  ASSERT(p_err_copy(error) == NULL);
  p_mem_restore_vtable();
  p_err_free(error);
  return CUTE_SUCCESS;
}

CUTEST(error, invalid) {
  err_t *error;

  ASSERT(p_err_get_message(NULL) == NULL);
  ASSERT(p_err_get_code(NULL) == 0);
  ASSERT(p_err_get_native_code(NULL) == 0);
  ASSERT(p_err_get_domain(NULL) == P_ERR_DOMAIN_NONE);
  ASSERT(p_err_copy(NULL) == NULL);
  error = (err_t *) 0x1;
  p_err_set_code(NULL, 0);
  p_err_set_native_code(NULL, 0);
  p_err_set_message(NULL, NULL);
  p_err_set_error(NULL, 0, 0, NULL);
  p_err_set_err_p(NULL, 0, 0, NULL);
  p_err_set_err_p(&error, 0, 0, NULL);
  ASSERT(error == (err_t *) 0x1);
  p_err_clear(NULL);
  p_err_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(error, general) {
  err_t *error;
  err_t *copy_error;

  /* Empty initialization test */
  error = p_err_new();
  ASSERT(error != NULL);
  ASSERT(p_err_get_code(error) == 0);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(p_err_get_message(error) == NULL);
  copy_error = p_err_copy(error);
  ASSERT(copy_error != NULL);
  ASSERT(p_err_get_code(copy_error) == 0);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(p_err_get_message(copy_error) == NULL);
  p_err_free(copy_error);
  p_err_set_error(error, (int) P_ERR_DOMAIN_IO, -10, PERROR_TEST_MESSAGE);
  ASSERT(p_err_get_code(error) == (int) P_ERR_DOMAIN_IO);
  ASSERT(p_err_get_native_code(error) == -10);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_IO);
  ASSERT(strcmp(p_err_get_message(error), PERROR_TEST_MESSAGE) == 0);

  /* Change internal data */
  p_err_set_code(error, (int) P_ERR_DOMAIN_IPC);
  p_err_set_native_code(error, -20);
  p_err_set_message(error, PERROR_TEST_MESSAGE_2);
  ASSERT(p_err_get_code(error) == (int) P_ERR_DOMAIN_IPC);
  ASSERT(p_err_get_native_code(error) == -20);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_IPC);
  ASSERT(strcmp(p_err_get_message(error), PERROR_TEST_MESSAGE_2) == 0);

  /* Revert data back */
  p_err_set_code(error, 10);
  p_err_set_native_code(error, -10);
  p_err_set_message(error, PERROR_TEST_MESSAGE);
  copy_error = p_err_copy(error);
  ASSERT(copy_error != NULL);
  ASSERT(p_err_get_code(copy_error) == 10);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(p_err_get_native_code(copy_error) == -10);
  ASSERT(strcmp(p_err_get_message(copy_error), PERROR_TEST_MESSAGE) == 0);
  p_err_free(copy_error);
  p_err_set_error(error, 20, -20, PERROR_TEST_MESSAGE_2);
  ASSERT(p_err_get_code(error) == 20);
  ASSERT(p_err_get_native_code(error) == -20);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(strcmp(p_err_get_message(error), PERROR_TEST_MESSAGE_2) == 0);
  p_err_clear(error);
  ASSERT(p_err_get_code(error) == 0);
  ASSERT(p_err_get_native_code(error) == 0);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(p_err_get_message(error) == NULL);
  p_err_free(error);

  /* Literal initialization test */
  error = p_err_new_literal(30, -30, PERROR_TEST_MESSAGE);
  ASSERT(p_err_get_code(error) == 30);
  ASSERT(p_err_get_native_code(error) == -30);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(strcmp(p_err_get_message(error), PERROR_TEST_MESSAGE) == 0);
  copy_error = p_err_copy(error);
  ASSERT(copy_error != NULL);
  ASSERT(p_err_get_code(copy_error) == 30);
  ASSERT(p_err_get_native_code(copy_error) == -30);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(strcmp(p_err_get_message(copy_error), PERROR_TEST_MESSAGE) == 0);
  p_err_free(copy_error);
  p_err_free(error);

  /* Through the double pointer */
  error = NULL;
  p_err_set_err_p(&error, 10, -10, PERROR_TEST_MESSAGE);
  ASSERT(p_err_get_code(error) == 10);
  ASSERT(p_err_get_native_code(error) == -10);
  ASSERT(p_err_get_domain(error) == P_ERR_DOMAIN_NONE);
  ASSERT(strcmp(p_err_get_message(error), PERROR_TEST_MESSAGE) == 0);
  p_err_free(error);

  /* System codes */
  p_err_set_last_system(10);
  ASSERT(p_err_get_last_system() == 10);
  p_err_set_last_system(0);
  ASSERT(p_err_get_last_system() == 0);
#ifndef P_OS_OS2
  p_err_set_last_net(20);
  ASSERT(p_err_get_last_net() == 20);
  p_err_set_last_net(0);
  ASSERT(p_err_get_last_net() == 0);
#endif
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(error, nomem);
  CUTEST_PASS(error, invalid);
  CUTEST_PASS(error, general);
  return EXIT_SUCCESS;
}
