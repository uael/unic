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

#ifndef UNIC_HEADER_Pthread_PRIVATE_H
# define UNIC_HEADER_Pthread_PRIVATE_H

#include "unic/macros.h"
#include "unic/types.h"
#include "unic/thread.h"

/*!@brief Base thread structure */
typedef struct thread_base {

  /*!@brief Reference counter. */
  int ref_count;

  /*!@brief Return code. */
  int ret_code;

  /*!@brief Our thread flag. */
  bool ours;

  /*!@brief Joinable flag. */
  bool joinable;

  /*!@brief Thread routine. */
  thread_fn_t func;

  /*!@brief Thread input data. */
  ptr_t data;

  /*!@brief Thread priority. */
  thread_prio_t prio;
} thread_base_t;

#endif /* UNIC_HEADER_Pthread_PRIVATE_H */
