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

#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

static int thread_wakes_1 = 0;
static int thread_wakes_2 = 0;
static int thread_to_wakes = 0;
static volatile bool is_threads_working = false;
static U_HANDLE thread1_id = (U_HANDLE) NULL;
static U_HANDLE thread2_id = (U_HANDLE) NULL;
static thread_t *thread1_obj = NULL;
static thread_t *thread2_obj = NULL;
static thread_key_t *tls_key = NULL;
static thread_key_t *tls_key_2 = NULL;
static volatile int free_counter = 0;

ptr_t
pmem_alloc(size_t nbytes) {
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  U_UNUSED (block);
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  U_UNUSED (block);
}

void
free_with_check(ptr_t mem) {
  u_free(mem);
  u_atomic_int_inc(&free_counter);
}

static void *
test_thread_func(void *data) {
  int *counter;

  counter = (int *) (data);
  if ((*counter) == 1) {
    thread1_id = u_thread_current_id();
    thread1_obj = u_thread_current();
  } else {
    thread2_id = u_thread_current_id();
    thread2_obj = u_thread_current();
  }
  u_thread_set_local(tls_key, (ptr_t) u_thread_current_id());
  *counter = 0;
  while (is_threads_working == true) {
    u_thread_sleep(5);
    ++(*counter);
    u_thread_yield();
    if (u_thread_get_local(tls_key) != (ptr_t) u_thread_current_id()) {
      u_thread_exit(-1);
    }
  }
  u_thread_exit(*counter);
  return NULL;
}

static void *
test_thread_nonjoinable_func(void *data) {
  int *counter;
  int i;

  counter = (int *) (data);
  is_threads_working = true;
  for (i = thread_to_wakes; i > 0; --i) {
    u_thread_sleep(5);
    ++(*counter);
    u_thread_yield();
  }
  is_threads_working = false;
  u_thread_exit(0);
  return NULL;
}

static void *
test_thread_tls_func(void *data) {
  int self_thread_free;
  int *tls_value;
  int prev_tls;
  int counter;
  int *last_tls;
  int *tls_new_value;

  self_thread_free = *((int *) data);
  tls_value = (int *) u_malloc0(sizeof(int));
  *tls_value = 0;
  u_thread_set_local(tls_key, (ptr_t) tls_value);
  prev_tls = 0;
  counter = 0;
  while (is_threads_working == true) {
    u_thread_sleep(5);
    last_tls = (int *) u_thread_get_local(tls_key);
    if ((*last_tls) != prev_tls) {
      u_thread_exit(-1);
    }
    tls_new_value = (int *) u_malloc0(sizeof(int));
    *tls_new_value = (*last_tls) + 1;
    prev_tls = (*last_tls) + 1;
    u_thread_replace_local(tls_key, (ptr_t) tls_new_value);
    if (self_thread_free) {
      u_free(last_tls);
    }
    ++counter;
    u_thread_yield();
  }
  if (self_thread_free) {
    last_tls = (int *) u_thread_get_local(tls_key);
    if ((*last_tls) != prev_tls) {
      u_thread_exit(-1);
    }
    u_free(last_tls);
    u_thread_replace_local(tls_key, (ptr_t) NULL);
  }
  u_thread_exit(counter);
  return NULL;
}

static void *
test_thread_tls_create_func(void *data) {
  int *tls_value;
  int *tls_value_2;

  tls_value = (int *) u_malloc0(sizeof(int));
  *tls_value = 0;
  u_thread_set_local(tls_key, (ptr_t) tls_value);
  tls_value_2 = (int *) u_malloc0(sizeof(int));
  *tls_value_2 = 0;
  u_thread_set_local(tls_key_2, (ptr_t) tls_value_2);
  return NULL;
}

CUTEST(thread, nomem) {
  thread_key_t *thread_key;
  mem_vtable_t vtable;
  ptr_t tls_value;

  thread_key = u_thread_local_new(u_free);
  ASSERT(thread_key != NULL);
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  thread_wakes_1 = 0;
  thread_wakes_2 = 0;
  ASSERT(u_thread_create((thread_fn_t) test_thread_func,
    (ptr_t) &thread_wakes_1,
    true
  ) == NULL);
  ASSERT(u_thread_create_full((thread_fn_t) test_thread_func,
    (ptr_t) &thread_wakes_2,
    true,
    U_thread_PRIORITY_NORMAL,
    0
  ) == NULL);
  ASSERT(u_thread_current() == NULL);
  ASSERT(u_thread_local_new(NULL) == NULL);
  u_thread_exit(0);
  u_thread_set_local(thread_key, PINT_TO_POINTER (10));
  tls_value = u_thread_get_local(thread_key);
  if (tls_value != NULL) {
    ASSERT(tls_value == PINT_TO_POINTER(10));
    u_thread_set_local(thread_key, NULL);
  }
  u_thread_replace_local(thread_key, PINT_TO_POINTER (12));
  tls_value = u_thread_get_local(thread_key);
  if (tls_value != NULL) {
    ASSERT(tls_value == PINT_TO_POINTER(12));
    u_thread_set_local(thread_key, NULL);
  }
  u_mem_restore_vtable();
  u_thread_local_free(thread_key);
  return CUTE_SUCCESS;
}

CUTEST(thread, bad_input) {
  ASSERT(u_thread_create(NULL, NULL, false) == NULL);
  ASSERT(u_thread_create_full(NULL, NULL, false, U_thread_PRIORITY_NORMAL, 0)
    == NULL);
  ASSERT(u_thread_join(NULL) == -1);
  ASSERT(u_thread_set_priority(NULL, U_thread_PRIORITY_NORMAL) == false);
  ASSERT(u_thread_get_local(NULL) == NULL);
  u_thread_set_local(NULL, NULL);
  u_thread_replace_local(NULL, NULL);
  u_thread_ref(NULL);
  u_thread_unref(NULL);
  u_thread_local_free(NULL);
  u_thread_exit(0);
  return CUTE_SUCCESS;
}

