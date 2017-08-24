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

#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

static byte_t test_str[] = "This is a test string!";

static int is_thread_exit = 0;

static int read_count = 0;

static int write_count = 0;

#ifndef U_OS_HPUX

volatile static bool is_working = false;

static void *
shmbuf_test_write_thread(void) {
  shmbuf_t *buffer;
  ssize_t op_result;

  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  if (buffer == NULL) {
    u_uthread_exit(1);
  }
  while (is_working == true) {
    u_uthread_sleep(3);
    op_result = u_shmbuf_get_free_space(buffer, NULL);

    if (op_result < 0) {
      if (is_thread_exit > 0) {
        break;
      } else {
        ++is_thread_exit;
        u_shmbuf_free(buffer);
        u_uthread_exit(1);
      }
    }

    if ((size_t) op_result < sizeof(test_str)) {
      continue;
    }

    op_result =
      u_shmbuf_write(buffer, (ptr_t) test_str, sizeof(test_str), NULL);

    if (op_result < 0) {
      if (is_thread_exit > 0) {
        break;
      } else {
        ++is_thread_exit;
        u_shmbuf_free(buffer);
        u_uthread_exit(1);
      }
    }

    if (op_result != sizeof(test_str)) {
      ++is_thread_exit;
      u_shmbuf_free(buffer);
      u_uthread_exit(1);
    }

    ++read_count;
  }

  ++is_thread_exit;

  u_shmbuf_free(buffer);
  u_uthread_exit(0);

  return NULL;
}

static void *
shmbuf_test_read_thread(void) {
  ssize_t op_result;
  shmbuf_t *buffer;
  byte_t test_buf[sizeof(test_str)];

  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  if (buffer == NULL) {
    u_uthread_exit(1);
  }

  while (is_working == true) {
    u_uthread_sleep(3);
    op_result = u_shmbuf_get_used_space(buffer, NULL);

    if (op_result < 0) {
      if (is_thread_exit > 0) {
        break;
      } else {
        ++is_thread_exit;
        u_shmbuf_free(buffer);
        u_uthread_exit(1);
      }
    }

    if ((size_t) op_result < sizeof(test_str)) {
      continue;
    }

    op_result =
      u_shmbuf_read(buffer, (ptr_t) test_buf, sizeof(test_buf), NULL);

    if (op_result < 0) {
      if (is_thread_exit > 0) {
        break;
      } else {
        ++is_thread_exit;
        u_shmbuf_free(buffer);
        u_uthread_exit(1);
      }
    }

    if (op_result != sizeof(test_buf)) {
      ++is_thread_exit;
      u_shmbuf_free(buffer);
      u_uthread_exit(1);
    }

    if (strncmp(test_buf, test_str, sizeof(test_buf)) != 0) {
      ++is_thread_exit;
      u_shmbuf_free(buffer);
      u_uthread_exit(1);
    }

    ++write_count;
  }

  ++is_thread_exit;

  u_shmbuf_free(buffer);
  u_uthread_exit(0);

  return NULL;
}

#endif /* !U_OS_HPUX */

