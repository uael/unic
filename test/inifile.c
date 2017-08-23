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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

#define PINIFILE_STRESS_LINE  2048
#define PINIFILE_MAX_LINE  1024

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

static bool
create_test_ini_file(bool last_empty_section) {
  FILE *file = fopen("." P_DIR_SEP "p_ini_test_file.ini", "w");

  if (file == NULL) {
    return false;
  }

  byte_t *buf = (byte_t *) p_malloc0(PINIFILE_STRESS_LINE + 1);

  for (int i = 0; i < PINIFILE_STRESS_LINE; ++i) {
    buf[i] = (byte_t) (97 + i % 20);
  }

  /* Empty section */
  fprintf(file, "[empty_section]\n");

  /* Numeric section */
  fprintf(file, "[numeric_section]\n");
  fprintf(file, "int_parameter_1 = 4\n");
  fprintf(file, "int_parameter_2 = 5 ;This is a comment\n");
  fprintf(file, "int_parameter_3 = 6 #This is another type of a comment\n");
  fprintf(file, "# Whole line is a comment\n");
  fprintf(file, "; Yet another comment line\n");
  fprintf(file, "float_parameter_1 = 3.24\n");
  fprintf(file, "float_parameter_2 = 0.15\n");

  /* String section */
  fprintf(file, "[string_section]\n");
  fprintf(file, "string_parameter_1 = Test string\n");
  fprintf(file, "string_parameter_2 = \"Test string with #'\"\n");
  fprintf(file, "string_parameter_3 = \n");
  fprintf(file, "string_parameter_4 = 12345 ;Comment\n");
  fprintf(file, "string_parameter_4 = 54321\n");
  fprintf(file, "string_parameter_5 = 'Test string'\n");
  fprintf(file, "string_parameter_6 = %s\n", buf);
  fprintf(file, "string_parameter_7 = ''\n");
  fprintf(file, "string_parameter_8 = \"\"\n");
  fprintf(file, "%s = stress line\n", buf);

  /* Boolean section */
  fprintf(file, "[boolean_section]\n");
  fprintf(file, "boolean_parameter_1 = true ;True value\n");
  fprintf(file, "boolean_parameter_2 = 0 ;False value\n");
  fprintf(file, "boolean_parameter_3 = false ;False value\n");
  fprintf(file, "boolean_parameter_4 = 1 ;True value\n");

  /* List section */
  fprintf(file, "[list_section]\n");
  fprintf(file, "list_parameter_1 = {1\t2\t5\t10} ;First list\n");
  fprintf(file, "list_parameter_2 = {2.0 3.0 5.0} #Second list\n");
  fprintf(file, "list_parameter_3 = {true false 1} #Last list\n");

  /* Empty section */
  if (last_empty_section) {
    fprintf(file, "[empty_section_2]\n");
  }

  p_free(buf);

  return fclose(file) == 0;
}

CUTEST(inifile, nomem) {

  ASSERT(create_test_ini_file(false));

  ini_file_t *ini = p_inifile_new("." P_DIR_SEP "p_ini_test_file.ini");
  ASSERT(ini != NULL);

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  ASSERT(p_mem_set_vtable(&vtable) == true);

  ASSERT(p_inifile_new("."
    P_DIR_SEP
    "p_ini_test_file.ini") == NULL);
  ASSERT(p_inifile_parse(ini, NULL) == true);
  ASSERT(p_inifile_sections(ini) == NULL);

  p_mem_restore_vtable();

  p_inifile_free(ini);

  ini = p_inifile_new("." P_DIR_SEP "p_ini_test_file.ini");
  ASSERT(ini != NULL);

  ASSERT(p_inifile_parse(ini, NULL) == true);
  list_t *section_list = p_inifile_sections(ini);
  ASSERT(section_list != NULL);
  ASSERT(p_list_length(section_list) == 4);

  p_list_foreach(section_list, (fn_t) p_free, NULL);
  p_list_free(section_list);
  p_inifile_free(ini);

  ASSERT(p_file_remove("."
    P_DIR_SEP
    "p_ini_test_file.ini", NULL) == true);

  return CUTE_SUCCESS;
}

