/*
 * Copyright (C) 2013-2017 Alexander Saprykin <xelfium@gmail.com>
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

#define BOOST_TEST_MODULE puthread_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static int_t              thread_wakes_1     = 0;
static int_t              thread_wakes_2     = 0;
static int_t              thread_to_wakes    = 0;
static volatile bool is_threads_working = false;

static P_HANDLE   thread1_id  = (P_HANDLE) NULL;
static P_HANDLE   thread2_id  = (P_HANDLE) NULL;
static PUThread * thread1_obj = NULL;
static PUThread * thread2_obj = NULL;

static PUThreadKey * tls_key      = NULL;
static PUThreadKey * tls_key_2    = NULL;
static volatile int_t free_counter = 0;

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

extern "C" void free_with_check (ptr_t mem)
{
	p_free (mem);
	p_atomic_int_inc (&free_counter);
}

static void * test_thread_func (void *data)
{
	int_t *counter = static_cast < int_t * > (data);

	if ((*counter) == 1) {
		thread1_id  = p_uthread_current_id ();
		thread1_obj = p_uthread_current ();
	} else {
		thread2_id  = p_uthread_current_id ();
		thread2_obj = p_uthread_current ();
	}

	p_uthread_set_local (tls_key, (ptr_t) p_uthread_current_id ());

	*counter = 0;

	while (is_threads_working == true) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();

		if (p_uthread_get_local (tls_key) != (ptr_t) p_uthread_current_id ())
			p_uthread_exit (-1);
	}

	p_uthread_exit (*counter);

	return NULL;
}

static void * test_thread_nonjoinable_func (void *data)
{
	int_t *counter = static_cast < int_t * > (data);

	is_threads_working = true;

	for (int i = thread_to_wakes; i > 0; --i) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();
	}

	is_threads_working = false;

	p_uthread_exit (0);

	return NULL;
}

static void * test_thread_tls_func (void *data)
{
	int_t self_thread_free = *((int_t *) data);

	int_t *tls_value = (int_t *) p_malloc0 (sizeof (int_t));
	*tls_value = 0;
	p_uthread_set_local (tls_key, (ptr_t) tls_value);

	int_t prev_tls = 0;
	int_t counter  = 0;

	while (is_threads_working == true) {
		p_uthread_sleep (10);

		int_t *last_tls = (int_t *) p_uthread_get_local (tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit (-1);

		int_t *tls_new_value = (int_t *) p_malloc0 (sizeof (int_t));

		*tls_new_value = (*last_tls) + 1;
		prev_tls       = (*last_tls) + 1;

		p_uthread_replace_local (tls_key, (ptr_t) tls_new_value);

		if (self_thread_free)
			p_free (last_tls);

		++counter;

		p_uthread_yield ();
	}

	if (self_thread_free) {
		int_t *last_tls = (int_t *) p_uthread_get_local (tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit (-1);

		p_free (last_tls);

		p_uthread_replace_local (tls_key, (ptr_t) NULL);
	}

	p_uthread_exit (counter);

	return NULL;
}

static void * test_thread_tls_create_func (void *data)
{
	int_t *tls_value = (int_t *) p_malloc0 (sizeof (int_t));
	*tls_value = 0;
	p_uthread_set_local (tls_key, (ptr_t) tls_value);

	int_t *tls_value_2 = (int_t *) p_malloc0 (sizeof (int_t));
	*tls_value_2 = 0;
	p_uthread_set_local (tls_key_2, (ptr_t) tls_value_2);

	return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (puthread_nomem_test)
{
	p_libsys_init ();

	PUThreadKey *thread_key = p_uthread_local_new (p_free);
	BOOST_CHECK (thread_key != NULL);

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	thread_wakes_1 = 0;
	thread_wakes_2 = 0;

	BOOST_CHECK (p_uthread_create ((PUThreadFunc) test_thread_func,
				       (ptr_t) &thread_wakes_1,
				       true) == NULL);

	BOOST_CHECK (p_uthread_create_full ((PUThreadFunc) test_thread_func,
					    (ptr_t) &thread_wakes_2,
					    true,
					    P_UTHREAD_PRIORITY_NORMAL,
					    0) == NULL);

	BOOST_CHECK (p_uthread_current () == NULL);
	BOOST_CHECK (p_uthread_local_new (NULL) == NULL);

	p_uthread_exit (0);

	p_uthread_set_local (thread_key, PINT_TO_POINTER (10));

	ptr_t tls_value = p_uthread_get_local (thread_key);

	if (tls_value != NULL) {
		BOOST_CHECK (tls_value == PINT_TO_POINTER (10));
		p_uthread_set_local (thread_key, NULL);
	}

	p_uthread_replace_local (thread_key, PINT_TO_POINTER (12));

	tls_value = p_uthread_get_local (thread_key);

	if (tls_value != NULL) {
		BOOST_CHECK (tls_value == PINT_TO_POINTER (12));
		p_uthread_set_local (thread_key, NULL);
	}

	p_mem_restore_vtable ();

	p_uthread_local_free (thread_key);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_bad_input_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_uthread_create (NULL, NULL, false) == NULL);
	BOOST_CHECK (p_uthread_create_full (NULL, NULL, false, P_UTHREAD_PRIORITY_NORMAL, 0) == NULL);
	BOOST_CHECK (p_uthread_join (NULL) == -1);
	BOOST_CHECK (p_uthread_set_priority (NULL, P_UTHREAD_PRIORITY_NORMAL) == false);
	BOOST_CHECK (p_uthread_get_local (NULL) == NULL);
	p_uthread_set_local (NULL, NULL);
	p_uthread_replace_local (NULL, NULL);
	p_uthread_ref (NULL);
	p_uthread_unref (NULL);
	p_uthread_local_free (NULL);
	p_uthread_exit (0);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_general_test)
{
	p_libsys_init ();

	thread_wakes_1 = 1;
	thread_wakes_2 = 2;
	thread1_id     = (P_HANDLE) NULL;
	thread2_id     = (P_HANDLE) NULL;
	thread1_obj    = NULL;
	thread2_obj    = NULL;

	tls_key = p_uthread_local_new (NULL);
	BOOST_CHECK (tls_key != NULL);

	is_threads_working = true;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_func,
					   (ptr_t) &thread_wakes_1,
					   true);

	PUThread *thr2 = p_uthread_create_full ((PUThreadFunc) test_thread_func,
						(ptr_t) &thread_wakes_2,
						true,
						P_UTHREAD_PRIORITY_NORMAL,
						64 * 1024);

	p_uthread_ref (thr1);

	p_uthread_set_priority (thr1, P_UTHREAD_PRIORITY_NORMAL);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = false;

	BOOST_CHECK (p_uthread_join (thr1) == thread_wakes_1);
	BOOST_CHECK (p_uthread_join (thr2) == thread_wakes_2);

	BOOST_REQUIRE (thread1_id != thread2_id);
	BOOST_CHECK (thread1_id != p_uthread_current_id () &&
		     thread2_id != p_uthread_current_id ());

	BOOST_CHECK (thread1_obj == thr1);
	BOOST_CHECK (thread2_obj == thr2);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_uthread_unref (thr1);

	PUThread *cur_thr = p_uthread_current ();
	BOOST_CHECK (cur_thr != NULL);

	BOOST_CHECK (p_uthread_ideal_count () > 0);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_nonjoinable_test)
{
	p_libsys_init ();

	thread_wakes_1     = 0;
	thread_to_wakes    = 100;
	is_threads_working = true;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_nonjoinable_func,
					   (ptr_t) &thread_wakes_1,
					   false);

	BOOST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (3000);

	BOOST_CHECK (p_uthread_join (thr1) == -1);

	while (is_threads_working == true)
		p_uthread_sleep (10);

	BOOST_CHECK (thread_wakes_1 == thread_to_wakes);

	p_uthread_unref (thr1);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_tls_test)
{
	p_libsys_init ();

	/* With destroy notification */
	tls_key = p_uthread_local_new (free_with_check);

	is_threads_working = true;
	free_counter       = 0;

	int_t self_thread_free = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ptr_t) &self_thread_free,
					   true);

	PUThread *thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ptr_t) &self_thread_free,
					   true);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = false;

	int_t total_counter = 0;

	total_counter += (p_uthread_join (thr1) + 1);
	total_counter += (p_uthread_join (thr2) + 1);

	BOOST_CHECK (total_counter == free_counter);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	/* Without destroy notification */
	tls_key = p_uthread_local_new (NULL);

	free_counter       = 0;
	is_threads_working = true;
	self_thread_free   = 1;

	thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ptr_t) &self_thread_free,
				 true);

	thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ptr_t) &self_thread_free,
				 true);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = false;

	total_counter = 0;

	total_counter += (p_uthread_join (thr1) + 1);
	total_counter += (p_uthread_join (thr2) + 1);

	BOOST_CHECK (total_counter > 0);
	BOOST_CHECK (free_counter == 0);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	/* With implicit thread exit */
	tls_key = p_uthread_local_new (free_with_check);
	tls_key_2 = p_uthread_local_new (free_with_check);

	free_counter = 0;

	thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_create_func,
				 NULL,
				 true);
	thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_create_func,
				 NULL,
				 true);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_join (thr1);
	p_uthread_join (thr2);

	BOOST_CHECK (free_counter == 4);

	p_uthread_local_free (tls_key);
	p_uthread_local_free (tls_key_2);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
