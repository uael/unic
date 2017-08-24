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

#include "unic/mem.h"
#include "unic/profiler.h"
#include "profiler-private.h"

extern u64_t
u_profiler_get_ticks_internal(void);

extern u64_t
u_profiler_elapsed_usecs_internal(
  const profiler_t *profiler);

profiler_t *
u_profiler_new() {
  profiler_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(profiler_t))) == NULL)) {
    U_ERROR ("profiler_t: failed to allocate memory");
    return NULL;
  }
  ret->counter = u_profiler_get_ticks_internal();
  return ret;
}

void
u_profiler_reset(profiler_t *profiler) {
  if (U_UNLIKELY (profiler == NULL)) {
    return;
  }
  profiler->counter = u_profiler_get_ticks_internal();
}

u64_t
u_profiler_elapsed_usecs(const profiler_t *profiler) {
  if (U_UNLIKELY (profiler == NULL)) {
    return 0;
  }
  return u_profiler_elapsed_usecs_internal(profiler);
}

void
u_profiler_free(profiler_t *profiler) {
  u_free(profiler);
}
