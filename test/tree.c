/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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
#include <math.h>
#include "cute.h"
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PTREE_STRESS_ITERATIONS  20
#define PTREE_STRESS_NODES  10000
#define PTREE_STRESS_ROOT_MIN  10000
#define PTREE_STRESS_TRAVS  30
typedef struct tree_data {
  int cmp_counter;
  int key_destroy_counter;
  int value_destroy_counter;
  int traverse_counter;
  int traverse_thres;
  int key_sum;
  int value_sum;
  int last_key;
  int key_order_errors;
} tree_data_t;

static tree_data_t tree_data = {0, 0, 0, 0, 0, 0, 0, 0, 0};

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
tree_complexity(tree_t *tree) {
  if (tree == NULL || p_tree_get_nnodes(tree) == 0) {
    return 0;
  }
  switch (p_tree_get_type(tree)) {
    case P_TREE_TYPE_BINARY:
      return p_tree_get_nnodes(tree);
    case P_TREE_TYPE_RB:
      return 2 * ((int) (log((double) p_tree_get_nnodes(tree) + 1) / log(2.0)));
    case P_TREE_TYPE_AVL: {
      double phi = (1 + sqrt(5.0)) / 2.0;
      return (int) (
        log(sqrt(5.0) * (p_tree_get_nnodes(tree) + 2)) / log(phi) - 2
      );
    }
    default:
      return p_tree_get_nnodes(tree);
  }
}

static int
compare_keys(const_ptr_t a, const_ptr_t b) {
  int p1;
  int p2;

  p1 = PPOINTER_TO_INT (a);
  p2 = PPOINTER_TO_INT (b);
  if (p1 < p2) {
    return -1;
  } else if (p1 > p2) {
    return 1;
  } else {
    return 0;
  }
}

static int
compare_keys_data(const_ptr_t a, const_ptr_t b, ptr_t data) {
  int p1;
  int p2;

  p1 = PPOINTER_TO_INT (a);
  p2 = PPOINTER_TO_INT (b);
  if (data != NULL) {
    ((tree_data_t *) data)->cmp_counter++;
  }
  if (p1 < p2) {
    return -1;
  } else if (p1 > p2) {
    return 1;
  } else {
    return 0;
  }
}

static void
key_destroy_notify(ptr_t data) {
  tree_data.key_destroy_counter++;
  tree_data.key_sum += PPOINTER_TO_INT (data);
}

static void
value_destroy_notify(ptr_t data) {
  tree_data.value_destroy_counter++;
  tree_data.value_sum += PPOINTER_TO_INT (data);
}

static bool
tree_traverse(ptr_t key, ptr_t value, ptr_t data) {
  tree_data_t *tdata;

  tdata = ((tree_data_t *) data);
  tdata->key_sum += PPOINTER_TO_INT (key);
  tdata->value_sum += PPOINTER_TO_INT (value);
  tdata->traverse_counter++;
  if (tdata->last_key >= PPOINTER_TO_INT (key)) {
    tdata->key_order_errors++;
  }
  tdata->last_key = PPOINTER_TO_INT (key);
  return false;
}

static bool
tree_traverse_thres(ptr_t key, ptr_t value, ptr_t data) {
  tree_data_t *tdata;

  tdata = ((tree_data_t *) data);
  tree_traverse(key, value, data);
  return tdata->traverse_counter >= tdata->traverse_thres ? true : false;
}

static bool
check_tree_data_is_zero() {
  return tree_data.cmp_counter == 0 &&
    tree_data.key_destroy_counter == 0 &&
    tree_data.value_destroy_counter == 0 &&
    tree_data.traverse_counter == 0 &&
    tree_data.traverse_thres == 0 &&
    tree_data.key_sum == 0 &&
    tree_data.value_sum == 0 &&
    tree_data.last_key == 0 &&
    tree_data.key_order_errors == 0;
}