CUTEST(thread, general) {
  thread_t *thr1;
  thread_t *thr2;
  thread_t *cur_thr;

  thread_wakes_1 = 1;
  thread_wakes_2 = 2;
  thread1_id = (U_HANDLE) NULL;
  thread2_id = (U_HANDLE) NULL;
  thread1_obj = NULL;
  thread2_obj = NULL;
  tls_key = u_thread_local_new(NULL);
  ASSERT(tls_key != NULL);
  is_threads_working = true;
  thr1 = u_thread_create((thread_fn_t) test_thread_func,
    (ptr_t) &thread_wakes_1,
    true
  );
  thr2 = u_thread_create_full((thread_fn_t) test_thread_func,
    (ptr_t) &thread_wakes_2,
    true,
    U_thread_PRIORITY_NORMAL,
    64 * 1024
  );
  u_thread_ref(thr1);
  u_thread_set_priority(thr1, U_thread_PRIORITY_NORMAL);
  ASSERT(thr1 != NULL);
  ASSERT(thr2 != NULL);
  u_thread_sleep(5);
  is_threads_working = false;
  ASSERT(u_thread_join(thr1) == thread_wakes_1);
  ASSERT(u_thread_join(thr2) == thread_wakes_2);
  ASSERT(thread1_id != thread2_id);
  ASSERT(thread1_id != u_thread_current_id() &&
    thread2_id != u_thread_current_id());
  ASSERT(thread1_obj == thr1);
  ASSERT(thread2_obj == thr2);
  u_thread_local_free(tls_key);
  u_thread_unref(thr1);
  u_thread_unref(thr2);
  u_thread_unref(thr1);
  cur_thr = u_thread_current();
  ASSERT(cur_thr != NULL);
  ASSERT(u_thread_ideal_count() > 0);
  return CUTE_SUCCESS;
}

CUTEST(thread, nonjoinable) {
  thread_t *thr1;

  thread_wakes_1 = 0;
  thread_to_wakes = 100;
  is_threads_working = true;
  thr1 = u_thread_create((thread_fn_t) test_thread_nonjoinable_func,
    (ptr_t) &thread_wakes_1,
    false
  );
  ASSERT(thr1 != NULL);
  u_thread_sleep(2);
  ASSERT(u_thread_join(thr1) == -1);
  while (is_threads_working == true) {
    u_thread_sleep(5);
  }
  ASSERT(thread_wakes_1 == thread_to_wakes);
  u_thread_unref(thr1);
  return CUTE_SUCCESS;
}

CUTEST(thread, tls) {
  thread_t *thr2;
  thread_t *thr1;
  int total_counter;
  int self_thread_free;

  /* With destroy notification */
  tls_key = u_thread_local_new(free_with_check);
  is_threads_working = true;
  free_counter = 0;
  self_thread_free = 0;
  thr1 = u_thread_create((thread_fn_t) test_thread_tls_func,
    (ptr_t) &self_thread_free,
    true
  );
  thr2 = u_thread_create((thread_fn_t) test_thread_tls_func,
    (ptr_t) &self_thread_free,
    true
  );
  ASSERT(thr1 != NULL);
  ASSERT(thr2 != NULL);
  u_thread_sleep(50);
  is_threads_working = false;
  total_counter = 0;
  total_counter += (u_thread_join(thr1) + 1);
  total_counter += (u_thread_join(thr2) + 1);
  ASSERT(total_counter == free_counter);
  u_thread_local_free(tls_key);
  u_thread_unref(thr1);
  u_thread_unref(thr2);

  /* Without destroy notification */
  tls_key = u_thread_local_new(NULL);
  free_counter = 0;
  is_threads_working = true;
  self_thread_free = 1;
  thr1 = u_thread_create((thread_fn_t) test_thread_tls_func,
    (ptr_t) &self_thread_free,
    true
  );
  thr2 = u_thread_create((thread_fn_t) test_thread_tls_func,
    (ptr_t) &self_thread_free,
    true
  );
  ASSERT(thr1 != NULL);
  ASSERT(thr2 != NULL);
  u_thread_sleep(50);
  is_threads_working = false;
  total_counter = 0;
  total_counter += (u_thread_join(thr1) + 1);
  total_counter += (u_thread_join(thr2) + 1);
  ASSERT(total_counter > 0);
  ASSERT(free_counter == 0);
  u_thread_local_free(tls_key);
  u_thread_unref(thr1);
  u_thread_unref(thr2);

  /* With implicit thread exit */
  tls_key = u_thread_local_new(free_with_check);
  tls_key_2 = u_thread_local_new(free_with_check);
  free_counter = 0;
  thr1 = u_thread_create((thread_fn_t) test_thread_tls_create_func,
    NULL,
    true
  );
  thr2 = u_thread_create((thread_fn_t) test_thread_tls_create_func,
    NULL,
    true
  );
  ASSERT(thr1 != NULL);
  ASSERT(thr2 != NULL);
  u_thread_join(thr1);
  u_thread_join(thr2);
  ASSERT(free_counter == 4);
  u_thread_local_free(tls_key);
  u_thread_local_free(tls_key_2);
  u_thread_unref(thr1);
  u_thread_unref(thr2);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(thread, nomem);
  CUTEST_PASS(thread, bad_input);
  CUTEST_PASS(thread, general);
  CUTEST_PASS(thread, nonjoinable);
  CUTEST_PASS(thread, tls);
  return EXIT_SUCCESS;
}
