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

#ifndef UNIC_HEADER_PTIMEPROFILER_PRIVATE_H
# define UNIC_HEADER_PTIMEPROFILER_PRIVATE_H

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Time profiler opaque data structure. */
struct profiler {

  /*!@brief Ticks counter. */
  u64_t counter;
};
#endif /* UNIC_HEADER_PTIMEPROFILER_PRIVATE_H */
