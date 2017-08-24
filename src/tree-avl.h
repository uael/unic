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

#ifndef UNIC_HEADER_PTREEAVL_H
# define UNIC_HEADER_PTREEAVL_H

#include "unic/macros.h"
#include "unic/types.h"
#include "tree-private.h"

bool
u_tree_avl_insert(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  ptr_t key,
  ptr_t value);

bool
u_tree_avl_remove(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  const_ptr_t key);

void
u_tree_avl_node_free(PTreeBaseNode *node);

#endif /* UNIC_HEADER_PTREEAVL_H */
