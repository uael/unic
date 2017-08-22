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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE perror_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PERROR_TEST_MESSAGE	"PError test error message"
#define PERROR_TEST_MESSAGE_2	"Another PError test error message"

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

BOOST_AUTO_TEST_CASE (perror_nomem_test)
{
	p_libsys_init ();

	err_t *error = p_error_new_literal (0, 0, NULL);
	BOOST_CHECK (error != NULL);

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_error_new () == NULL);
	BOOST_CHECK (p_error_new_literal (0, 0, NULL) == NULL);
	BOOST_CHECK (p_error_copy (error) == NULL);

	p_mem_restore_vtable ();

	p_error_free (error);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (perror_invalid_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_error_get_message (NULL) == NULL);
	BOOST_CHECK (p_error_get_code (NULL) == 0);
	BOOST_CHECK (p_error_get_native_code (NULL) == 0);
	BOOST_CHECK (p_error_get_domain (NULL) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (p_error_copy (NULL) == NULL);

	err_t *error = (err_t *) 0x1;

	p_error_set_code (NULL, 0);
	p_error_set_native_code (NULL, 0);
	p_error_set_message (NULL, NULL);

	p_error_set_error (NULL, 0, 0, NULL);
	p_error_set_error_p (NULL, 0, 0, NULL);

	p_error_set_error_p (&error, 0, 0, NULL);
	BOOST_CHECK (error == (err_t *) 0x1);

	p_error_clear (NULL);
	p_error_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (perror_general_test)
{
	p_libsys_init ();

	/* Empty initialization test */
	err_t *error = p_error_new ();

	BOOST_CHECK (error != NULL);
	BOOST_CHECK (p_error_get_code (error) == 0);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (p_error_get_message (error) == NULL);

	err_t *copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 0);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (p_error_get_message (copy_error) == NULL);

	p_error_free (copy_error);
	copy_error = NULL;

	p_error_set_error (error, (int_t) P_ERR_DOMAIN_IO, -10, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == (int_t) P_ERR_DOMAIN_IO);
	BOOST_CHECK (p_error_get_native_code (error) == -10);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_IO);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	/* Change internal data */
	p_error_set_code (error, (int_t) P_ERR_DOMAIN_IPC);
	p_error_set_native_code (error, -20);
	p_error_set_message (error, PERROR_TEST_MESSAGE_2);

	BOOST_CHECK (p_error_get_code (error) == (int_t) P_ERR_DOMAIN_IPC);
	BOOST_CHECK (p_error_get_native_code (error) == -20);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_IPC);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE_2) == 0);

	/* Revert data back */
	p_error_set_code (error, 10);
	p_error_set_native_code (error, -10);
	p_error_set_message (error, PERROR_TEST_MESSAGE);

	copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 10);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (p_error_get_native_code (copy_error) == -10);

	BOOST_CHECK (strcmp (p_error_get_message (copy_error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (copy_error);
	copy_error = NULL;

	p_error_set_error (error, 20, -20, PERROR_TEST_MESSAGE_2);

	BOOST_CHECK (p_error_get_code (error) == 20);
	BOOST_CHECK (p_error_get_native_code (error) == -20);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE_2) == 0);

	p_error_clear (error);

	BOOST_CHECK (p_error_get_code (error) == 0);
	BOOST_CHECK (p_error_get_native_code (error) == 0);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (p_error_get_message (error) == NULL);

	p_error_free (error);
	error = NULL;

	/* Literal initialization test */
	error = p_error_new_literal (30, -30, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == 30);
	BOOST_CHECK (p_error_get_native_code (error) == -30);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 30);
	BOOST_CHECK (p_error_get_native_code (copy_error) == -30);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (strcmp (p_error_get_message (copy_error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (copy_error);
	p_error_free (error);

	/* Through the double pointer */
	error = NULL;
	p_error_set_error_p (&error, 10, -10, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == 10);
	BOOST_CHECK (p_error_get_native_code (error) == -10);
	BOOST_CHECK (p_error_get_domain (error) == P_ERR_DOMAIN_NONE);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (error);

	/* System codes */
	p_error_set_last_system (10);
	BOOST_CHECK (p_error_get_last_system () == 10);
	p_error_set_last_system (0);
	BOOST_CHECK (p_error_get_last_system () == 0);

#ifndef P_OS_OS2
	p_error_set_last_net (20);
	BOOST_CHECK (p_error_get_last_net () == 20);
	p_error_set_last_net (0);
	BOOST_CHECK (p_error_get_last_net () == 0);
#endif

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