static bool
general_tree_test(tree_t *tree, tree_kind_t type, bool check_cmp,
  bool check_notify) {
  int i;

  memset(&tree_data, 0, sizeof(tree_data));
  ASSERT(tree != NULL);
  ASSERT(p_tree_get_nnodes(tree) == 0);
  ASSERT(p_tree_get_type(tree) == type);
  ASSERT(p_tree_lookup(tree, NULL) == NULL);
  ASSERT(p_tree_remove(tree, NULL) == false);
  p_tree_insert(tree, NULL, PINT_TO_POINTER (10));
  ASSERT(p_tree_get_nnodes(tree) == 1);
  ASSERT(p_tree_lookup(tree, NULL) == PINT_TO_POINTER(10));
  ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(2)) == NULL);
  ASSERT(p_tree_remove(tree, NULL) == true);
  ASSERT(p_tree_get_nnodes(tree) == 0);
  p_tree_foreach(tree, (traverse_fn_t) tree_traverse, &tree_data);
  ASSERT(tree_data.traverse_counter == 0);
  ASSERT(tree_data.key_order_errors == 0);

  /* Because we have NULL-key node */
  ASSERT(tree_data.key_sum == 0);
  if (check_notify)
    ASSERT(tree_data.value_sum == 10);
  else
    ASSERT(tree_data.value_sum == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  p_tree_insert(tree, PINT_TO_POINTER (4), PINT_TO_POINTER (40));
  p_tree_insert(tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  p_tree_insert(tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));
  p_tree_insert(tree, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
  p_tree_insert(tree, PINT_TO_POINTER (6), PINT_TO_POINTER (60));
  p_tree_insert(tree, PINT_TO_POINTER (3), PINT_TO_POINTER (30));
  ASSERT(p_tree_get_nnodes(tree) == 6);
  p_tree_insert(tree, PINT_TO_POINTER (1), PINT_TO_POINTER (100));
  p_tree_insert(tree, PINT_TO_POINTER (5), PINT_TO_POINTER (500));
  ASSERT(p_tree_get_nnodes(tree) == 6);
  p_tree_insert(tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
  p_tree_insert(tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));
  ASSERT(p_tree_get_nnodes(tree) == 6);
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0);
  else
    ASSERT(tree_data.cmp_counter == 0);
  if (check_notify) {
    ASSERT(tree_data.key_sum == 12);
    ASSERT(tree_data.value_sum == 660);
  } else {
    ASSERT(tree_data.key_sum == 0);
    ASSERT(tree_data.value_sum == 0);
  }
  ASSERT(tree_data.traverse_counter == 0);
  ASSERT(tree_data.key_order_errors == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  p_tree_foreach(tree, (traverse_fn_t) tree_traverse, &tree_data);
  ASSERT(p_tree_get_nnodes(tree) == 6);
  ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_sum == 21);
  ASSERT(tree_data.value_sum == 210);
  ASSERT(tree_data.traverse_counter == 6);
  ASSERT(tree_data.key_order_errors == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  for (i = 0; i < 7; ++i)
    ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(i)) == PINT_TO_POINTER(i * 10));
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0);
  else
    ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_sum == 0);
  ASSERT(tree_data.value_sum == 0);
  ASSERT(tree_data.key_order_errors == 0);
  tree_data.cmp_counter = 0;
  ASSERT(p_tree_remove(tree, PINT_TO_POINTER(7)) == false);
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0 &&
      tree_data.cmp_counter <= tree_complexity(tree));
  else
    ASSERT(tree_data.cmp_counter == 0);
  if (check_notify) {
    ASSERT(tree_data.key_sum == 0);
    ASSERT(tree_data.value_sum == 0);
  }
  tree_data.cmp_counter = 0;
  for (i = 0; i < 7; ++i)
    ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(i)) == PINT_TO_POINTER(i * 10));
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0);
  else
    ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_sum == 0);
  ASSERT(tree_data.value_sum == 0);
  ASSERT(tree_data.key_order_errors == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  tree_data.traverse_thres = 5;
  p_tree_foreach(tree, (traverse_fn_t) tree_traverse_thres, &tree_data);
  ASSERT(p_tree_get_nnodes(tree) == 6);
  ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_sum == 15);
  ASSERT(tree_data.value_sum == 150);
  ASSERT(tree_data.traverse_counter == 5);
  ASSERT(tree_data.key_order_errors == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  tree_data.traverse_thres = 3;
  p_tree_foreach(tree, (traverse_fn_t) tree_traverse_thres, &tree_data);
  ASSERT(p_tree_get_nnodes(tree) == 6);
  ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_sum == 6);
  ASSERT(tree_data.value_sum == 60);
  ASSERT(tree_data.traverse_counter == 3);
  ASSERT(tree_data.key_order_errors == 0);
  memset(&tree_data, 0, sizeof(tree_data));
  ASSERT(p_tree_remove(tree, PINT_TO_POINTER(1)) == true);
  ASSERT(p_tree_remove(tree, PINT_TO_POINTER(6)) == true);
  ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(1)) == NULL);
  ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(6)) == NULL);
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0);
  else
    ASSERT(tree_data.cmp_counter == 0);
  if (check_notify) {
    ASSERT(tree_data.key_sum == 7);
    ASSERT(tree_data.value_sum == 70);
  } else {
    ASSERT(tree_data.key_sum == 0);
    ASSERT(tree_data.value_sum == 0);
  }
  tree_data.cmp_counter = 0;
  for (i = 2; i < 6; ++i)
    ASSERT(
      p_tree_lookup(tree, PINT_TO_POINTER(i)) == PINT_TO_POINTER(i * 10));
  if (check_cmp)
    ASSERT(tree_data.cmp_counter > 0);
  else
    ASSERT(tree_data.cmp_counter == 0);
  if (check_notify) {
    ASSERT(tree_data.key_sum == 7);
    ASSERT(tree_data.value_sum == 70);
  } else {
    ASSERT(tree_data.key_sum == 0);
    ASSERT(tree_data.value_sum == 0);
  }
  ASSERT(tree_data.key_order_errors == 0);
  tree_data.cmp_counter = 0;
  p_tree_foreach(tree, NULL, NULL);
  ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_order_errors == 0);
  p_tree_clear(tree);
  ASSERT(tree_data.cmp_counter == 0);
  ASSERT(tree_data.key_order_errors == 0);
  if (check_notify) {
    ASSERT(tree_data.key_sum == 21);
    ASSERT(tree_data.value_sum == 210);
  } else {
    ASSERT(tree_data.key_sum == 0);
    ASSERT(tree_data.value_sum == 0);
  }
  ASSERT(p_tree_get_nnodes(tree) == 0);
  return true;
}

