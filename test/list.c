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

typedef struct test_data {
  int test_array[3];
  int index;
} test_data_t;

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

static void
foreach_test_func(ptr_t data, ptr_t user_data) {
  test_data_t *test_data;

  if (user_data == NULL) {
    return;
  }
  test_data = (test_data_t *) user_data;
  if (test_data->index < 0 || test_data->index > 2) {
    return;
  }
  test_data->test_array[test_data->index] = U_POINTER_TO_INT (data);
  ++test_data->index;
}

CUTEST(list, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_list_append(NULL, PINT_TO_POINTER(10)) == NULL);
  ASSERT(u_list_prepend(NULL, PINT_TO_POINTER(10)) == NULL);
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(list, invalid) {
  ASSERT(u_list_remove(NULL, NULL) == NULL);
  ASSERT(u_list_last(NULL) == NULL);
  ASSERT(u_list_length(NULL) == 0);
  ASSERT(u_list_reverse(NULL) == NULL);
  u_list_free(NULL);
  u_list_foreach(NULL, NULL, NULL);
  return CUTE_SUCCESS;
}

CUTEST(list, general) {
  list_t *list = NULL;
  test_data_t test_data;

  /* Testing append */
  list = u_list_append(list, U_INT_TO_POINTER (32));
  list = u_list_append(list, U_INT_TO_POINTER (64));
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 2);

  /* Testing data access */
  ASSERT(U_POINTER_TO_INT(list->data) == 32);
  ASSERT(U_POINTER_TO_INT(u_list_last(list)->data) == 64);

  /* Testing prepend */
  list = u_list_prepend(list, U_INT_TO_POINTER (128));
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 3);
  ASSERT(U_POINTER_TO_INT(list->data) == 128);
  ASSERT(U_POINTER_TO_INT(u_list_last(list)->data) == 64);

  /* Testing for each loop */
  memset(&test_data, 0, sizeof(test_data));
  ASSERT(test_data.test_array[0] == 0);
  ASSERT(test_data.test_array[1] == 0);
  ASSERT(test_data.test_array[2] == 0);
  ASSERT(test_data.index == 0);
  u_list_foreach(list, (fn_t) foreach_test_func, (ptr_t) &test_data);
  ASSERT(test_data.index == 3);
  ASSERT(test_data.test_array[0] == 128);
  ASSERT(test_data.test_array[1] == 32);
  ASSERT(test_data.test_array[2] == 64);

  /* Testing reverse */

  list = u_list_reverse(list);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 3);
  ASSERT(U_POINTER_TO_INT(list->data) == 64);
  ASSERT(U_POINTER_TO_INT(u_list_last(list)->data) == 128);

  /* Testing for each loop */
  memset(&test_data, 0, sizeof(test_data));
  ASSERT(test_data.test_array[0] == 0);
  ASSERT(test_data.test_array[1] == 0);
  ASSERT(test_data.test_array[2] == 0);
  ASSERT(test_data.index == 0);
  u_list_foreach(list, (fn_t) foreach_test_func, (ptr_t) &test_data);
  ASSERT(test_data.index == 3);
  ASSERT(test_data.test_array[0] == 64);
  ASSERT(test_data.test_array[1] == 32);
  ASSERT(test_data.test_array[2] == 128);

  /* Testing remove */
  list = u_list_remove(list, U_INT_TO_POINTER (32));
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 2);
  list = u_list_remove(list, U_INT_TO_POINTER (128));
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  list = u_list_remove(list, U_INT_TO_POINTER (256));
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  list = u_list_remove(list, U_INT_TO_POINTER (64));
  ASSERT(list == NULL);
  ASSERT(u_list_length(list) == 0);
  u_list_free(list);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(list, nomem);
  CUTEST_PASS(list, invalid);
  CUTEST_PASS(list, general);
  return EXIT_SUCCESS;
}
