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

#include <time.h>
#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

static void *
shm_test_thread(void *arg) {
  uint_t i;
  int rand_num;
  size_t shm_size;
  ptr_t addr;
  shm_t *shm;

  if (arg == NULL) {
    u_thread_exit(1);
  }
  shm = (shm_t *) arg;
  rand_num = rand() % 127;
  shm_size = u_shm_get_size(shm);
  addr = u_shm_get_address(shm);
  if (shm_size == 0 || addr == NULL) {
    u_thread_exit(1);
  }
  if (!u_shm_lock(shm, NULL)) {
    u_thread_exit(1);
  }
  for (i = 0; i < shm_size; ++i) {
    *(((byte_t *) addr) + i) = (byte_t) rand_num;
  }
  if (!u_shm_unlock(shm, NULL)) {
    u_thread_exit(1);
  }
  u_thread_exit(0);
  return NULL;
}

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

CUTEST(shm, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(
    u_shm_new("u_shm_test_memory_block", 1024, U_SHM_ACCESS_READWRITE, NULL)
      == NULL
  );
  u_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(shm, invalid) {
  shm_t *shm;

  ASSERT(u_shm_new(NULL, 0, U_SHM_ACCESS_READWRITE, NULL) == NULL);
  ASSERT(u_shm_lock(NULL, NULL) == false);
  ASSERT(u_shm_unlock(NULL, NULL) == false);
  ASSERT(u_shm_get_address(NULL) == NULL);
  ASSERT(u_shm_get_size(NULL) == 0);
  u_shm_take_ownership(NULL);
  shm = u_shm_new("u_shm_invalid_test", 0, U_SHM_ACCESS_READWRITE, NULL);
  u_shm_take_ownership(shm);
  u_shm_free(shm);
  shm = u_shm_new("u_shm_invalid_test", 10, (shm_access_t) -1, NULL);
  u_shm_take_ownership(shm);
  u_shm_free(shm);
  return CUTE_SUCCESS;
}

CUTEST(shm, general) {
  shm_t *shm;
#ifndef U_OS_HPUX
  shm_t *shm2;
#endif
  ptr_t addr, addr2;
  int i;

  shm = u_shm_new(
    "u_shm_test_memory_block", 1024, U_SHM_ACCESS_READWRITE, NULL
  );
  ASSERT(shm != NULL);
  u_shm_take_ownership(shm);
  u_shm_free(shm);
  shm =
    u_shm_new("u_shm_test_memory_block", 1024, U_SHM_ACCESS_READWRITE, NULL);
  ASSERT(shm != NULL);
  ASSERT(u_shm_get_size(shm) == 1024);
  addr = u_shm_get_address(shm);
  ASSERT(addr != NULL);
#ifndef U_OS_HPUX
  shm2 =
    u_shm_new("u_shm_test_memory_block", 1024, U_SHM_ACCESS_READONLY, NULL);
  if (shm2 == NULL) {
    /* OK, some systems may want exactly the same permissions */
    shm2 =
      u_shm_new("u_shm_test_memory_block", 1024, U_SHM_ACCESS_READWRITE, NULL);
  }
  ASSERT(shm2 != NULL);
  ASSERT(u_shm_get_size(shm2) == 1024);
  addr2 = u_shm_get_address(shm2);
  ASSERT(shm2 != NULL);
#endif
  for (i = 0; i < 512; ++i) {
    ASSERT(u_shm_lock(shm, NULL));
    *(((byte_t *) addr) + i) = 'a';
    ASSERT(u_shm_unlock(shm, NULL));
  }
#ifndef U_OS_HPUX
  for (i = 0; i < 512; ++i) {
    ASSERT(u_shm_lock(shm2, NULL));
    ASSERT(*(((byte_t *) addr) + i) == 'a');
    ASSERT(u_shm_unlock(shm2, NULL));
  }
#else
  for (i = 0; i < 512; ++i) {
    ASSERT(u_shm_lock (shm, NULL));
    ASSERT(*(((byte_t *) addr) + i) == 'a');
    ASSERT(u_shm_unlock (shm, NULL));
  }
#endif
  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock(shm, NULL));
    *(((byte_t *) addr) + i) = 'b';
    ASSERT(u_shm_unlock(shm, NULL));
  }
