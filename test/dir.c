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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PDIR_ENTRY_DIR    "test_2"
#define PDIR_ENTRY_FILE    "test_file.txt"
#define PDIR_TEST_DIR    "." P_DIR_SEP "pdir_test_dir"
#define PDIR_TEST_DIR_IN  "." P_DIR_SEP "pdir_test_dir" P_DIR_SEP "test_2"
#define PDIR_TEST_FILE    "." P_DIR_SEP "pdir_test_dir" P_DIR_SEP "test_file.txt"

ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED (block);
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  P_UNUSED (block);
}

CUTEST(dir, nomem) {

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  /* Cleanup previous run */
  p_dir_remove(PDIR_TEST_DIR_IN, NULL);
  p_dir_remove(PDIR_TEST_DIR, NULL);

  ASSERT(p_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(p_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);

  ASSERT(p_dir_new(PDIR_TEST_DIR
    "/", NULL) == NULL);

  /* Revert memory management back */
  p_mem_restore_vtable();

  /* Try out of memory when iterating */
  dir_t *dir = p_dir_new(PDIR_TEST_DIR"/", NULL);
  ASSERT(dir != NULL);

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(p_dir_get_next_entry(dir, NULL) == NULL);

  /* Cleanup */
  p_mem_restore_vtable();

  p_dir_free(dir);

  ASSERT(p_dir_remove(PDIR_TEST_DIR_IN, NULL) == true);
  ASSERT(p_dir_remove(PDIR_TEST_DIR, NULL) == true);

  return CUTE_SUCCESS;
}

CUTEST(dir, general) {

  ASSERT(p_dir_new(NULL, NULL) == NULL);
  ASSERT(p_dir_new("."
    P_DIR_SEP
    "pdir_test_dir_new", NULL) == NULL);
  ASSERT(p_dir_create(NULL, -1, NULL) == false);
#ifndef P_OS_VMS
  ASSERT(p_dir_create("."
    P_DIR_SEP
    "pdir_test_dir_new"
    P_DIR_SEP
    "test_dir", -1, NULL) == false);
#endif
  ASSERT(p_dir_remove(NULL, NULL) == false);
  ASSERT(p_dir_remove("."
    P_DIR_SEP
    "pdir_test_dir_new", NULL) == false);
  ASSERT(p_dir_is_exists(NULL) == false);
  ASSERT(p_dir_is_exists("."
    P_DIR_SEP
    "pdir_test_dir_new") == false);
  ASSERT(p_dir_get_path(NULL) == NULL);
  ASSERT(p_dir_get_next_entry(NULL, NULL) == NULL);
  ASSERT(p_dir_rewind(NULL, NULL) == false);

  p_dir_entry_free(NULL);
  p_dir_free(NULL);

  /* Cleanup previous run */
  p_dir_remove(PDIR_TEST_DIR_IN, NULL);
  p_dir_remove(PDIR_TEST_DIR, NULL);

  ASSERT(p_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(p_dir_create(PDIR_TEST_DIR, 0777, NULL) == true);
  ASSERT(p_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);
  ASSERT(p_dir_create(PDIR_TEST_DIR_IN, 0777, NULL) == true);

  FILE *file = fopen(PDIR_TEST_FILE, "w");
  ASSERT(file != NULL);
  ASSERT(p_file_is_exists(PDIR_TEST_FILE) == true);

  fprintf(file, "This is a test file string\n");

  ASSERT(fclose(file) == 0);

  ASSERT(p_dir_is_exists(PDIR_TEST_DIR) == true);
  ASSERT(p_dir_is_exists(PDIR_TEST_DIR_IN) == true);

  dir_t *dir = p_dir_new(PDIR_TEST_DIR"/", NULL);

  ASSERT(dir != NULL);

  int dir_count = 0;
  int file_count = 0;
  bool has_entry_dir = false;
  bool has_entry_file = false;

  dirent_t *entry;

  while ((entry = p_dir_get_next_entry(dir, NULL)) != NULL) {
    ASSERT(entry->name != NULL);

    switch (entry->type) {
      case P_DIRENT_DIR:
        ++dir_count;
        break;
      case P_DIRENT_FILE:
        ++file_count;
        break;
      case P_DIRENT_OTHER:
      default:
        break;
    }

    if (strcmp(entry->name, PDIR_ENTRY_DIR) == 0) {
      has_entry_dir = true;
    } else if (strcmp(entry->name, PDIR_ENTRY_FILE) == 0) {
      has_entry_file = true;
    }

    p_dir_entry_free(entry);
  }

  ASSERT(dir_count > 0 && dir_count < 4);
  ASSERT(file_count == 1);
  ASSERT(has_entry_dir == true);
  ASSERT(has_entry_file == true);

  ASSERT(p_dir_rewind(dir, NULL) == true);

  int dir_count_2 = 0;
  int file_count_2 = 0;
  has_entry_dir = false;
  has_entry_file = false;

  while ((entry = p_dir_get_next_entry(dir, NULL)) != NULL) {
    ASSERT(entry->name != NULL);

    switch (entry->type) {
      case P_DIRENT_DIR:
        ++dir_count_2;
        break;
      case P_DIRENT_FILE:
        ++file_count_2;
        break;
      case P_DIRENT_OTHER:
      default:
        break;
    }

    if (strcmp(entry->name, PDIR_ENTRY_DIR) == 0) {
      has_entry_dir = true;
    } else if (strcmp(entry->name, PDIR_ENTRY_FILE) == 0) {
      has_entry_file = true;
    }

    p_dir_entry_free(entry);
  }

  ASSERT(dir_count_2 > 0 && dir_count_2 < 4);
  ASSERT(file_count_2 == 1);
  ASSERT(has_entry_dir == true);
  ASSERT(has_entry_file == true);

  /* Compare two previous attempts */
  ASSERT(dir_count == dir_count_2);
  ASSERT(file_count == file_count_2);

  /* Remove all stuff */
  ASSERT(p_file_remove(PDIR_TEST_FILE, NULL) == true);
  ASSERT(p_dir_remove(PDIR_TEST_DIR, NULL) == false);
  ASSERT(p_dir_remove(PDIR_TEST_DIR_IN, NULL) == true);
  ASSERT(p_dir_remove(PDIR_TEST_DIR, NULL) == true);

  ASSERT(p_dir_is_exists(PDIR_TEST_DIR_IN) == false);
  ASSERT(p_dir_is_exists(PDIR_TEST_DIR) == false);

  byte_t *orig_path = p_dir_get_path(dir);
  ASSERT(strcmp(orig_path, PDIR_TEST_DIR
    "/") == 0);
  p_free(orig_path);

  p_dir_free(dir);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(dir, nomem);
  CUTEST_PASS(dir, general);
  return EXIT_SUCCESS;
}