ptr_t
pmem_alloc(size_t nbytes) {
  U_UNUSED(nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  U_UNUSED(block);
  U_UNUSED(nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  U_UNUSED(block);
}

CUTEST(shmbuf, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(u_mem_set_vtable(&vtable) == true);

  ASSERT(u_shmbuf_new("shm_test_buffer", 1024, NULL) == NULL);

  u_mem_restore_vtable();

  return CUTE_SUCCESS;
}

CUTEST(shmbuf, bad_input) {
  shmbuf_t *buf;

  ASSERT(u_shmbuf_new(NULL, 0, NULL) == NULL);
  ASSERT(u_shmbuf_read(NULL, NULL, 0, NULL) == -1);
  ASSERT(u_shmbuf_write(NULL, NULL, 0, NULL) == -1);
  ASSERT(u_shmbuf_get_free_space(NULL, NULL) == -1);
  ASSERT(u_shmbuf_get_used_space(NULL, NULL) == -1);
  buf = u_shmbuf_new("shm_invalid_buffer", 0, NULL);
  u_shmbuf_take_ownership(buf);
  u_shmbuf_free(buf);
  u_shmbuf_clear(NULL);
  u_shmbuf_free(NULL);

  return CUTE_SUCCESS;
}

CUTEST(shmbuf, general) {
  byte_t test_buf[sizeof(test_str)];
  byte_t *large_buf;
  shmbuf_t *buffer;

  /* Buffer may be from the previous test on UNIX systems */
  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  ASSERT(buffer != NULL);
  u_shmbuf_take_ownership(buffer);
  u_shmbuf_free(buffer);
  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  ASSERT(buffer != NULL);

  ASSERT(u_shmbuf_get_free_space(buffer, NULL) == 1024);
  ASSERT(u_shmbuf_get_used_space(buffer, NULL) == 0);
  u_shmbuf_clear(buffer);
  ASSERT(u_shmbuf_get_free_space(buffer, NULL) == 1024);
  ASSERT(u_shmbuf_get_used_space(buffer, NULL) == 0);

  memset(test_buf, 0, sizeof(test_buf));

  ASSERT(
    u_shmbuf_write(buffer, (ptr_t) test_str, sizeof(test_str), NULL)
      == sizeof(test_str));
  ASSERT(
    u_shmbuf_get_free_space(buffer, NULL) == (1024 - sizeof(test_str)));
  ASSERT(u_shmbuf_get_used_space(buffer, NULL) == sizeof(test_str));
  ASSERT(
    u_shmbuf_read(buffer, (ptr_t) test_buf, sizeof(test_buf), NULL)
      == sizeof(test_str));
  ASSERT(
    u_shmbuf_read(buffer, (ptr_t) test_buf, sizeof(test_buf), NULL) == 0);

  ASSERT(strncmp(test_buf, test_str, sizeof(test_str)) == 0);
  ASSERT(u_shmbuf_get_free_space(buffer, NULL) == 1024);
  ASSERT(u_shmbuf_get_used_space(buffer, NULL) == 0);

  u_shmbuf_clear(buffer);

  large_buf = (byte_t *) u_malloc0(2048);
  ASSERT(large_buf != NULL);
  ASSERT(u_shmbuf_write(buffer, (ptr_t) large_buf, 2048, NULL) == 0);

  u_free(large_buf);
  u_shmbuf_free(buffer);

  return CUTE_SUCCESS;
}

#ifndef U_OS_HPUX

CUTEST(shmbuf, thread) {
  shmbuf_t *buffer;
  uthread_t *thr1, *thr2;

  /* Buffer may be from the previous test on UNIX systems */
  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  ASSERT(buffer != NULL);
  u_shmbuf_take_ownership(buffer);
  u_shmbuf_free(buffer);

  is_thread_exit = 0;
  read_count = 0;
  write_count = 0;
  is_working = true;

  buffer = u_shmbuf_new("shm_test_buffer", 1024, NULL);
  ASSERT(buffer != NULL);

  thr1 =
    u_uthread_create((uthread_fn_t) shmbuf_test_write_thread, NULL, true);
  ASSERT(thr1 != NULL);

  thr2 =
    u_uthread_create((uthread_fn_t) shmbuf_test_read_thread, NULL, true);
  ASSERT(thr1 != NULL);

  u_uthread_sleep(50);

  is_working = false;

  ASSERT(u_uthread_join(thr1) == 0);
  ASSERT(u_uthread_join(thr2) == 0);

  ASSERT(read_count > 0);
  ASSERT(write_count > 0);

  u_shmbuf_free(buffer);
  u_uthread_unref(thr1);
  u_uthread_unref(thr2);

  return CUTE_SUCCESS;
}

#endif /* !U_OS_HPUX */

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(shmbuf, nomem);
  CUTEST_PASS(shmbuf, bad_input);
  CUTEST_PASS(shmbuf, general);
  CUTEST_PASS(shmbuf, thread);
  return EXIT_SUCCESS;
}
