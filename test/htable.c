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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PHASHTABLE_STRESS_COUNT  10000

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

static int
test_htable_values(const_ptr_t a, const_ptr_t b) {
  return a > b ? 0 : (a < b ? -1 : 1);
}

CUTEST(htable, nomem) {

  htable_t *table = p_htable_new();
  ASSERT(table != NULL);

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(p_htable_new() == NULL);
  p_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  ASSERT(p_htable_keys(table) == NULL);
  ASSERT(p_htable_values(table) == NULL);

  p_mem_restore_vtable();

  p_htable_free(table);

  return CUTE_SUCCESS;
}

CUTEST(htable, invalid) {

  ASSERT(p_htable_keys(NULL) == NULL);
  ASSERT(p_htable_values(NULL) == NULL);
  ASSERT(p_htable_lookup(NULL, NULL) == NULL);
  ASSERT(p_htable_lookup_by_value(NULL, NULL, NULL) == NULL);
  p_htable_insert(NULL, NULL, NULL);
  p_htable_remove(NULL, NULL);
  p_htable_free(NULL);

  return CUTE_SUCCESS;
}

CUTEST(htable, general) {
  htable_t *table = NULL;
  list_t *list = NULL;

  table = p_htable_new();
  ASSERT(table != NULL);

  /* Test for NULL key */
  p_htable_insert(table, NULL, PINT_TO_POINTER (1));
  list = p_htable_keys(table);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 0);
  p_list_free(list);
  list = p_htable_values(table);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  p_list_free(list);
  p_htable_remove(table, NULL);

  /* Test for insertion */
  p_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  list = p_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 10);
  p_list_free(list);
  list = p_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  p_list_free(list);

  /* False remove */
  p_htable_remove(table, PINT_TO_POINTER (2));
  list = p_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 10);
  p_list_free(list);
  list = p_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  p_list_free(list);

  /* Replace existing value */
  p_htable_insert(table, PINT_TO_POINTER (1), PINT_TO_POINTER (15));
  list = p_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 15);
  p_list_free(list);
  list = p_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 1);
  p_list_free(list);

  /* More insertion */
  p_htable_insert(table, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
  p_htable_insert(table, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

  list = p_htable_values(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 65);
  p_list_free(list);
  list = p_htable_keys(table);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 6);
  p_list_free(list);

  ASSERT(
    PPOINTER_TO_INT(p_htable_lookup(table, PINT_TO_POINTER(1))) == 15);
  ASSERT(
    PPOINTER_TO_INT(p_htable_lookup(table, PINT_TO_POINTER(2))) == 20);
  ASSERT(
    PPOINTER_TO_INT(p_htable_lookup(table, PINT_TO_POINTER(3))) == 30);
  ASSERT(p_htable_lookup(table, PINT_TO_POINTER(4)) == (ptr_t) -1);
  p_htable_insert(table, PINT_TO_POINTER (22), PINT_TO_POINTER (20));

  list = p_htable_lookup_by_value(
    table,
    PINT_TO_POINTER (19),
    (cmp_fn_t) test_htable_values
  );
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 3);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) +
    PPOINTER_TO_INT(list->next->next->data) == 27);
  p_list_free(list);

  list = p_htable_lookup_by_value(
    table,
    PINT_TO_POINTER (20),
    NULL);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 2);
  ASSERT(PPOINTER_TO_INT(list->data) +
    PPOINTER_TO_INT(list->next->data) == 24);
  p_list_free(list);

  ASSERT(
    PPOINTER_TO_INT(p_htable_lookup(table, PINT_TO_POINTER(22))) == 20);

  p_htable_remove(table, PINT_TO_POINTER (1));
  p_htable_remove(table, PINT_TO_POINTER (2));

  list = p_htable_keys(table);
  ASSERT(p_list_length(list) == 2);
  p_list_free(list);
  list = p_htable_values(table);
  ASSERT(p_list_length(list) == 2);
  p_list_free(list);

  p_htable_remove(table, PINT_TO_POINTER (3));

  list = p_htable_keys(table);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 22);
  p_list_free(list);
  list = p_htable_values(table);
  ASSERT(p_list_length(list) == 1);
  ASSERT(PPOINTER_TO_INT(list->data) == 20);
  p_list_free(list);

  p_htable_remove(table, PINT_TO_POINTER (22));

  ASSERT(p_htable_keys(table) == NULL);
  ASSERT(p_htable_values(table) == NULL);

  p_htable_free(table);

  return CUTE_SUCCESS;
}

CUTEST(htable, stress) {

  htable_t *table = p_htable_new();
  ASSERT(table != NULL);

  srand((unsigned int) time(NULL));

  int counter = 0;

  int *keys = (int *) p_malloc0(PHASHTABLE_STRESS_COUNT * sizeof(int));
  int *values = (int *) p_malloc0(PHASHTABLE_STRESS_COUNT * sizeof(int));

  ASSERT(keys != NULL);
  ASSERT(values != NULL);

  while (counter != PHASHTABLE_STRESS_COUNT) {
    int rand_number = rand();

    if (p_htable_lookup(table, PINT_TO_POINTER (rand_number))
      != (ptr_t) (-1)) {
        continue;
    }

    keys[counter] = rand_number;
    values[counter] = rand() + 1;

    p_htable_remove(table, PINT_TO_POINTER (keys[counter]));
    p_htable_insert(
      table, PINT_TO_POINTER (keys[counter]),
      PINT_TO_POINTER (values[counter]));

    ++counter;
  }

  for (int i = 0; i < PHASHTABLE_STRESS_COUNT; ++i) {
    ASSERT(p_htable_lookup(table, PINT_TO_POINTER(keys[i])) ==
      PINT_TO_POINTER(values[i]));

    p_htable_remove(table, PINT_TO_POINTER (keys[i]));
    ASSERT(
      p_htable_lookup(table, PINT_TO_POINTER(keys[i])) == (ptr_t) (-1));
  }

  ASSERT(p_htable_keys(table) == NULL);
  ASSERT(p_htable_values(table) == NULL);

  p_free(keys);
  p_free(values);

  p_htable_free(table);

  /* Try to free at once */
  table = p_htable_new();
  ASSERT(table != NULL);

  counter = 0;

  while (counter != PHASHTABLE_STRESS_COUNT) {
    int rand_number = rand();

    if (p_htable_lookup(table, PINT_TO_POINTER (rand_number))
      != (ptr_t) (-1)) {
        continue;
    }

    p_htable_insert(
      table, PINT_TO_POINTER (rand_number),
      PINT_TO_POINTER (rand() + 1));

    ++counter;
  }

  p_htable_free(table);

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