CUTEST(inifile, bad_input) {
  ini_file_t *ini = NULL;

  p_inifile_free(ini);
  ASSERT(p_inifile_new(NULL) == NULL);
  ASSERT(p_inifile_parse(ini, NULL) == false);
  ASSERT(p_inifile_is_parsed(ini) == false);
  ASSERT(
    p_inifile_is_key_exists(ini, "string_section", "string_paramter_1")
      == false);
  ASSERT(p_inifile_sections(ini) == NULL);
  ASSERT(p_inifile_keys(ini, "string_section") == NULL);
  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_parameter_1",
      false
    ) == false);
  ASSERT (
    p_inifile_parameter_double(
      ini, "numeric_section", "float_parameter_1",
      1.0
    ) == 1.0);
  ASSERT(
    p_inifile_parameter_int(ini, "numeric_section", "int_parameter_1", 0)
      == 0);
  ASSERT(
    p_inifile_parameter_list(ini, "list_section", "list_parameter_1") == NULL);
  ASSERT(
    p_inifile_parameter_string(
      ini, "string_section", "string_parameter_1",
      NULL
    ) == NULL);

  ini = p_inifile_new("./bad_file_path/fake.ini");
  ASSERT(ini != NULL);
  ASSERT(p_inifile_parse(ini, NULL) == false);
  p_inifile_free(ini);

  ASSERT(create_test_ini_file(true));

  return CUTE_SUCCESS;
}

