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

CUTEST(process, general) {
  uint32_t pid;

  pid = p_process_get_current_pid();
  ASSERT(pid > 0);
  ASSERT(p_process_is_running(pid) == true);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(process, general);
  return EXIT_SUCCESS;
}
