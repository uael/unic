/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

#define PDIR_ENTRY_DIR    "test_2"
#define PDIR_ENTRY_FILE    "test_file.txt"
#define PDIR_TEST_DIR    "." U_DIR_SEP "pdir_test_dir"
#define PDIR_TEST_DIR_IN  "." U_DIR_SEP "pdir_test_dir" U_DIR_SEP "test_2"
#define PDIR_TEST_FILE    "." U_DIR_SEP "pdir_test_dir" U_DIR_SEP "test_file.txt"

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

CUTEST(dir, nomem) {
  dir_t *dir;
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(u_mem_set_vtable(&vtable) == true);

  /* Cleanup previous run */
  u_dir_remove(PDIR_TEST_DIR_IN, NULL);
  u_dir_remove(PDIR_TEST_DIR, NULL);

  ASSERT(u_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(u_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);

  ASSERT(u_dir_new(PDIR_TEST_DIR "/", NULL) == NULL);

  /* Revert memory management back */
  u_mem_restore_vtable();

  /* Try out of memory when iterating */
  dir = u_dir_new(PDIR_TEST_DIR"/", NULL);
  ASSERT(dir != NULL);

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(u_mem_set_vtable(&vtable) == true);

  ASSERT(u_dir_get_next_entry(dir, NULL) == NULL);

  /* Cleanup */
  u_mem_restore_vtable();

  u_dir_free(dir);

  ASSERT(u_dir_remove(PDIR_TEST_DIR_IN, NULL) == true);
  ASSERT(u_dir_remove(PDIR_TEST_DIR, NULL) == true);

  return CUTE_SUCCESS;
}

CUTEST(dir, general) {
  FILE *file;
  byte_t *orig_path;
  dir_t *dir;
  int dir_count;
  int file_count;
  dirent_t *entry;
  int dir_count_2;
  int file_count_2;
  bool has_entry_dir;
  bool has_entry_file;

  ASSERT(u_dir_new(NULL, NULL) == NULL);
  ASSERT(u_dir_new("."
    U_DIR_SEP
    "pdir_test_dir_new", NULL) == NULL);
  ASSERT(u_dir_create(NULL, -1, NULL) == false);
#ifndef U_OS_VMS
  ASSERT(u_dir_create("."
    U_DIR_SEP
    "pdir_test_dir_new"
    U_DIR_SEP
    "test_dir", -1, NULL) == false);
#endif
  ASSERT(u_dir_remove(NULL, NULL) == false);
  ASSERT(u_dir_remove("."
    U_DIR_SEP
    "pdir_test_dir_new", NULL) == false);
  ASSERT(u_dir_is_exists(NULL) == false);
  ASSERT(u_dir_is_exists("."
    U_DIR_SEP
    "pdir_test_dir_new") == false);
  ASSERT(u_dir_get_path(NULL) == NULL);
  ASSERT(u_dir_get_next_entry(NULL, NULL) == NULL);
  ASSERT(u_dir_rewind(NULL, NULL) == false);

  u_dir_entry_free(NULL);
  u_dir_free(NULL);

  /* Cleanup previous run */
  u_dir_remove(PDIR_TEST_DIR_IN, NULL);
  u_dir_remove(PDIR_TEST_DIR, NULL);

  ASSERT(u_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(u_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(u_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);
  ASSERT(u_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);
  file = fopen(PDIR_TEST_FILE, "w");
  ASSERT(file != NULL);
  ASSERT(u_file_is_exists(PDIR_TEST_FILE) == true);

  fprintf(file, "This is a test file string\n");

  ASSERT(fclose(file) == 0);

  ASSERT(u_dir_is_exists(PDIR_TEST_DIR) == true);
  ASSERT(u_dir_is_exists(PDIR_TEST_DIR_IN) == true);

  dir = u_dir_new(PDIR_TEST_DIR"/", NULL);
  ASSERT(dir != NULL);

  dir_count = 0;
  file_count = 0;
  has_entry_dir = false;
  has_entry_file = false;

  while ((entry = u_dir_get_next_entry(dir, NULL)) != NULL) {
    ASSERT(entry->name != NULL);

    switch (entry->type) {
      case U_DIRENT_DIR:
        ++dir_count;
        break;
      case U_DIRENT_FILE:
        ++file_count;
        break;
      case U_DIRENT_OTHER:
      default:
        break;
    }

    if (strcmp(entry->name, PDIR_ENTRY_DIR) == 0) {
      has_entry_dir = true;
    } else if (strcmp(entry->name, PDIR_ENTRY_FILE) == 0) {
      has_entry_file = true;
    }

    u_dir_entry_free(entry);
  }

  ASSERT(dir_count > 0 && dir_count < 4);
  ASSERT(file_count == 1);
  ASSERT(has_entry_dir == true);
  ASSERT(has_entry_file == true);

  ASSERT(u_dir_rewind(dir, NULL) == true);

  dir_count_2 = 0;
  file_count_2 = 0;
  has_entry_dir = false;
  has_entry_file = false;

  while ((entry = u_dir_get_next_entry(dir, NULL)) != NULL) {
    ASSERT(entry->name != NULL);

    switch (entry->type) {
      case U_DIRENT_DIR:
        ++dir_count_2;
        break;
      case U_DIRENT_FILE:
        ++file_count_2;
        break;
      case U_DIRENT_OTHER:
      default:
        break;
    }

    if (strcmp(entry->name, PDIR_ENTRY_DIR) == 0) {
      has_entry_dir = true;
    } else if (strcmp(entry->name, PDIR_ENTRY_FILE) == 0) {
      has_entry_file = true;
    }

    u_dir_entry_free(entry);
  }

  ASSERT(dir_count_2 > 0 && dir_count_2 < 4);
  ASSERT(file_count_2 == 1);
  ASSERT(has_entry_dir == true);
  ASSERT(has_entry_file == true);

  /* Compare two previous attempts */
  ASSERT(dir_count == dir_count_2);
  ASSERT(file_count == file_count_2);

  /* Remove all stuff */
  ASSERT(u_file_remove(PDIR_TEST_FILE, NULL) == true);
  ASSERT(u_dir_remove(PDIR_TEST_DIR, NULL) == false);
  ASSERT(u_dir_remove(PDIR_TEST_DIR_IN, NULL) == true);
  ASSERT(u_dir_remove(PDIR_TEST_DIR, NULL) == true);

  ASSERT(u_dir_is_exists(PDIR_TEST_DIR_IN) == false);
  ASSERT(u_dir_is_exists(PDIR_TEST_DIR) == false);
  orig_path = u_dir_get_path(dir);
  ASSERT(strcmp(orig_path, PDIR_TEST_DIR "/") == 0);
  u_free(orig_path);

  u_dir_free(dir);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(dir, nomem);
  CUTEST_PASS(dir, general);
  return EXIT_SUCCESS;
}
