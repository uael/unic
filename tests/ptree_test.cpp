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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ptree_test

#include "plib.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

#define PTREE_STRESS_ITERATIONS	20
#define PTREE_STRESS_NODES	10000
#define PTREE_STRESS_ROOT_MIN	10000
#define PTREE_STRESS_TRAVS	30

typedef struct _TreeData {
	int_t	cmp_counter;
	int_t	key_destroy_counter;
	int_t	value_destroy_counter;
	int_t	traverse_counter;
	int_t	traverse_thres;
	int_t	key_sum;
	int_t	value_sum;
	int_t	last_key;
	int_t	key_order_errors;
} TreeData;

static TreeData tree_data = {0, 0, 0, 0, 0, 0, 0, 0, 0};

extern "C" ptr_t pmem_alloc (size_t nbytes)
{
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" ptr_t pmem_realloc (ptr_t block, size_t nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" void pmem_free (ptr_t block)
{
	P_UNUSED (block);
}

static int_t
tree_complexity (tree_t *tree)
{
	if (tree == NULL || p_tree_get_nnodes (tree) == 0)
		return 0;

	switch (p_tree_get_type (tree)) {
	case P_TREE_TYPE_BINARY:
		return p_tree_get_nnodes (tree);
	case P_TREE_TYPE_RB:
		return 2 * ((int_t) (log ((double) p_tree_get_nnodes (tree) + 1) / log (2.0)));
	case P_TREE_TYPE_AVL:
	{
		double phi = (1 + sqrt (5.0)) / 2.0;
		return (int_t) (log (sqrt (5.0) * (p_tree_get_nnodes (tree) + 2)) / log (phi) - 2);
	}
	default:
		return p_tree_get_nnodes (tree);
	}
}

static int_t
compare_keys (const_ptr_t a, const_ptr_t b)
{
	int p1 = PPOINTER_TO_INT (a);
	int p2 = PPOINTER_TO_INT (b);

	if (p1 < p2)
		return -1;
	else if (p1 > p2)
		return 1;
	else
		return 0;
}

static int_t
compare_keys_data (const_ptr_t a, const_ptr_t b, ptr_t data)
{
	int p1 = PPOINTER_TO_INT (a);
	int p2 = PPOINTER_TO_INT (b);

	if (data != NULL)
		((TreeData *) data)->cmp_counter++;

	if (p1 < p2)
		return -1;
	else if (p1 > p2)
		return 1;
	else
		return 0;
}

static void
key_destroy_notify (ptr_t data)
{
	tree_data.key_destroy_counter++;
	tree_data.key_sum += PPOINTER_TO_INT (data);
}

static void
value_destroy_notify (ptr_t data)
{
	tree_data.value_destroy_counter++;
	tree_data.value_sum += PPOINTER_TO_INT (data);
}

static bool
tree_traverse (ptr_t key, ptr_t value, ptr_t data)
{
	TreeData* tdata = ((TreeData *) data);

	tdata->key_sum	 += PPOINTER_TO_INT (key);
	tdata->value_sum += PPOINTER_TO_INT (value);
	tdata->traverse_counter++;

	if (tdata->last_key >= PPOINTER_TO_INT (key))
		tdata->key_order_errors++;

	tdata->last_key = PPOINTER_TO_INT (key);

	return false;
}

static bool
tree_traverse_thres (ptr_t key, ptr_t value, ptr_t data)
{
	TreeData* tdata = ((TreeData *) data);

	tree_traverse (key, value, data);

	return tdata->traverse_counter >= tdata->traverse_thres ? true : false;
}

static bool
check_tree_data_is_zero ()
{
	return tree_data.cmp_counter           == 0 &&
	       tree_data.key_destroy_counter   == 0 &&
	       tree_data.value_destroy_counter == 0 &&
	       tree_data.traverse_counter      == 0 &&
	       tree_data.traverse_thres	       == 0 &&
	       tree_data.key_sum               == 0 &&
	       tree_data.value_sum             == 0 &&
	       tree_data.last_key              == 0 &&
	       tree_data.key_order_errors      == 0;
}

static bool
general_tree_test (tree_t *tree, PTreeType type, bool check_cmp, bool check_notify)
{
	memset (&tree_data, 0, sizeof (tree_data));

	BOOST_REQUIRE (tree != NULL);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);
	BOOST_CHECK (p_tree_get_type (tree) == type);
	BOOST_CHECK (p_tree_lookup (tree, NULL) == NULL);
	BOOST_CHECK (p_tree_remove (tree, NULL) == false);

	p_tree_insert (tree, NULL, PINT_TO_POINTER (10));
	BOOST_CHECK (p_tree_get_nnodes (tree) == 1);
	BOOST_CHECK (p_tree_lookup (tree, NULL) == PINT_TO_POINTER (10));
	BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (2)) == NULL);
	BOOST_CHECK (p_tree_remove (tree, NULL) == true);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

	p_tree_foreach (tree, (traverse_fn_t) tree_traverse, &tree_data);
	BOOST_CHECK (tree_data.traverse_counter == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	/* Because we have NULL-key node */
	BOOST_CHECK (tree_data.key_sum == 0);

	if (check_notify)
		BOOST_CHECK (tree_data.value_sum == 10);
	else
		BOOST_CHECK (tree_data.value_sum == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	p_tree_insert (tree, PINT_TO_POINTER (4), PINT_TO_POINTER (40));
	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));
	p_tree_insert (tree, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
	p_tree_insert (tree, PINT_TO_POINTER (6), PINT_TO_POINTER (60));
	p_tree_insert (tree, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (100));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (500));

	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));

	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0);
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		BOOST_CHECK (tree_data.key_sum   == 12);
		BOOST_CHECK (tree_data.value_sum == 660);
	} else {
		BOOST_CHECK (tree_data.key_sum   == 0);
		BOOST_CHECK (tree_data.value_sum == 0);
	}

	BOOST_CHECK (tree_data.traverse_counter == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	p_tree_foreach (tree, (traverse_fn_t) tree_traverse, &tree_data);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	BOOST_CHECK (tree_data.cmp_counter      == 0);
	BOOST_CHECK (tree_data.key_sum          == 21);
	BOOST_CHECK (tree_data.value_sum        == 210);
	BOOST_CHECK (tree_data.traverse_counter == 6);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	for (int i = 0; i < 7; ++i)
		BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0);
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	BOOST_CHECK (tree_data.key_sum          == 0);
	BOOST_CHECK (tree_data.value_sum        == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	tree_data.cmp_counter = 0;

	BOOST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (7)) == false);

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0 &&
			     tree_data.cmp_counter <= tree_complexity (tree));
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		BOOST_CHECK (tree_data.key_sum   == 0);
		BOOST_CHECK (tree_data.value_sum == 0);
	}

	tree_data.cmp_counter = 0;

	for (int i = 0; i < 7; ++i)
		BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0);
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	BOOST_CHECK (tree_data.key_sum          == 0);
	BOOST_CHECK (tree_data.value_sum        == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	tree_data.traverse_thres = 5;

	p_tree_foreach (tree, (traverse_fn_t) tree_traverse_thres, &tree_data);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	BOOST_CHECK (tree_data.cmp_counter      == 0);
	BOOST_CHECK (tree_data.key_sum          == 15);
	BOOST_CHECK (tree_data.value_sum        == 150);
	BOOST_CHECK (tree_data.traverse_counter == 5);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	tree_data.traverse_thres = 3;

	p_tree_foreach (tree, (traverse_fn_t) tree_traverse_thres, &tree_data);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 6);

	BOOST_CHECK (tree_data.cmp_counter      == 0);
	BOOST_CHECK (tree_data.key_sum          == 6);
	BOOST_CHECK (tree_data.value_sum        == 60);
	BOOST_CHECK (tree_data.traverse_counter == 3);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	BOOST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (1)) == true);
	BOOST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (6)) == true);
	BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (1)) == NULL);
	BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (6)) == NULL);

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0);
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		BOOST_CHECK (tree_data.key_sum   == 7);
		BOOST_CHECK (tree_data.value_sum == 70);
	} else {
		BOOST_CHECK (tree_data.key_sum   == 0);
		BOOST_CHECK (tree_data.value_sum == 0);
	}

	tree_data.cmp_counter = 0;

	for (int i = 2; i < 6; ++i)
		BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		BOOST_CHECK (tree_data.cmp_counter > 0);
	else
		BOOST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		BOOST_CHECK (tree_data.key_sum   == 7);
		BOOST_CHECK (tree_data.value_sum == 70);
	} else {
		BOOST_CHECK (tree_data.key_sum   == 0);
		BOOST_CHECK (tree_data.value_sum == 0);
	}

	BOOST_CHECK (tree_data.key_order_errors == 0);

	tree_data.cmp_counter = 0;

	p_tree_foreach (tree, NULL, NULL);

	BOOST_CHECK (tree_data.cmp_counter      == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	p_tree_clear (tree);

	BOOST_CHECK (tree_data.cmp_counter      == 0);
	BOOST_CHECK (tree_data.key_order_errors == 0);

	if (check_notify) {
		BOOST_CHECK (tree_data.key_sum   == 21);
		BOOST_CHECK (tree_data.value_sum == 210);
	} else {
		BOOST_CHECK (tree_data.key_sum   == 0);
		BOOST_CHECK (tree_data.value_sum == 0);
	}

	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

	return true;
}

