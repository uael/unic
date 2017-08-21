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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pmain_test

#include "plib.h"

#include <stdlib.h>
#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static int_t alloc_counter   = 0;
static int_t realloc_counter = 0;
static int_t free_counter    = 0;

extern "C" ptr_t pmem_alloc (size_t nbytes)
{
	++alloc_counter;
	return (ptr_t) malloc (nbytes);
}

extern "C" ptr_t pmem_realloc (ptr_t block, size_t nbytes)
{
	++realloc_counter;
	return (ptr_t) realloc (block, nbytes);
}

extern "C" void pmem_free (ptr_t block)
{
	++free_counter;
	free (block);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pmain_general_test)
{
	p_libsys_init ();
	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pmain_double_test)
{
	p_libsys_init_full (NULL);
	p_libsys_init ();
	p_libsys_shutdown ();
	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pmain_vtable_test)
{
	PMemVTable	vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	p_libsys_init_full (&vtable);

	alloc_counter   = 0;
	realloc_counter = 0;
	free_counter    = 0;

	byte_t *buf = (byte_t *) p_malloc0 (10);
	byte_t *new_buf = (byte_t *) p_realloc ((ptr_t) buf, 20);

	BOOST_REQUIRE (new_buf != NULL);

	buf = new_buf;

	p_free (buf);

	BOOST_CHECK (alloc_counter > 0);
	BOOST_CHECK (realloc_counter > 0);
	BOOST_CHECK (free_counter > 0);

	BOOST_CHECK (strcmp (p_libsys_version (), PLIBSYS_VERSION_STR) == 0);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