#ifndef U_OS_HPUX
  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock(shm2, NULL));
    ASSERT(*(((byte_t *) addr) + i) != 'c');
    ASSERT(u_shm_unlock(shm2, NULL));
  }
  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock(shm2, NULL));
    ASSERT(*(((byte_t *) addr) + i) == 'b');
    ASSERT(u_shm_unlock(shm2, NULL));
  }
#else
  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock (shm, NULL));
    ASSERT(*(((byte_t *) addr) + i) != 'c');
    ASSERT(u_shm_unlock (shm, NULL));
  }

  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock (shm, NULL));
    ASSERT(*(((byte_t *) addr) + i) == 'b');
    ASSERT(u_shm_unlock (shm, NULL));
  }
#endif
  u_shm_free(shm);
  shm =
    u_shm_new("u_shm_test_memory_block_2", 1024, U_SHM_ACCESS_READWRITE, NULL);
  ASSERT(shm != NULL);
  ASSERT(u_shm_get_size(shm) == 1024);
  addr = u_shm_get_address(shm);
  ASSERT(addr != NULL);
  for (i = 0; i < 1024; ++i) {
    ASSERT(u_shm_lock(shm, NULL));
    ASSERT(*(((byte_t *) addr) + i) != 'b');
    ASSERT(u_shm_unlock(shm, NULL));
  }
  u_shm_free(shm);
#ifndef U_OS_HPUX
  u_shm_free(shm2);
#endif
  return CUTE_SUCCESS;
}

CUTEST(shm, thread) {
  shm_t *shm;
  thread_t *thr1, *thr2, *thr3;
  ptr_t addr;
  int i, val;
  bool test_ok;

  srand((uint_t) time(NULL));
  shm =
    u_shm_new(
      "u_shm_test_memory_block", 1024 * 1024, U_SHM_ACCESS_READWRITE,
      NULL);
  ASSERT(shm != NULL);
  u_shm_take_ownership(shm);
  u_shm_free(shm);
  shm =
    u_shm_new(
      "u_shm_test_memory_block", 1024 * 1024, U_SHM_ACCESS_READWRITE,
      NULL);
  ASSERT(shm != NULL);
  if (u_shm_get_size(shm) != 1024 * 1024) {
    u_shm_free(shm);
    shm =
      u_shm_new(
        "u_shm_test_memory_block", 1024 * 1024, U_SHM_ACCESS_READWRITE,
        NULL);
    ASSERT(shm != NULL);
  }
  ASSERT(u_shm_get_size(shm) == 1024 * 1024);
  addr = u_shm_get_address(shm);
  ASSERT(addr != NULL);
  thr1 = u_thread_create((thread_fn_t) shm_test_thread, (ptr_t) shm, true);
  ASSERT(thr1 != NULL);
  thr2 = u_thread_create((thread_fn_t) shm_test_thread, (ptr_t) shm, true);
  ASSERT(thr2 != NULL);
  thr3 = u_thread_create((thread_fn_t) shm_test_thread, (ptr_t) shm, true);
  ASSERT(thr3 != NULL);
  ASSERT(u_thread_join(thr1) == 0);
  ASSERT(u_thread_join(thr2) == 0);
  ASSERT(u_thread_join(thr3) == 0);
  test_ok = true;
  val = *((byte_t *) addr);
  for (i = 1; i < 1024 * 1024; ++i) {
    if (*(((byte_t *) addr) + i) != val) {
      test_ok = false;
      break;
    }
  }
  ASSERT(test_ok == true);
  u_thread_unref(thr1);
  u_thread_unref(thr2);
  u_thread_unref(thr3);
  u_shm_free(shm);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(shm, nomem);
  CUTEST_PASS(shm, invalid);
  CUTEST_PASS(shm, general);
  CUTEST_PASS(shm, thread);
  return EXIT_SUCCESS;
}
