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

#ifndef UNIC_HEADER_PTREE_PRIVATE_H
# define UNIC_HEADER_PTREE_PRIVATE_H

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Base tree leaf structure. */
typedef struct PTreeBaseNode_ {

  /*!@brief Left child. */
  struct PTreeBaseNode_ *left;

  /*!@brief Right child. */
  struct PTreeBaseNode_ *right;

  /*!@brief Node key. */
  ptr_t key;

  /*!@brief Node value. */
  ptr_t value;
} PTreeBaseNode;

#endif /* UNIC_HEADER_PTREE_PRIVATE_H */
