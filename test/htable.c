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

#include <time.h>
#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

#define PHASHTABLE_STRESS_COUNT  10000

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

static int
test_htable_values(const_ptr_t a, const_ptr_t b) {
  return a > b ? 0 : (a < b ? -1 : 1);
}

CUTEST(htable, nomem) {
  htable_t *table;
  mem_vtable_t vtable;

  table = u_htable_new();
  ASSERT(table != NULL);
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_htable_new() == NULL);
  u_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  ASSERT(u_htable_keys(table) == NULL);
  ASSERT(u_htable_values(table) == NULL);
  u_mem_restore_vtable();
  u_htable_free(table);
  return CUTE_SUCCESS;
}

CUTEST(htable, invalid) {
  ASSERT(u_htable_keys(NULL) == NULL);
  ASSERT(u_htable_values(NULL) == NULL);
  ASSERT(u_htable_lookup(NULL, NULL) == NULL);
  ASSERT(u_htable_lookup_by_value(NULL, NULL, NULL) == NULL);
  u_htable_insert(NULL, NULL, NULL);
  u_htable_remove(NULL, NULL);
  u_htable_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(htable, general) {
  htable_t *table;
  list_t *list;

  table = u_htable_new();
  ASSERT(table != NULL);

  /* Test for NULL key */
  u_htable_insert(table, NULL, PINT_TO_POINTER (1));
  list = u_htable_keys(table);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 0);
  u_list_free(list);
  list = u_htable_values(table);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  u_list_free(list);
  u_htable_remove(table, NULL);

  /* Test for insertion */
  u_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  list = u_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 10);
  u_list_free(list);
  list = u_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  u_list_free(list);

  /* False remove */
  u_htable_remove(table, PINT_TO_POINTER (2));
  list = u_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 10);
  u_list_free(list);
  list = u_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  u_list_free(list);

  /* Replace existing value */
  u_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (15));
  list = u_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 15);
  u_list_free(list);
  list = u_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  u_list_free(list);

  /* More insertion */
  u_htable_insert(table, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
  u_htable_insert(table, PINT_TO_POINTER (3), PINT_TO_POINTER (30));
  list = u_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 65);
  u_list_free(list);
  list = u_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 6);
  u_list_free(list);
  ASSERT(PPOINTER_TO_INT(u_htable_lookup(table, PINT_TO_POINTER(1))) == 15);
  ASSERT(PPOINTER_TO_INT(u_htable_lookup(table, PINT_TO_POINTER(2))) == 20);
  ASSERT(PPOINTER_TO_INT(u_htable_lookup(table, PINT_TO_POINTER(3))) == 30);
  ASSERT(u_htable_lookup(table, PINT_TO_POINTER(4)) == (ptr_t) -1);
  u_htable_insert(table, PINT_TO_POINTER (22), PINT_TO_POINTER (20));
  list = u_htable_lookup_by_value(
    table,
    PINT_TO_POINTER (19),
    (cmp_fn_t) test_htable_values
  );
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 27);
  u_list_free(list);
  list = u_htable_lookup_by_value(table, PINT_TO_POINTER (20), NULL);
  ASSERT(list != NULL);
  ASSERT(u_list_length(list) == 2);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) == 24);
  u_list_free(list);
  ASSERT(PPOINTER_TO_INT(u_htable_lookup(table, PINT_TO_POINTER(22))) == 20);
  u_htable_remove(table, PINT_TO_POINTER (1));
  u_htable_remove(table, PINT_TO_POINTER (2));
  list = u_htable_keys(table);
  ASSERT(u_list_length(list) == 2);
  u_list_free(list);
  list = u_htable_values(table);
  ASSERT(u_list_length(list) == 2);
  u_list_free(list);
  u_htable_remove(table, PINT_TO_POINTER (3));
  list = u_htable_keys(table);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 22);
  u_list_free(list);
  list = u_htable_values(table);
  ASSERT(u_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 20);
  u_list_free(list);
  u_htable_remove(table, PINT_TO_POINTER (22));
  ASSERT(u_htable_keys(table) == NULL);
  ASSERT(u_htable_values(table) == NULL);
  u_htable_free(table);
  return CUTE_SUCCESS;
}

CUTEST(htable, stress) {
  htable_t *table;
  int counter;
  int *keys;
  int *values;
  int rand_number;
  int i;

  table = u_htable_new();
  ASSERT(table != NULL);
  srand((unsigned int) time(NULL));
  counter = 0;
  keys = (int *) u_malloc0(PHASHTABLE_STRESS_COUNT * sizeof(int));
  values = (int *) u_malloc0(PHASHTABLE_STRESS_COUNT * sizeof(int));
  ASSERT(keys != NULL);
  ASSERT(values != NULL);
  while (counter != PHASHTABLE_STRESS_COUNT) {
    rand_number = rand();
    if (u_htable_lookup(table, PINT_TO_POINTER (rand_number))
      != (ptr_t) (-1)) {
      continue;
    }
    keys[counter] = rand_number;
    values[counter] = rand() + 1;
    u_htable_remove(table, PINT_TO_POINTER (keys[counter]));
    u_htable_insert(
      table, PINT_TO_POINTER (keys[counter]),
      PINT_TO_POINTER (values[counter]));
    ++counter;
  }
  for (i = 0; i < PHASHTABLE_STRESS_COUNT; ++i) {
    ASSERT(u_htable_lookup(table, PINT_TO_POINTER(keys[i])) ==
      PINT_TO_POINTER(values[i]));
    u_htable_remove(table, PINT_TO_POINTER (keys[i]));
    ASSERT(
      u_htable_lookup(table, PINT_TO_POINTER(keys[i])) == (ptr_t) (-1));
  }
  ASSERT(u_htable_keys(table) == NULL);
  ASSERT(u_htable_values(table) == NULL);
  u_free(keys);
  u_free(values);
  u_htable_free(table);

  /* Try to free at once */
  table = u_htable_new();
  ASSERT(table != NULL);
  counter = 0;
  while (counter != PHASHTABLE_STRESS_COUNT) {
    rand_number = rand();
    if (u_htable_lookup(table, PINT_TO_POINTER (rand_number)) != (ptr_t) (-1)) {
      continue;
    }
    u_htable_insert(
      table, PINT_TO_POINTER (rand_number),
      PINT_TO_POINTER (rand() + 1));
    ++counter;
  }
  u_htable_free(table);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(htable, nomem);
  CUTEST_PASS(htable, invalid);
  CUTEST_PASS(htable, general);
  CUTEST_PASS(htable, stress);
  return EXIT_SUCCESS;
}
