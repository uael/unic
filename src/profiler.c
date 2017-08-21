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

#include "p/mem.h"
#include "p/profiler.h"
#include "ptimeprofiler-private.h"

extern uint64_t p_profiler_get_ticks_internal(void);
extern uint64_t p_profiler_elapsed_usecs_internal(
  const p_profiler_t *profiler);

P_API p_profiler_t *
p_profiler_new() {
  p_profiler_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_profiler_t))) == NULL)) {
    P_ERROR ("p_profiler_t: failed to allocate memory");
    return NULL;
  }

  ret->counter = p_profiler_get_ticks_internal();

  return ret;
}

P_API void
p_profiler_reset(p_profiler_t *profiler) {
  if (P_UNLIKELY (profiler == NULL))
    return;

  profiler->counter = p_profiler_get_ticks_internal();
}

P_API uint64_t
p_profiler_elapsed_usecs(const p_profiler_t *profiler) {
  if (P_UNLIKELY (profiler == NULL))
    return 0;

  return p_profiler_elapsed_usecs_internal(profiler);
}

P_API void
p_profiler_free(p_profiler_t *profiler) {
  p_free(profiler);
}