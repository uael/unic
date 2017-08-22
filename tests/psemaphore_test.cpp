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

#define BOOST_TEST_MODULE psemaphore_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PSEMAPHORE_MAX_VAL 10

static int_t semaphore_test_val = 0;
static int_t is_thread_exit     = 0;

static void clean_error (err_t **error)
{
	if (error == NULL || *error == NULL)
		return;

	p_error_free (*error);
	*error = NULL;
}

static void * semaphore_test_thread (void *)
{
	sem_t	*sem;
	int_t		i;

	sem = p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_OPEN, NULL);

	if (sem == NULL)
		p_uthread_exit (1);

	for (i = 0; i < 1000; ++i) {
		if (!p_semaphore_acquire (sem, NULL)) {
			if (is_thread_exit > 0) {
				semaphore_test_val = PSEMAPHORE_MAX_VAL;
				break;
			}

			p_uthread_exit (1);
		}

		if (semaphore_test_val == PSEMAPHORE_MAX_VAL)
			--semaphore_test_val;
		else {
			p_uthread_sleep (1);
			++semaphore_test_val;
		}

		if (!p_semaphore_release (sem, NULL)) {
			if (is_thread_exit > 0) {
				semaphore_test_val = PSEMAPHORE_MAX_VAL;
				break;
			}

			p_uthread_exit (1);
		}
	}

	++is_thread_exit;

	p_semaphore_free (sem);
	p_uthread_exit (0);

	return NULL;
}

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

BOOST_AUTO_TEST_CASE (psemaphore_nomem_test)
{
	p_libsys_init ();

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_CREATE, NULL) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (psemaphore_general_test)
{
	sem_t	*sem = NULL;
	err_t		*error = NULL;
	int_t		i;

	p_libsys_init ();

	BOOST_CHECK (p_semaphore_new (NULL, 0, P_SEM_ACCESS_CREATE, &error) == NULL);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	BOOST_REQUIRE (p_semaphore_acquire (sem, &error) == false);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	BOOST_REQUIRE (p_semaphore_release (sem, &error) == false);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	p_semaphore_take_ownership (sem);
	p_semaphore_free (NULL);

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_acquire (sem, NULL));

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_release (sem, NULL));

	for (i = 0; i < 1000; ++i) {
		BOOST_CHECK (p_semaphore_acquire (sem, NULL));
		BOOST_CHECK (p_semaphore_release (sem, NULL));
	}

	p_semaphore_free (sem);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (psemaphore_thread_test)
{
	uthread_t	*thr1, *thr2;
	sem_t	*sem = NULL;

	p_libsys_init ();

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	sem                = NULL;
	is_thread_exit     = 0;
	semaphore_test_val = PSEMAPHORE_MAX_VAL;

	thr1 = p_uthread_create ((uthread_fn_t) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((uthread_fn_t) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	BOOST_REQUIRE (semaphore_test_val == PSEMAPHORE_MAX_VAL);

	BOOST_REQUIRE (p_semaphore_acquire (sem, NULL) == false);
	BOOST_REQUIRE (p_semaphore_release (sem, NULL) == false);
	p_semaphore_free (sem);
	p_semaphore_take_ownership (sem);

	sem = p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_OPEN, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
