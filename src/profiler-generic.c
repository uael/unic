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

#include "p/profiler.h"
#include "profiler-private.h"

u64_t
p_profiler_get_ticks_internal() {
  i64_t val;
  if (P_UNLIKELY ((val = (i64_t) time(NULL)) == -1)) {
    P_ERROR (
      "profiler_t::p_profiler_get_ticks_internal: time() failed");
    return 0;
  }
  return (u64_t) (val * 1000000);
}

u64_t
p_profiler_elapsed_usecs_internal(const profiler_t *profiler) {
  return p_profiler_get_ticks_internal() - profiler->counter;
}

void
p_profiler_init(void) {
}

void
p_profiler_shutdown(void) {
}
