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

#define PFILE_TEST_FILE "." U_DIR_SEP "u_test_file.txt"

CUTEST(file, general) {
  FILE *file;

  ASSERT(u_file_is_exists(PFILE_TEST_FILE) == false);
  ASSERT(u_file_remove(NULL, NULL) == false);
  ASSERT(u_file_remove("."U_DIR_SEP" test_file_remove.txt", NULL) == false);
  ASSERT(file = fopen(PFILE_TEST_FILE, "w"));
  ASSERT(fprintf(file, "This is a test file string\n"));
  ASSERT(u_file_is_exists(PFILE_TEST_FILE) == true);
  ASSERT(fclose(file) == 0);
  ASSERT(u_file_remove(PFILE_TEST_FILE, NULL) == true);
  ASSERT(u_file_is_exists(PFILE_TEST_FILE) == false);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(file, general);
  return EXIT_SUCCESS;
}
