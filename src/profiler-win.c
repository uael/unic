/*
 * Copyright (C) 2015-2017 Alexander Saprykin <xelfium@gmail.com>
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

/* https://msdn.microsoft.com/ru-ru/library/windows/desktop/dn553408(v=vs.85).aspx */

#include "p/profiler.h"
#include "profiler-private.h"

#if PLIBSYS_HAS_LLDIV
#  include <stdlib.h>
#endif

typedef u64_t (WINAPI
  *PWin32TicksFunc)(void);

typedef u64_t (*PWin32ElapsedFunc)(u64_t last_counter);

static PWin32TicksFunc pp_profiler_ticks_func = NULL;

static PWin32ElapsedFunc pp_profiler_elapsed_func = NULL;

static u64_t pp_profiler_freq = 1;

static u64_t
WINAPIpp_profiler_get_hr_ticks(void);

static u64_t
pp_profiler_elapsed_hr(u64_t last_counter);

static u64_t
pp_profiler_elapsed_tick64(u64_t last_counter);

static u64_t
pp_profiler_elapsed_tick(u64_t last_counter);

static u64_t WINAPI
pp_profiler_get_hr_ticks(void) {
  LARGE_INTEGER tcounter;
  if (P_UNLIKELY (QueryPerformanceCounter(&tcounter) == false)) {
    P_ERROR (
      "profiler_t::pp_profiler_get_hr_ticks: QueryPerformanceCounter() failed");
    tcounter.QuadPart = 0;
  }
  return (u64_t) tcounter.QuadPart;
}

static u64_t
pp_profiler_elapsed_hr(u64_t last_counter) {
  u64_t ticks;
#ifdef PLIBSYS_HAS_LLDIV
  lldiv_t ldres;
#endif
  u64_t quot;
  u64_t rem;
  ticks = pp_profiler_ticks_func() - last_counter;
#ifdef PLIBSYS_HAS_LLDIV
  ldres = lldiv((long long) ticks, (long long) pp_profiler_freq);
  quot = ldres.quot;
  rem = ldres.rem;
#else
  quot = ticks / pp_profiler_freq;
  rem = ticks % pp_profiler_freq;
#endif
  return (u64_t) (quot * 1000000 + (rem * 1000000) / pp_profiler_freq);
}

static u64_t
pp_profiler_elapsed_tick64(u64_t last_counter) {
  return (pp_profiler_ticks_func() - last_counter) * 1000;
}

static u64_t
pp_profiler_elapsed_tick(u64_t last_counter) {
  u64_t val;
  u64_t high_bit;
  high_bit = 0;
  val = pp_profiler_ticks_func();
  if (P_UNLIKELY (val < last_counter)) {
    high_bit = 1;
  }
  return ((val | (high_bit << 32)) - last_counter) * 1000;
}

u64_t
p_profiler_get_ticks_internal() {
  return pp_profiler_ticks_func();
}

u64_t
p_profiler_elapsed_usecs_internal(const profiler_t *profiler) {
  return pp_profiler_elapsed_func(profiler->counter);
}

void
p_profiler_init(void) {
  LARGE_INTEGER tcounter;
  HMODULE hmodule;
  bool has_qpc;
  has_qpc =
    (QueryPerformanceCounter(&tcounter) != 0 && tcounter.QuadPart != 0) ? true
      : false;
  if (has_qpc == true) {
    if (P_UNLIKELY (QueryPerformanceFrequency(&tcounter) == 0)) {
      P_ERROR (
        "profiler_t::p_profiler_init: QueryPerformanceFrequency() failed");
      has_qpc = false;
    } else {
      pp_profiler_freq = (u64_t) (tcounter.QuadPart);
      pp_profiler_ticks_func =
        (PWin32TicksFunc) pp_profiler_get_hr_ticks;
      pp_profiler_elapsed_func =
        (PWin32ElapsedFunc) pp_profiler_elapsed_hr;
    }
  }
  if (P_UNLIKELY (has_qpc == false)) {
    hmodule = GetModuleHandleA("kernel32.dll");
    if (P_UNLIKELY (hmodule == NULL)) {
      P_ERROR (
        "profiler_t::p_profiler_init: failed to load kernel32.dll module");
      return;
    }
    pp_profiler_ticks_func =
      (PWin32TicksFunc) GetProcAddress(hmodule, "GetTickCount64");
    pp_profiler_elapsed_func =
      (PWin32ElapsedFunc) pp_profiler_elapsed_tick64;
    if (P_UNLIKELY (pp_profiler_ticks_func == NULL)) {
      pp_profiler_ticks_func =
        (PWin32TicksFunc) GetProcAddress(hmodule, "GetTickCount");
      pp_profiler_elapsed_func =
        (PWin32ElapsedFunc) pp_profiler_elapsed_tick;
    }
    if (P_UNLIKELY (pp_profiler_ticks_func == NULL)) {
      P_ERROR ("profiler_t::p_profiler_init: no reliable tick counter");
      pp_profiler_elapsed_func = NULL;
    }
  }
}

void
p_profiler_shutdown(void) {
  pp_profiler_freq = 1;
  pp_profiler_ticks_func = NULL;
  pp_profiler_elapsed_func = NULL;
}
