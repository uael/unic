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

#define BOOST_TEST_MODULE pcondvariable_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PCONDTEST_MAX_QUEUE 10

static int_t              thread_wakeups   = 0;
static int_t              thread_queue     = 0;
static p_condvar_t *   queue_empty_cond = NULL;
static p_condvar_t *   queue_full_cond  = NULL;
static PMutex *          cond_mutex       = NULL;
volatile static bool is_working       = true;

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

static void * producer_test_thread (void *)
{
	while (is_working == true) {
		if (!p_mutex_lock (cond_mutex)) {
			is_working = false;
			p_condvar_broadcast (queue_full_cond);
			p_uthread_exit (1);
		}

		while (thread_queue >= PCONDTEST_MAX_QUEUE && is_working == true) {
			if (!p_condvar_wait (queue_empty_cond, cond_mutex)) {
				is_working = false;
				p_condvar_broadcast (queue_full_cond);
				p_mutex_unlock (cond_mutex);
				p_uthread_exit (1);
			}
		}

		if (is_working) {
			++thread_queue;
			++thread_wakeups;
		}

		if (!p_condvar_broadcast (queue_full_cond)) {
			is_working = false;
			p_mutex_unlock (cond_mutex);
			p_uthread_exit (1);
		}

		if (!p_mutex_unlock (cond_mutex)) {
			is_working = false;
			p_condvar_broadcast (queue_full_cond);
			p_uthread_exit (1);
		}
	}

	p_condvar_broadcast (queue_full_cond);
	p_uthread_exit (0);

	return NULL;
}

static void * consumer_test_thread (void *)
{
	while (is_working == true) {
		if (!p_mutex_lock (cond_mutex)) {
			is_working = false;
			p_condvar_signal (queue_empty_cond);
			p_uthread_exit (1);
		}

		while (thread_queue <= 0 && is_working == true) {
			if (!p_condvar_wait (queue_full_cond, cond_mutex)) {
				is_working = false;
				p_condvar_signal (queue_empty_cond);
				p_mutex_unlock (cond_mutex);
				p_uthread_exit (1);
			}
		}

		if (is_working) {
			--thread_queue;
			++thread_wakeups;
		}

		if (!p_condvar_signal (queue_empty_cond)) {
			is_working = false;
			p_mutex_unlock (cond_mutex);
			p_uthread_exit (1);
		}

		if (!p_mutex_unlock (cond_mutex)) {
			is_working = false;
			p_condvar_signal (queue_empty_cond);
			p_uthread_exit (1);
		}
	}

	p_condvar_signal (queue_empty_cond);
	p_uthread_exit (0);

	return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pcondvariable_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);
	BOOST_CHECK (p_condvar_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pcondvariable_bad_input_test)
{
	p_libsys_init ();

	BOOST_REQUIRE (p_condvar_broadcast (NULL) == false);
	BOOST_REQUIRE (p_condvar_signal (NULL) == false);
	BOOST_REQUIRE (p_condvar_wait (NULL, NULL) == false);
	p_condvar_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pcondvariable_general_test)
{
	PUThread *thr1, *thr2, *thr3;

	p_libsys_init ();

	queue_empty_cond = p_condvar_new ();
	BOOST_REQUIRE (queue_empty_cond != NULL);
	queue_full_cond = p_condvar_new ();
	BOOST_REQUIRE (queue_full_cond != NULL);
	cond_mutex = p_mutex_new ();
	BOOST_REQUIRE (cond_mutex != NULL);

	is_working     = true;
	thread_wakeups = 0;
	thread_queue   = 0;

	thr1 = p_uthread_create ((PUThreadFunc) producer_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	thr3 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, true);
	BOOST_REQUIRE (thr3 != NULL);

	BOOST_REQUIRE (p_condvar_broadcast (queue_empty_cond) == true);
	BOOST_REQUIRE (p_condvar_broadcast (queue_full_cond) == true);

	p_uthread_sleep (4000);

	is_working = false;

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);
	BOOST_CHECK (p_uthread_join (thr3) == 0);

	BOOST_REQUIRE (thread_wakeups > 0 && thread_queue >= 0 && thread_queue <= 10);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_uthread_unref (thr3);
	p_condvar_free (queue_empty_cond);
	p_condvar_free (queue_full_cond);
	p_mutex_free (cond_mutex);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
