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

CUTEST(string, nomem) {
  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(p_mem_set_vtable(&vtable) == true);
  ASSERT(p_strdup("test string") == NULL);
  p_mem_restore_vtable();
  return CUTE_SUCCESS;
}

CUTEST(string, strdup) {
  const byte_t *test_str_1;
  byte_t *new_string;

  test_str_1 = "Test string";
  new_string = p_strdup(test_str_1);
  ASSERT(new_string != NULL);
  p_free(new_string);
  return CUTE_SUCCESS;
}

CUTEST(string, strchomp) {
  const byte_t *test_chomp_str_orig;
  const byte_t *test_chomp_str_1;
  const byte_t *test_chomp_str_2;
  const byte_t *test_chomp_str_3;
  const byte_t *test_chomp_str_4;
  const byte_t *test_chomp_str_5;
  const byte_t *test_chomp_str_6;
  const byte_t *test_chomp_str_7;
  const byte_t *test_chomp_str_8;
  const byte_t *test_chomp_str_9;
  const byte_t *test_chomp_str_10;
  const byte_t *test_chomp_str_11;
  byte_t *new_string;

  test_chomp_str_orig = "Test chomp string";
  test_chomp_str_1 = "Test chomp string  ";
  test_chomp_str_2 = "\n\nTest chomp string  ";
  test_chomp_str_3 = "\n\rTest chomp string  \n";
  test_chomp_str_4 = "Test chomp string\n\n";
  test_chomp_str_5 = "  \rTest chomp string \n\n  ";
  test_chomp_str_6 = "  \rI\n\n  ";
  test_chomp_str_7 = "\n";
  test_chomp_str_8 = "I";
  test_chomp_str_9 = "";
  test_chomp_str_10 = " ";
  test_chomp_str_11 = NULL;
  new_string = p_strchomp(test_chomp_str_1);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp(test_chomp_str_orig, new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_2);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp(test_chomp_str_orig, new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_3);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp(test_chomp_str_orig, new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_4);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp(test_chomp_str_orig, new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_5);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp(test_chomp_str_orig, new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_6);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp("I", new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_7);
  ASSERT(new_string != NULL);
  ASSERT(*new_string == '\0');
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_8);
  ASSERT(new_string != NULL);
  ASSERT_EQ(strcmp("I", new_string), 0);
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_9);
  ASSERT(new_string != NULL);
  ASSERT(*new_string == '\0');
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_10);
  ASSERT(new_string != NULL);
  ASSERT(*new_string == '\0');
  p_free(new_string);
  new_string = p_strchomp(test_chomp_str_11);
  ASSERT(new_string == NULL);
  return CUTE_SUCCESS;
}

CUTEST(string, strtok) {
  byte_t *test_string;
  byte_t *token, *next_token;
  byte_t *test_string_2;
  byte_t *test_string_3;
  byte_t *test_string_4;

  ASSERT(p_strtok(NULL, NULL, NULL) == NULL);

  /* First string */
  test_string = p_strdup("1,2,3");

  /* Check third parameter for possible NULL */
  token = p_strtok(test_string, ",", NULL);
  if (strcmp(token, "1") != 0) {
    token = p_strtok(test_string, ",", &next_token);
    ASSERT(token != NULL);
    ASSERT(strcmp(token, "1") == 0);
  }
  token = p_strtok(NULL, ",", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "2") == 0);
  token = p_strtok(NULL, ",", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "3") == 0);
  token = p_strtok(NULL, ",", &next_token);
  ASSERT(token == NULL);

  /* Second string */
  test_string_2 = p_strdup("Test string, to test");
  token = p_strtok(test_string_2, " ", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "Test") == 0);
  token = p_strtok(NULL, ", ", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "string") == 0);
  token = p_strtok(NULL, ", ", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "to") == 0);
  token = p_strtok(NULL, ", \t\n", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "test") == 0);
  token = p_strtok(NULL, ", \t\n", &next_token);
  ASSERT(token == NULL);

  /* Third string */
  test_string_3 = p_strdup("compile\ttest\ndeploy");
  token = p_strtok(test_string_3, "\t\n", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "compile") == 0);
  token = p_strtok(NULL, "\t\n", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "test") == 0);
  token = p_strtok(NULL, "\t\n", &next_token);
  ASSERT(token != NULL);
  ASSERT(strcmp(token, "deploy") == 0);
  token = p_strtok(NULL, ", \t\n", &next_token);
  ASSERT(token == NULL);

  /* Fourth string */
  test_string_4 = p_strdup("\t  \t\n  \t");
  token = p_strtok(test_string_4, "\t\n ", &next_token);
  ASSERT(token == NULL);

  /* free strs */
  p_free(test_string);
  p_free(test_string_2);
  p_free(test_string_3);
  p_free(test_string_4);
  return CUTE_SUCCESS;
}

CUTEST(string, strtod) {

  /* Incorrect input */
  ASSERT_CLOSE(p_strtod(NULL), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("e2"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("e-2"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("-e2"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("-e-2"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("0,3"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("12,3"), 12.0, 0.0001);

  /* Correct input */
  ASSERT_CLOSE(p_strtod("0"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("0.0"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("-0"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("-0.0"), 0.0, 0.0001);
  ASSERT_CLOSE(p_strtod("3.14"), 3.14, 0.0001);
  ASSERT_CLOSE(p_strtod("+3.14"), 3.14, 0.0001);
  ASSERT_CLOSE(p_strtod("-12.256"), -12.256, 0.0001);
  ASSERT_CLOSE(p_strtod("0.056"), 0.056, 0.0001);
  ASSERT_CLOSE(p_strtod("-0.057"), -0.057, 0.0001);
  ASSERT_CLOSE(p_strtod("1.5423e2"), 154.23, 0.0001);
  ASSERT_CLOSE(p_strtod("1e3"), 1000.0, 0.0001);
  ASSERT_CLOSE(p_strtod("1e+3"), 1000.0, 0.0001);
  ASSERT_CLOSE(p_strtod("-2.56e1"), -25.6, 0.0001);
  ASSERT_CLOSE(p_strtod("-2.56e+1"), -25.6, 0.0001);
  ASSERT_CLOSE(p_strtod("123e-2"), 1.23, 0.0001);
  ASSERT_CLOSE(p_strtod("3.14e-1"), 0.314, 0.0001);
  ASSERT_CLOSE(p_strtod("3.14e-60"), 3.14e-60, 0.0001);
  ASSERT_CLOSE(p_strtod("2.14e10"), 2.14e10, 0.0001);
  ASSERT_CLOSE(p_strtod("2.14e-10"), 2.14e-10, 0.0001);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(string, nomem);
  CUTEST_PASS(string, strdup);
  CUTEST_PASS(string, strchomp);
  CUTEST_PASS(string, strtok);
  CUTEST_PASS(string, strtod);
  return EXIT_SUCCESS;
}
