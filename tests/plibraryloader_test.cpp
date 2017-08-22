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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE plibraryloader_test

#include "plib.h"

#include <stdio.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

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

BOOST_AUTO_TEST_CASE (plibraryloader_nomem_test)
{
	p_libsys_init ();

	if (P_UNLIKELY (p_dl_is_ref_counted () == false)) {
		p_libsys_shutdown ();
		return;
	}

	/* We assume that 3rd argument is ourself library path */
	BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc > 1);

	/* Cleanup from previous run */
	p_file_remove ("." P_DIR_SEP "p_empty_file.txt", NULL);

	FILE *file = fopen ("." P_DIR_SEP "p_empty_file.txt", "w");
	BOOST_CHECK (file != NULL);
	BOOST_CHECK (fclose (file) == 0);

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

#ifdef P_OS_WIN
	SetErrorMode (SEM_FAILCRITICALERRORS);
#endif

	int argCount = boost::unit_test::framework::master_test_suite().argc;

	BOOST_CHECK (p_dl_new ("." P_DIR_SEP "p_empty_file.txt") == NULL);
	BOOST_CHECK (p_dl_new (boost::unit_test::framework::master_test_suite().argv[argCount - 1]) == NULL);

#ifdef P_OS_WIN
	SetErrorMode (0);
#endif

	p_mem_restore_vtable ();

	BOOST_CHECK (p_file_remove ("." P_DIR_SEP "p_empty_file.txt", NULL) == true);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (plibraryloader_general_test)
{
	dl_t	*loader;
	byte_t		*err_msg;
	void		(*shutdown_func) (void);

	p_libsys_init ();

	/* We assume that 3rd argument is ourself library path */
	BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc > 1);

	/* Invalid usage */
	BOOST_CHECK (p_dl_new (NULL) == NULL);
	BOOST_CHECK (p_dl_new ("./unexistent_file.nofile") == NULL);
	BOOST_CHECK (p_dl_get_symbol (NULL, NULL) == NULL);
	BOOST_CHECK (p_dl_get_symbol (NULL, "unexistent_symbol") == NULL);

	p_dl_free (NULL);

	/* General tests */

	/* At least not on HP-UX it should be true */
#if !defined (P_OS_HPUX)
	BOOST_CHECK (p_dl_is_ref_counted () == true);
#else
	p_dl_is_ref_counted ();
#endif

	err_msg = p_dl_get_last_error (NULL);
	p_free (err_msg);

	if (P_UNLIKELY (p_dl_is_ref_counted () == false)) {
		p_libsys_shutdown ();
		return;
	}

	int argCount = boost::unit_test::framework::master_test_suite().argc;

	loader = p_dl_new (boost::unit_test::framework::master_test_suite().argv[argCount - 1]);
	BOOST_REQUIRE (loader != NULL);

	BOOST_CHECK (p_dl_get_symbol (loader, "there_is_no_such_a_symbol") == (PFuncAddr) NULL);

	err_msg = p_dl_get_last_error (loader);
	BOOST_CHECK (err_msg != NULL);
	p_free (err_msg);

	shutdown_func = (void (*) (void)) p_dl_get_symbol (loader, "p_libsys_shutdown");

	if (shutdown_func == NULL)
		shutdown_func = (void (*) (void)) p_dl_get_symbol (loader, "_p_libsys_shutdown");

	BOOST_REQUIRE (shutdown_func != NULL);

	err_msg = p_dl_get_last_error (loader);
	p_free (err_msg);

	p_dl_free (loader);

#ifdef P_OS_BEOS
	p_libsys_shutdown ();
#else
	/* We have already loaded reference to ourself library, it's OK */
	shutdown_func ();
#endif
}

BOOST_AUTO_TEST_SUITE_END()