CUTEST(inifile, read) {

  ini_file_t *ini = p_inifile_new("." P_DIR_SEP "p_ini_test_file.ini");
  ASSERT(ini != NULL);
  ASSERT(p_inifile_is_parsed(ini) == false);

  ASSERT(p_inifile_parse(ini, NULL) == true);
  ASSERT(p_inifile_is_parsed(ini) == true);
  ASSERT(p_inifile_parse(ini, NULL) == true);
  ASSERT(p_inifile_is_parsed(ini) == true);

  /* Test list of sections */
  list_t *list = p_inifile_sections(ini);
  ASSERT(list != NULL);
  ASSERT(p_list_length(list) == 4);

  p_list_foreach(list, (fn_t) p_free, NULL);
  p_list_free(list);

  /* Test empty section */
  list = p_inifile_keys(ini, "empty_section");
  ASSERT(list == NULL);

  /* Test numeric section */
  list = p_inifile_keys(ini, "numeric_section");
  ASSERT(p_list_length(list) == 5);
  p_list_foreach(list, (fn_t) p_free, NULL);
  p_list_free(list);

  ASSERT(
    p_inifile_parameter_list(ini, "numeric_section", "int_parameter_1")
      == NULL);
  ASSERT(
    p_inifile_parameter_int(ini, "numeric_section", "int_parameter_1", -1)
      == 4);
  ASSERT(
    p_inifile_parameter_int(ini, "numeric_section", "int_parameter_2", -1)
      == 5);
  ASSERT(
    p_inifile_parameter_int(ini, "numeric_section", "int_parameter_3", -1)
      == 6);
  ASSERT(
    p_inifile_parameter_int(ini, "numeric_section", "int_parameter_def", 10)
      == 10);
  ASSERT(
    p_inifile_parameter_double(
      ini, "numeric_section", "float_parameter_1",
      -1.0
    ) == 3.24
  );
  ASSERT(
    p_inifile_parameter_double(
      ini, "numeric_section", "float_parameter_2",
      -1.0
    ) >= 0.15
  );
  ASSERT (p_inifile_parameter_double(
    ini, "numeric_section_no",
    "float_parameter_def", 10.0
  ) == 10.0);
  ASSERT(
    p_inifile_is_key_exists(ini, "numeric_section", "int_parameter_1")
      == true);
  ASSERT(
    p_inifile_is_key_exists(ini, "numeric_section", "float_parameter_1")
      == true);
  ASSERT(
    p_inifile_is_key_exists(ini, "numeric_section_false", "float_parameter_1")
      == false);

  /* Test string section */
  list = p_inifile_keys(ini, "string_section");
  ASSERT(p_list_length(list) == 8);
  p_list_foreach(list, (fn_t) p_free, NULL);
  p_list_free(list);

  byte_t *str =
    p_inifile_parameter_string(
      ini, "string_section", "string_parameter_1",
      NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "Test string") == 0);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_2",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "Test string with #'") == 0);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_3",
    NULL);
  ASSERT(str == NULL);
  ASSERT(
    p_inifile_is_key_exists(ini, "string_section", "string_parameter_3")
      == false);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_4",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "54321") == 0);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_5",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "Test string") == 0);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_6",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strlen(str) > 0 && strlen(str) < PINIFILE_MAX_LINE);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_7",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "") == 0);
  p_free(str);

  str = p_inifile_parameter_string(
    ini, "string_section", "string_parameter_8",
    NULL);
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "") == 0);
  p_free(str);

  str =
    p_inifile_parameter_string(
      ini, "string_section", "string_parameter_def",
      "default_value"
    );
  ASSERT(str != NULL);
  ASSERT(strcmp(str, "default_value") == 0);
  p_free(str);

  /* Test boolean section */
  list = p_inifile_keys(ini, "boolean_section");
  ASSERT(p_list_length(list) == 4);
  p_list_foreach(list, (fn_t) p_free, NULL);
  p_list_free(list);

  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_parameter_1",
      false
    ) == true);
  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_parameter_2",
      true
    ) == false);
  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_parameter_3",
      true
    ) == false);
  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_parameter_4",
      false
    ) == true);
  ASSERT(
    p_inifile_parameter_boolean(
      ini, "boolean_section", "boolean_section_def",
      true
    ) == true);

  /* Test list section */
  list = p_inifile_keys(ini, "list_section");
  ASSERT(p_list_length(list) == 3);
  p_list_foreach(list, (fn_t) p_free, NULL);
  p_list_free(list);

  /* -- First list parameter */
  list_t *list_val =
    p_inifile_parameter_list(ini, "list_section", "list_parameter_1");
  ASSERT(list_val != NULL);
  ASSERT(p_list_length(list_val) == 4);

  int int_sum = 0;
  for (list_t *iter = list_val; iter != NULL; iter = iter->next) {
    int_sum += atoi((const byte_t *) (iter->data));
  }

  ASSERT(int_sum == 18);
  p_list_foreach(list_val, (fn_t) p_free, NULL);
  p_list_free(list_val);

  /* -- Second list parameter */
  list_val = p_inifile_parameter_list(ini, "list_section", "list_parameter_2");
  ASSERT(list_val != NULL);
  ASSERT(p_list_length(list_val) == 3);

  double flt_sum = 0;
  for (list_t *iter = list_val; iter != NULL; iter = iter->next) {
    flt_sum += atof((const byte_t *) (iter->data));
  }

  ASSERT(flt_sum == 10.0);
  p_list_foreach(list_val, (fn_t) p_free, NULL);
  p_list_free(list_val);

  /* -- Third list parameter */
  list_val = p_inifile_parameter_list(ini, "list_section", "list_parameter_3");
  ASSERT(list_val != NULL);
  ASSERT(p_list_length(list_val) == 3);

  bool bool_sum = true;
  for (list_t *iter = list_val; iter != NULL; iter = iter->next) {
    bool_sum = bool_sum && atoi((const byte_t *) (iter->data));
  }

  ASSERT(bool_sum == false);
  p_list_foreach(list_val, (fn_t) p_free, NULL);
  p_list_free(list_val);

  /* -- False list parameter */
  ASSERT(
    p_inifile_parameter_list(ini, "list_section_no", "list_parameter_def")
      == NULL);

  p_inifile_free(ini);

  ASSERT(p_file_remove("."
    P_DIR_SEP
    "p_ini_test_file.ini", NULL) == true);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(inifile, nomem);
  CUTEST_PASS(inifile, bad_input);
  CUTEST_PASS(inifile, read);
  return EXIT_SUCCESS;
}
