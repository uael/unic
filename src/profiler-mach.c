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

#include <mach/mach_time.h>

#include "unic/profiler.h"
#include "profiler-private.h"

static u64_t pp_profiler_freq_num = 0;
static u64_t pp_profiler_freq_denom = 0;

u64_t
u_profiler_get_ticks_internal() {
  u64_t val = mach_absolute_time();

  /* To prevent overflow */
  val /= 1000;
  val *= pp_profiler_freq_num;
  val /= pp_profiler_freq_denom;
  return val;
}

u64_t
u_profiler_elapsed_usecs_internal(const profiler_t *profiler) {
  return u_profiler_get_ticks_internal() - profiler->counter;
}

void
u_profiler_init(void) {
  mach_timebase_info_data_t tb;
  if (U_UNLIKELY (mach_timebase_info(&tb) != KERN_SUCCESS || tb.denom == 0)) {
    U_ERROR (
      "profiler_t::u_profiler_init: mach_timebase_info() failed");
    return;
  }
  pp_profiler_freq_num = (u64_t) tb.numer;
  pp_profiler_freq_denom = (u64_t) tb.denom;
}

void
u_profiler_shutdown(void) {
  pp_profiler_freq_num = 0;
  pp_profiler_freq_denom = 0;
}
