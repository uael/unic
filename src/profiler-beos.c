/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#include <kernel/OS.h>

#include "unic/profiler.h"
#include "profiler-private.h"

u64_t
u_profiler_get_ticks_internal() {
  return (u64_t) system_time();
}

u64_t
u_profiler_elapsed_usecs_internal(const profiler_t *profiler) {
  return ((u64_t) system_time()) - profiler->counter;
}

void
u_profiler_init(void) {
}

void
u_profiler_shutdown(void) {
}
