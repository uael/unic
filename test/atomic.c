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

/* Actually we couldn't test the work of the atomic operations across the
 * threads, but at least we can test the sanity of operations */

CUTEST(atomic, general) {
  ptr_t atomic_pointer;
  int atomic_int;
  int i;

  (void) u_atomic_is_lock_free();
  atomic_int = 0;
  u_atomic_int_set(&atomic_int, 10);
  ASSERT(u_atomic_int_add(&atomic_int, 5) == 10);
  ASSERT(u_atomic_int_get(&atomic_int) == 15);
  u_atomic_int_add(&atomic_int, -5);
  ASSERT(u_atomic_int_get(&atomic_int) == 10);
  u_atomic_int_inc(&atomic_int);
  ASSERT(u_atomic_int_get(&atomic_int) == 11);
  ASSERT(u_atomic_int_dec_and_test(&atomic_int) == false);
  ASSERT(u_atomic_int_get(&atomic_int) == 10);
  ASSERT(u_atomic_int_compare_and_exchange(&atomic_int, 10, -10) == true);
  ASSERT(u_atomic_int_get(&atomic_int) == -10);
  ASSERT(u_atomic_int_compare_and_exchange(&atomic_int, 10, 20) == false);
  ASSERT(u_atomic_int_get(&atomic_int) == -10);
  u_atomic_int_inc(&atomic_int);
  ASSERT(u_atomic_int_get(&atomic_int) == -9);
  u_atomic_int_set(&atomic_int, 4);
  ASSERT(u_atomic_int_get(&atomic_int) == 4);
  ASSERT(u_atomic_int_xor((uint_t *) &atomic_int, (uint_t) 1) == 4);
  ASSERT(u_atomic_int_get(&atomic_int) == 5);
  ASSERT(u_atomic_int_or((uint_t *) &atomic_int, (uint_t) 2) == 5);
  ASSERT(u_atomic_int_get(&atomic_int) == 7);
  ASSERT(u_atomic_int_and((uint_t *) &atomic_int, (uint_t) 1) == 7);
  ASSERT(u_atomic_int_get(&atomic_int) == 1);
  u_atomic_int_set(&atomic_int, 51);
  ASSERT(u_atomic_int_get(&atomic_int) == 51);
  for (i = 51; i > 1; --i) {
    ASSERT(u_atomic_int_dec_and_test(&atomic_int) == false);
    ASSERT(u_atomic_int_get(&atomic_int) == (i - 1));
  }
  ASSERT(u_atomic_int_dec_and_test(&atomic_int) == true);
  ASSERT(u_atomic_int_get(&atomic_int) == 0);
  atomic_pointer = NULL;
  u_atomic_pointer_set(&atomic_pointer, PUINT_TO_POINTER (U_MAXSIZE));
  ASSERT(
    u_atomic_pointer_get(&atomic_pointer) == PUINT_TO_POINTER(U_MAXSIZE)
  );
  u_atomic_pointer_set(&atomic_pointer, PUINT_TO_POINTER (100));
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PUINT_TO_POINTER(100));
  ASSERT(u_atomic_pointer_add(&atomic_pointer, (ssize_t) 100) == 100);
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PUINT_TO_POINTER(200));
  u_atomic_pointer_set(&atomic_pointer, PINT_TO_POINTER (4));
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PINT_TO_POINTER(4));
  ASSERT(u_atomic_pointer_xor(&atomic_pointer, (size_t) 1) == 4);
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PINT_TO_POINTER(5));
  ASSERT(u_atomic_pointer_or(&atomic_pointer, (size_t) 2) == 5);
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PINT_TO_POINTER(7));
  ASSERT(u_atomic_pointer_and(&atomic_pointer, (size_t) 1) == 7);
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == PINT_TO_POINTER(1));
  ASSERT(
    u_atomic_pointer_compare_and_exchange(
      &atomic_pointer, PUINT_TO_POINTER(1),
      NULL
    ) == true
  );
  ASSERT(u_atomic_pointer_get(&atomic_pointer) == NULL);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(atomic, general);
  return EXIT_SUCCESS;
}
