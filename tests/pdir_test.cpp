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

#define BOOST_TEST_MODULE pdir_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PDIR_ENTRY_DIR		"test_2"
#define PDIR_ENTRY_FILE		"test_file.txt"
#define PDIR_TEST_DIR		"." P_DIR_SEPARATOR "pdir_test_dir"
#define PDIR_TEST_DIR_IN	"." P_DIR_SEPARATOR "pdir_test_dir" P_DIR_SEPARATOR "test_2"
#define PDIR_TEST_FILE		"." P_DIR_SEPARATOR "pdir_test_dir" P_DIR_SEPARATOR "test_file.txt"

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

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pdir_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	/* Cleanup previous run */
	p_dir_remove (PDIR_TEST_DIR_IN, NULL);
	p_dir_remove (PDIR_TEST_DIR, NULL);

	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == true);
	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == true);

	BOOST_CHECK (p_dir_new (PDIR_TEST_DIR"/", NULL) == NULL);

	/* Revert memory management back */
	p_mem_restore_vtable ();

	/* Try out of memory when iterating */
	p_dir_t *dir = p_dir_new (PDIR_TEST_DIR"/", NULL);
	BOOST_CHECK (dir != NULL);

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_dir_get_next_entry (dir, NULL) == NULL);

	/* Cleanup */
	p_mem_restore_vtable ();

	p_dir_free (dir);

	BOOST_CHECK (p_dir_remove (PDIR_TEST_DIR_IN, NULL) == true);
	BOOST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == true);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pdir_general_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_dir_new (NULL, NULL) == NULL);
	BOOST_CHECK (p_dir_new ("." P_DIR_SEPARATOR "pdir_test_dir_new", NULL) == NULL);
	BOOST_CHECK (p_dir_create (NULL, -1, NULL) == false);
#ifndef P_OS_VMS
	BOOST_CHECK (p_dir_create ("." P_DIR_SEPARATOR "pdir_test_dir_new" P_DIR_SEPARATOR "test_dir", -1, NULL) == false);
#endif
	BOOST_CHECK (p_dir_remove (NULL, NULL) == false);
	BOOST_CHECK (p_dir_remove ("." P_DIR_SEPARATOR "pdir_test_dir_new", NULL) == false);
	BOOST_CHECK (p_dir_is_exists (NULL) == false);
	BOOST_CHECK (p_dir_is_exists ("." P_DIR_SEPARATOR "pdir_test_dir_new") == false);
	BOOST_CHECK (p_dir_get_path (NULL) == NULL);
	BOOST_CHECK (p_dir_get_next_entry (NULL, NULL) == NULL);
	BOOST_CHECK (p_dir_rewind (NULL, NULL) == false);

	p_dir_entry_free (NULL);
	p_dir_free (NULL);

	/* Cleanup previous run */
	p_dir_remove (PDIR_TEST_DIR_IN, NULL);
	p_dir_remove (PDIR_TEST_DIR, NULL);

	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == true);
	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == true);
	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == true);
	BOOST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == true);

	FILE *file = fopen (PDIR_TEST_FILE, "w");
	BOOST_REQUIRE (file != NULL);
	BOOST_REQUIRE (p_file_is_exists (PDIR_TEST_FILE) == true);

	fprintf (file, "This is a test file string\n");

	BOOST_CHECK (fclose (file) == 0);

	BOOST_CHECK (p_dir_is_exists (PDIR_TEST_DIR) == true);
	BOOST_CHECK (p_dir_is_exists (PDIR_TEST_DIR_IN) == true);

	p_dir_t *dir = p_dir_new (PDIR_TEST_DIR"/", NULL);

	BOOST_CHECK (dir != NULL);

	int_t dir_count	= 0;
	int_t file_count	= 0;
	bool has_entry_dir	= false;
	bool has_entry_file	= false;

	p_dirent_t *entry;

	while ((entry = p_dir_get_next_entry (dir, NULL)) != NULL) {
		BOOST_CHECK (entry->name != NULL);

		switch (entry->type) {
		case P_DIR_ENTRY_TYPE_DIR:
			++dir_count;
			break;
		case P_DIR_ENTRY_TYPE_FILE:
			++file_count;
			break;
		case P_DIR_ENTRY_TYPE_OTHER:
		default:
			break;
		}

		if (strcmp (entry->name, PDIR_ENTRY_DIR) == 0)
			has_entry_dir = true;
		else if (strcmp (entry->name, PDIR_ENTRY_FILE) == 0)
			has_entry_file = true;

		p_dir_entry_free (entry);
	}

	BOOST_CHECK (dir_count > 0 && dir_count < 4);
	BOOST_CHECK (file_count == 1);
	BOOST_CHECK (has_entry_dir == true);
	BOOST_CHECK (has_entry_file == true);

	BOOST_CHECK (p_dir_rewind (dir, NULL) == true);

	int_t dir_count_2  = 0;
	int_t file_count_2 = 0;
	has_entry_dir	= false;
	has_entry_file	= false;

	while ((entry = p_dir_get_next_entry (dir, NULL)) != NULL) {
		BOOST_CHECK (entry->name != NULL);

		switch (entry->type) {
		case P_DIR_ENTRY_TYPE_DIR:
			++dir_count_2;
			break;
		case P_DIR_ENTRY_TYPE_FILE:
			++file_count_2;
			break;
		case P_DIR_ENTRY_TYPE_OTHER:
		default:
			break;
		}

		if (strcmp (entry->name, PDIR_ENTRY_DIR) == 0)
			has_entry_dir = true;
		else if (strcmp (entry->name, PDIR_ENTRY_FILE) == 0)
			has_entry_file = true;

		p_dir_entry_free (entry);
	}

	BOOST_CHECK (dir_count_2 > 0 && dir_count_2 < 4);
	BOOST_CHECK (file_count_2 == 1);
	BOOST_CHECK (has_entry_dir == true);
	BOOST_CHECK (has_entry_file == true);

	/* Compare two previous attempts */
	BOOST_CHECK (dir_count == dir_count_2);
	BOOST_CHECK (file_count == file_count_2);

	/* Remove all stuff */
	BOOST_CHECK (p_file_remove (PDIR_TEST_FILE, NULL) == true);
	BOOST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == false);
	BOOST_CHECK (p_dir_remove (PDIR_TEST_DIR_IN, NULL) == true);
	BOOST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == true);

	BOOST_CHECK (p_dir_is_exists (PDIR_TEST_DIR_IN) == false);
	BOOST_CHECK (p_dir_is_exists (PDIR_TEST_DIR) == false);

	byte_t *orig_path = p_dir_get_path (dir);
	BOOST_CHECK (strcmp (orig_path, PDIR_TEST_DIR"/") == 0);
	p_free (orig_path);

	p_dir_free (dir);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