static bool
stress_tree_test (tree_t *tree, int node_count)
{
	BOOST_REQUIRE (tree != NULL);
	BOOST_REQUIRE (node_count > 0);
	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

	srand ((unsigned int) time (NULL));

	int counter = 0;

	memset (&tree_data, 0, sizeof (tree_data));

	int_t *keys   = (int_t *) p_malloc0 ((size_t) node_count * sizeof (int_t));
	int_t *values = (int_t *) p_malloc0 ((size_t) node_count * sizeof (int_t));

	BOOST_REQUIRE (keys != NULL);
	BOOST_REQUIRE (values != NULL);

	while (counter != node_count) {
		int_t rand_number = rand ();

		if (counter == 0 && rand_number < PTREE_STRESS_ROOT_MIN)
			continue;

		memset (&tree_data, 0, sizeof (tree_data));

		if (p_tree_lookup (tree, PINT_TO_POINTER (rand_number)) != NULL)
			continue;

		if (counter > 0)
			BOOST_CHECK (tree_data.cmp_counter > 0 &&
				     tree_data.cmp_counter <= tree_complexity (tree));

		memset (&tree_data, 0, sizeof (tree_data));

		keys[counter]   = rand_number;
		values[counter] = rand () + 1;

		p_tree_insert (tree, PINT_TO_POINTER (keys[counter]), PINT_TO_POINTER (values[counter]));

		if (counter > 0)
			BOOST_CHECK (tree_data.cmp_counter > 0 &&
				     tree_data.cmp_counter <= tree_complexity (tree));

		++counter;
	}

	for (int i = 0; i < PTREE_STRESS_TRAVS; ++i) {
		memset (&tree_data, 0, sizeof (tree_data));

		tree_data.traverse_thres = i + 1;
		tree_data.last_key       = -1;

		p_tree_foreach (tree, (traverse_fn_t) tree_traverse_thres, &tree_data);

		BOOST_CHECK (tree_data.traverse_counter == i + 1);
		BOOST_CHECK (tree_data.key_order_errors == 0);
	}

	for (int i = 0; i < node_count; ++i) {
		memset (&tree_data, 0, sizeof (tree_data));

		BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (keys[i])) ==
			     PINT_TO_POINTER (values[i]));

		BOOST_CHECK (tree_data.cmp_counter > 0 &&
			     tree_data.cmp_counter <= tree_complexity (tree));

		BOOST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (keys[i])) == true);
		BOOST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (keys[i])) == NULL);
	}

	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

	for (int i = 0; i < node_count; ++i)
		p_tree_insert (tree, PINT_TO_POINTER (keys[i]), PINT_TO_POINTER (values[i]));

	BOOST_CHECK (p_tree_get_nnodes (tree) == node_count);

	p_tree_clear (tree);

	BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

	p_free (keys);
	p_free (values);

	return true;
}

