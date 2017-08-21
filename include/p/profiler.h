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

/**
 * @file p/bench.h
 * @brief Time profiler
 * @author Alexander Saprykin
 *
 * #PTimeProfiler acts like a time cronometer: in any moment of time you can
 * make a time slice to see how much time elapsed since the last slice or timer
 * start.
 *
 * This profiler is useful to gather information about execution time for calls
 * or parts of the code. It can help to leverage bottle-necks in your code.
 *
 * To start using a profiler create a new one with p_profiler_new() call
 * and p_profiler_elapsed_usecs() to get elapsed time since the creation.
 * If you need to reset a profiler use p_profiler_reset(). Remove a
 * profiler with p_profiler_free().
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef P_PROFILER_H__
#define P_PROFILER_H__

#include "p/macros.h"
#include "p/types.h"

/** Time profiler opaque data structure. */
typedef struct p_profiler p_profiler_t;

/**
 * @brief Creates a new #PTimeProfiler object.
 * @return Pointer to a newly created #PTimeProfiler object.
 * @since 0.0.1
 */
P_API p_profiler_t *p_profiler_new(void);

/**
 * @brief Resets the #PTimeProfiler's internal counter to zero.
 * @param profiler Time profiler to reset.
 * @since 0.0.1
 *
 * After a reset the time profiler begins to count elapsed time from that moment
 * of time.
 */
P_API void p_profiler_reset(p_profiler_t *profiler);

/**
 * @brief Calculates elapsed time since the last reset or creation.
 * @param profiler Time profiler to calculate elapsed time for.
 * @return Microseconds elapsed since the last reset or creation.
 * @since 0.0.1
 */
P_API uint64_t p_profiler_elapsed_usecs(const p_profiler_t *profiler);

/**
 * @brief Frees #PTimeProfiler object.
 * @param profiler #PTimeProfiler to free.
 * @since 0.0.1
 */
P_API void p_profiler_free(p_profiler_t *profiler);

#endif /* P_PROFILER_H__ */