static bool
stress_tree_test(tree_t *tree, int node_count) {
  int counter;
  int *keys;
  int *values;
  int rand_number;
  int i;

  ASSERT(tree != NULL);
  ASSERT(node_count > 0);
  ASSERT(p_tree_get_nnodes(tree) == 0);
  srand((unsigned int) time(NULL));
  counter = 0;
  memset(&tree_data, 0, sizeof(tree_data));
  keys = (int *) p_malloc0((size_t) node_count * sizeof(int));
  values = (int *) p_malloc0((size_t) node_count * sizeof(int));
  ASSERT(keys != NULL);
  ASSERT(values != NULL);
  while (counter != node_count) {
    rand_number = rand();
    if (counter == 0 && rand_number < PTREE_STRESS_ROOT_MIN) {
      continue;
    }
    memset(&tree_data, 0, sizeof(tree_data));
    if (p_tree_lookup(tree, PINT_TO_POINTER (rand_number)) != NULL) {
      continue;
    }
    if (counter > 0)
      ASSERT(tree_data.cmp_counter > 0 &&
        tree_data.cmp_counter <= tree_complexity(tree));
    memset(&tree_data, 0, sizeof(tree_data));
    keys[counter] = rand_number;
    values[counter] = rand() + 1;
    p_tree_insert(
      tree, PINT_TO_POINTER (keys[counter]),
      PINT_TO_POINTER (values[counter]));
    if (counter > 0)
      ASSERT(tree_data.cmp_counter > 0 &&
        tree_data.cmp_counter <= tree_complexity(tree));
    ++counter;
  }
  for (i = 0; i < PTREE_STRESS_TRAVS; ++i) {
    memset(&tree_data, 0, sizeof(tree_data));
    tree_data.traverse_thres = i + 1;
    tree_data.last_key = -1;
    p_tree_foreach(tree, (traverse_fn_t) tree_traverse_thres, &tree_data);
    ASSERT(tree_data.traverse_counter == i + 1);
    ASSERT(tree_data.key_order_errors == 0);
  }
  for (i = 0; i < node_count; ++i) {
    memset(&tree_data, 0, sizeof(tree_data));
    ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(keys[i])) ==
      PINT_TO_POINTER(values[i]));
    ASSERT(tree_data.cmp_counter > 0 &&
      tree_data.cmp_counter <= tree_complexity(tree));
    ASSERT(p_tree_remove(tree, PINT_TO_POINTER(keys[i])) == true);
    ASSERT(p_tree_lookup(tree, PINT_TO_POINTER(keys[i])) == NULL);
  }
  ASSERT(p_tree_get_nnodes(tree) == 0);
  for (i = 0; i < node_count; ++i) {
    p_tree_insert(tree, PINT_TO_POINTER (keys[i]), PINT_TO_POINTER (values[i]));
  }
  ASSERT(p_tree_get_nnodes(tree) == node_count);
  p_tree_clear(tree);
  ASSERT(p_tree_get_nnodes(tree) == 0);
  p_free(keys);
  p_free(values);
  return true;
}