BOOST_AUTO_TEST_CASE (ptree_nomem_test)
{
	p_libsys_init ();

	mem_vtable_t vtable;

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		tree_t *tree = p_tree_new ((PTreeType) i, (cmp_fn_t) compare_keys);
		BOOST_CHECK (tree != NULL);

		vtable.free    = pmem_free;
		vtable.malloc  = pmem_alloc;
		vtable.realloc = pmem_realloc;

		BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

		BOOST_CHECK (p_tree_new ((PTreeType) i, (cmp_fn_t) compare_keys) == NULL);
		p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
		BOOST_CHECK (p_tree_get_nnodes (tree) == 0);

		p_mem_restore_vtable ();

		p_tree_free (tree);
	}

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptree_invalid_test)
{
	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		/* Invalid usage */
		BOOST_CHECK (p_tree_new ((PTreeType) i, NULL) == NULL);
		BOOST_CHECK (p_tree_new ((PTreeType) -1, (cmp_fn_t) compare_keys) == NULL);
		BOOST_CHECK (p_tree_new ((PTreeType) -1, NULL) == NULL);

		BOOST_CHECK (p_tree_new_with_data ((PTreeType) i, NULL, NULL) == NULL);
		BOOST_CHECK (p_tree_new_with_data ((PTreeType) -1, (cmp_data_fn_t) compare_keys, NULL) == NULL);
		BOOST_CHECK (p_tree_new_with_data ((PTreeType) -1, NULL, NULL) == NULL);

		BOOST_CHECK (p_tree_new_full ((PTreeType) i,
					      NULL,
					      NULL,
					      NULL,
					      NULL) == NULL);
		BOOST_CHECK (p_tree_new_full ((PTreeType) -1,
					      (cmp_data_fn_t) compare_keys,
					      NULL,
					      NULL,
					      NULL) == NULL);
		BOOST_CHECK (p_tree_new_full ((PTreeType) -1,
					      NULL,
					      NULL,
					      NULL,
					      NULL) == NULL);

		BOOST_CHECK (p_tree_remove (NULL, NULL) == false);
		BOOST_CHECK (p_tree_lookup (NULL, NULL) == NULL);
		BOOST_CHECK (p_tree_get_type (NULL) == (PTreeType) -1);
		BOOST_CHECK (p_tree_get_nnodes (NULL) == 0);

		p_tree_insert (NULL, NULL, NULL);
		p_tree_foreach (NULL, NULL, NULL);
		p_tree_clear (NULL);
		p_tree_free (NULL);
	}

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptree_general_test)
{
	tree_t *tree;

	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		/* Test 1 */
		tree = p_tree_new ((PTreeType) i, (cmp_fn_t) compare_keys);

		BOOST_CHECK (general_tree_test (tree, (PTreeType) i, false, false) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		BOOST_CHECK (check_tree_data_is_zero () == true);

		/* Test 2 */
		tree = p_tree_new_with_data ((PTreeType) i,
					     (cmp_data_fn_t) compare_keys_data,
					     &tree_data);

		BOOST_CHECK (general_tree_test (tree, (PTreeType) i, true, false) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		BOOST_CHECK (check_tree_data_is_zero () == true);

		/* Test 3 */
		tree = p_tree_new_full ((PTreeType) i,
					(cmp_data_fn_t) compare_keys_data,
					&tree_data,
					(destroy_fn_t) key_destroy_notify,
					(destroy_fn_t) value_destroy_notify);
		BOOST_CHECK (general_tree_test (tree, (PTreeType) i, true, true) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		BOOST_CHECK (check_tree_data_is_zero () == true);
	}

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptree_stress_test)
{
	tree_t *tree;

	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		tree = p_tree_new_full ((PTreeType) i,
					(cmp_data_fn_t) compare_keys_data,
					&tree_data,
					(destroy_fn_t) key_destroy_notify,
					(destroy_fn_t) value_destroy_notify);

		for (int j = 0; j < PTREE_STRESS_ITERATIONS; ++j)
			BOOST_CHECK (stress_tree_test (tree, PTREE_STRESS_NODES) == true);

		p_tree_free (tree);
	}

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
