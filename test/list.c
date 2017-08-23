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

typedef struct _TestData {
  int test_array[3];
  int index;
} TestData;

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

static void
foreach_test_func(ptr_t data, ptr_t user_data) {
  TestData *test_data;

  if (user_data == NULL) {
    return;
  }
  test_data = (TestData *) user_data;
  if (test_data->index < 0 || test_data->index > 2) {
    return;
  }
  test_data->test_array[test_data->index] = P_POINTER_TO_INT (data);
  ++test_data->index;
}

CUTEST(list, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(p_list_append(NULL, PINT_TO_POINTER(10)) == NULL);
  ASSERT(p_list_prepend(NULL, PINT_TO_POINTER(10)) == NULL);

  p_mem_restore_vtable();

  return CUTE_SUCCESS;
}

CUTEST(list, invalid) {

  ASSERT(p_list_remove(NULL, NULL) == NULL);
  ASSERT(p_list_last(NULL) == NULL);
  ASSERT(p_list_length(NULL) == 0);
  ASSERT(p_list_reverse(NULL) == NULL);

  p_list_free(NULL);
  p_list_foreach(NULL, NULL, NULL);

  return CUTE_SUCCESS;
}

CUTEST(list, general) {
  list_t *list = NULL;
  TestData test_data;


  /* Testing append */
  list = p_list_append(list, P_INT_TO_POINTER (32));
  list = p_list_append(list, P_INT_TO_POINTER (64));

  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 2);

  /* Testing data access */
  ASSERT(P_POINTER_TO_INT(list->data) == 32);
  ASSERT(P_POINTER_TO_INT(p_list_last(list)->data) == 64);

  /* Testing prepend */
  list = p_list_prepend(list, P_INT_TO_POINTER (128));
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 3);
  ASSERT(P_POINTER_TO_INT(list->data) == 128);
  ASSERT(P_POINTER_TO_INT(p_list_last(list)->data) == 64);

  /* Testing for each loop */
  memset(&test_data, 0, sizeof(test_data));

  ASSERT(test_data.test_array[0] == 0);
  ASSERT(test_data.test_array[1] == 0);
  ASSERT(test_data.test_array[2] == 0);
  ASSERT(test_data.index == 0);

  p_list_foreach(list, (fn_t) foreach_test_func, (ptr_t) &test_data);

  ASSERT(test_data.index == 3);
  ASSERT(test_data.test_array[0] == 128);
  ASSERT(test_data.test_array[1] == 32);
  ASSERT(test_data.test_array[2] == 64);

  /* Testing reverse */

  list = p_list_reverse(list);

  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 3);
  ASSERT(P_POINTER_TO_INT(list->data) == 64);
  ASSERT(P_POINTER_TO_INT(p_list_last(list)->data) == 128);

  /* Testing for each loop */
  memset(&test_data, 0, sizeof(test_data));

  ASSERT(test_data.test_array[0] == 0);
  ASSERT(test_data.test_array[1] == 0);
  ASSERT(test_data.test_array[2] == 0);
  ASSERT(test_data.index == 0);

  p_list_foreach(list, (fn_t) foreach_test_func, (ptr_t) &test_data);

  ASSERT(test_data.index == 3);
  ASSERT(test_data.test_array[0] == 64);
  ASSERT(test_data.test_array[1] == 32);
  ASSERT(test_data.test_array[2] == 128);

  /* Testing remove */
  list = p_list_remove(list, P_INT_TO_POINTER (32));
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 2);

  list = p_list_remove(list, P_INT_TO_POINTER (128));
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);

  list = p_list_remove(list, P_INT_TO_POINTER (256));
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);

  list = p_list_remove(list, P_INT_TO_POINTER (64));
  ASSERT(list == NULL);
  ASSERT(p_list_length(list) == 0);

  p_list_free(list);

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
