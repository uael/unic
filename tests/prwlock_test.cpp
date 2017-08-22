/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#define BOOST_TEST_MODULE prwlock_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PRWLOCK_TEST_STRING_1 "This is a test string."
#define PRWLOCK_TEST_STRING_2 "Ouh, yet another string to check!"

static rwlock_t *         test_rwlock        = NULL;
static volatile bool is_threads_working = false;
static volatile int_t     writers_counter    = 0;
static byte_t             string_buf[50];

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

static void * reader_thread_func (void *data)
{
	P_UNUSED (data);

	int_t counter = 0;

	while (p_atomic_int_get (&writers_counter) == 0)
		p_uthread_sleep (10);

	while (is_threads_working == true) {
		p_uthread_sleep (10);

		if (p_rwlock_reader_trylock (test_rwlock) == false) {
			if (p_rwlock_reader_lock (test_rwlock) == false)
				p_uthread_exit (-1);
		}

		if (strcmp (string_buf, PRWLOCK_TEST_STRING_1) != 0 &&
		    strcmp (string_buf, PRWLOCK_TEST_STRING_2) != 0) {
			p_rwlock_reader_unlock (test_rwlock);
			p_uthread_exit (-1);
		}

		if (p_rwlock_reader_unlock (test_rwlock) == false)
			p_uthread_exit (-1);

		++counter;
	}

	p_uthread_exit (counter);

	return NULL;
}

static void * writer_thread_func (void *data)
{
	int_t string_num = PPOINTER_TO_INT (data);
	int_t counter    = 0;

	while (is_threads_working == true) {
		p_uthread_sleep (10);

		if (p_rwlock_writer_trylock (test_rwlock) == false) {
			if (p_rwlock_writer_lock (test_rwlock) == false)
				p_uthread_exit (-1);
		}

		memset (string_buf, 0, sizeof (string_buf));

		if (string_num == 1)
			strcpy (string_buf, PRWLOCK_TEST_STRING_1);
		else
			strcpy (string_buf, PRWLOCK_TEST_STRING_1);

		if (p_rwlock_writer_unlock (test_rwlock) == false)
			p_uthread_exit (-1);

		++counter;

		p_atomic_int_inc ((&writers_counter));
	}

	p_uthread_exit (counter);

	return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (prwlock_nomem_test)
{
	p_libsys_init ();

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_rwlock_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (prwlock_bad_input_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_rwlock_reader_lock (NULL) == false);
	BOOST_CHECK (p_rwlock_reader_trylock (NULL) == false);
	BOOST_CHECK (p_rwlock_reader_unlock (NULL) == false);
	BOOST_CHECK (p_rwlock_writer_lock (NULL) == false);
	BOOST_CHECK (p_rwlock_writer_trylock (NULL) == false);
	BOOST_CHECK (p_rwlock_writer_unlock (NULL) == false);
	p_rwlock_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (prwlock_general_test)
{
	p_libsys_init ();

	test_rwlock = p_rwlock_new ();

	BOOST_REQUIRE (test_rwlock != NULL);

	is_threads_working = true;
	writers_counter    = 0;

	uthread_t *reader_thr1 = p_uthread_create ((uthread_fn_t) reader_thread_func,
						  NULL,
						  true);

	uthread_t *reader_thr2 = p_uthread_create ((uthread_fn_t) reader_thread_func,
						  NULL,
						  true);

	uthread_t *writer_thr1 = p_uthread_create ((uthread_fn_t) writer_thread_func,
						  NULL,
						  true);

	uthread_t *writer_thr2 = p_uthread_create ((uthread_fn_t) writer_thread_func,
						  NULL,
						  true);

	BOOST_REQUIRE (reader_thr1 != NULL);
	BOOST_REQUIRE (reader_thr2 != NULL);
	BOOST_REQUIRE (writer_thr1 != NULL);
	BOOST_REQUIRE (writer_thr2 != NULL);

	p_uthread_sleep (10000);

	is_threads_working = false;

	BOOST_CHECK (p_uthread_join (reader_thr1) > 0);
	BOOST_CHECK (p_uthread_join (reader_thr2) > 0);
	BOOST_CHECK (p_uthread_join (writer_thr1) > 0);
	BOOST_CHECK (p_uthread_join (writer_thr2) > 0);

	p_uthread_unref (reader_thr1);
	p_uthread_unref (reader_thr2);
	p_uthread_unref (writer_thr1);
	p_uthread_unref (writer_thr2);

	p_rwlock_free (test_rwlock);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
