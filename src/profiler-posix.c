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

#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "unic/profiler.h"
#include "profiler-private.h"

#ifndef _POSIX_MONOTONIC_CLOCK
# define _POSIX_MONOTONIC_CLOCK (-1)
#endif

typedef u64_t (*PPOSIXTicksFunc)(void);

static PPOSIXTicksFunc pp_profiler_ticks_func = NULL;

#if (_POSIX_MONOTONIC_CLOCK >= 0) || defined (U_OS_IRIX)
static u64_t pp_profiler_get_ticks_clock();
#endif

static u64_t
pp_profiler_get_ticks_gtod();

#if (_POSIX_MONOTONIC_CLOCK >= 0) || defined (U_OS_IRIX)
static u64_t
pp_profiler_get_ticks_clock() {
  struct timespec ts;

#ifdef U_OS_IRIX
  if (U_UNLIKELY (clock_gettime (CLOCK_SGI_CYCLE, &ts) != 0)) {
#else
  if (U_UNLIKELY (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)) {
#endif
    U_ERROR (
      "profiler_t::pp_profiler_get_ticks_clock: clock_gettime() failed");
    return pp_profiler_get_ticks_gtod();
  } else
    return (u64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}
#endif

static u64_t
pp_profiler_get_ticks_gtod() {
  struct timeval tv;
  if (U_UNLIKELY (gettimeofday(&tv, NULL) != 0)) {
    U_ERROR (
      "profiler_t::pp_profiler_get_ticks_gtod: gettimeofday() failed");
    return 0;
  }
  return (u64_t) (tv.tv_sec * 1000000 + tv.tv_usec);
}

u64_t
u_profiler_get_ticks_internal() {
  return pp_profiler_ticks_func();
}

u64_t
u_profiler_elapsed_usecs_internal(const profiler_t *profiler) {
  return pp_profiler_ticks_func() - profiler->counter;
}

void
u_profiler_init(void) {
#if defined (U_OS_IRIX) || (_POSIX_MONOTONIC_CLOCK > 0)
  pp_profiler_ticks_func = (PPOSIXTicksFunc) pp_profiler_get_ticks_clock;
#elif (_POSIX_MONOTONIC_CLOCK == 0) && defined (_SC_MONOTONIC_CLOCK)
  if (U_LIKELY (sysconf(_SC_MONOTONIC_CLOCK) > 0))
    pp_profiler_ticks_func =
      (PPOSIXTicksFunc) pp_profiler_get_ticks_clock;
  else
    pp_profiler_ticks_func =
      (PPOSIXTicksFunc) pp_profiler_get_ticks_gtod;
#else
  pp_profiler_ticks_func = (PPOSIXTicksFunc) pp_profiler_get_ticks_gtod;
#endif
}

void
u_profiler_shutdown(void) {
  pp_profiler_ticks_func = NULL;
}