CUTEST(tree, nomem) {
  int i;
  mem_vtable_t vtable;
  tree_t *tree;

  for (i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
    tree = p_tree_new((tree_kind_t) i, (cmp_fn_t) compare_keys);
    ASSERT(tree != NULL);
    vtable.free = pmem_free;
    vtable.malloc = pmem_alloc;
    vtable.realloc = pmem_realloc;
    ASSERT(p_mem_set_vtable(&vtable) == true);
    ASSERT(p_tree_new((tree_kind_t) i, (cmp_fn_t) compare_keys) == NULL);
    p_tree_insert(tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
    ASSERT(p_tree_get_nnodes(tree) == 0);
    p_mem_restore_vtable();
    p_tree_free(tree);
  }
  return CUTE_SUCCESS;
}

CUTEST(tree, invalid) {
  int i;

  for (i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
    /* Invalid usage */
    ASSERT(p_tree_new((tree_kind_t) i, NULL) == NULL);
    ASSERT(p_tree_new((tree_kind_t) -1, (cmp_fn_t) compare_keys) == NULL);
    ASSERT(p_tree_new((tree_kind_t) -1, NULL) == NULL);
    ASSERT(p_tree_new_with_data((tree_kind_t) i, NULL, NULL) == NULL);
    ASSERT(
      p_tree_new_with_data((tree_kind_t) -1, (cmp_data_fn_t) compare_keys, NULL)
        == NULL
    );
    ASSERT(p_tree_new_with_data((tree_kind_t) -1, NULL, NULL) == NULL);
    ASSERT(p_tree_new_full((tree_kind_t) i,
      NULL,
      NULL,
      NULL,
      NULL
    ) == NULL);
    ASSERT(p_tree_new_full((tree_kind_t) -1,
      (cmp_data_fn_t) compare_keys,
      NULL,
      NULL,
      NULL
    ) == NULL);
    ASSERT(p_tree_new_full((tree_kind_t) -1,
      NULL,
      NULL,
      NULL,
      NULL
    ) == NULL);
    ASSERT(p_tree_remove(NULL, NULL) == false);
    ASSERT(p_tree_lookup(NULL, NULL) == NULL);
    ASSERT(p_tree_get_type(NULL) == (tree_kind_t) -1);
    ASSERT(p_tree_get_nnodes(NULL) == 0);
    p_tree_insert(NULL, NULL, NULL);
    p_tree_foreach(NULL, NULL, NULL);
    p_tree_clear(NULL);
    p_tree_free(NULL);
  }
  return CUTE_SUCCESS;
}

CUTEST(tree, general) {
  tree_t *tree;
  int i;

  for (i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
    /* Test 1 */
    tree = p_tree_new((tree_kind_t) i, (cmp_fn_t) compare_keys);
    ASSERT(general_tree_test(tree, (tree_kind_t) i, false, false) == true);
    memset(&tree_data, 0, sizeof(tree_data));
    p_tree_free(tree);
    ASSERT(check_tree_data_is_zero() == true);

    /* Test 2 */
    tree = p_tree_new_with_data((tree_kind_t) i,
      (cmp_data_fn_t) compare_keys_data,
      &tree_data
    );
    ASSERT(general_tree_test(tree, (tree_kind_t) i, true, false) == true);
    memset(&tree_data, 0, sizeof(tree_data));
    p_tree_free(tree);
    ASSERT(check_tree_data_is_zero() == true);

    /* Test 3 */
    tree = p_tree_new_full((tree_kind_t) i,
      (cmp_data_fn_t) compare_keys_data,
      &tree_data,
      (destroy_fn_t) key_destroy_notify,
      (destroy_fn_t) value_destroy_notify
    );
    ASSERT(general_tree_test(tree, (tree_kind_t) i, true, true) == true);
    memset(&tree_data, 0, sizeof(tree_data));
    p_tree_free(tree);
    ASSERT(check_tree_data_is_zero() == true);
  }
  return CUTE_SUCCESS;
}

CUTEST(tree, stress) {
  tree_t *tree;
  int i;
  int j;

  for (i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
    tree = p_tree_new_full((tree_kind_t) i,
      (cmp_data_fn_t) compare_keys_data,
      &tree_data,
      (destroy_fn_t) key_destroy_notify,
      (destroy_fn_t) value_destroy_notify
    );
    for (j = 0; j < PTREE_STRESS_ITERATIONS; ++j)
      ASSERT(stress_tree_test(tree, PTREE_STRESS_NODES) == true);
    p_tree_free(tree);
  }
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(tree, nomem);
  CUTEST_PASS(tree, invalid);
  CUTEST_PASS(tree, general);
  CUTEST_PASS(tree, stress);
  return EXIT_SUCCESS;
}
