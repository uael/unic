/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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
#include "p/tree.h"
#include "ptree-avl.h"
#include "ptree-bst.h"
#include "ptree-rb.h"

typedef bool  (*PTreeInsertNode)(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  ptr_t key,
  ptr_t value);

typedef bool  (*PTreeRemoveNode)(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  const_ptr_t key);

typedef void    (*PTreeFreeNode)(PTreeBaseNode *node);

struct tree {
  PTreeBaseNode *root;
  PTreeInsertNode insert_node_func;
  PTreeRemoveNode remove_node_func;
  PTreeFreeNode free_node_func;
  destroy_fn_t key_destroy_func;
  destroy_fn_t value_destroy_func;
  cmp_data_fn_t compare_func;
  ptr_t data;
  tree_kind_t type;
  int_t nnodes;
};

tree_t *
p_tree_new(tree_kind_t type, cmp_fn_t func) {
  return p_tree_new_full(type, (cmp_data_fn_t) func, NULL, NULL, NULL);
}

tree_t *
p_tree_new_with_data(tree_kind_t type, cmp_data_fn_t func, ptr_t data) {
  return p_tree_new_full(type, func, data, NULL, NULL);
}

tree_t *
p_tree_new_full(tree_kind_t type, cmp_data_fn_t func, ptr_t data,
  destroy_fn_t key_destroy, destroy_fn_t value_destroy) {
  tree_t *ret;

  if (P_UNLIKELY (type < P_TREE_TYPE_BINARY || type > P_TREE_TYPE_AVL)) {
    return NULL;
  }
  if (P_UNLIKELY (func == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(tree_t))) == NULL)) {
    P_ERROR ("tree_t::p_tree_new_full: failed to allocate memory");
    return NULL;
  }
  ret->type = type;
  ret->compare_func = func;
  ret->data = data;
  ret->key_destroy_func = key_destroy;
  ret->value_destroy_func = value_destroy;
  switch (type) {
    case P_TREE_TYPE_BINARY:
      ret->insert_node_func = p_tree_bst_insert;
      ret->remove_node_func = p_tree_bst_remove;
      ret->free_node_func = p_tree_bst_node_free;
      break;
    case P_TREE_TYPE_RB:
      ret->insert_node_func = p_tree_rb_insert;
      ret->remove_node_func = p_tree_rb_remove;
      ret->free_node_func = p_tree_rb_node_free;
      break;
    case P_TREE_TYPE_AVL:
      ret->insert_node_func = p_tree_avl_insert;
      ret->remove_node_func = p_tree_avl_remove;
      ret->free_node_func = p_tree_avl_node_free;
      break;
  }
  return ret;
}

void
p_tree_insert(tree_t *tree, ptr_t key, ptr_t value) {
  bool result;

  if (P_UNLIKELY (tree == NULL)) {
    return;
  }
  result = tree->insert_node_func(
    &tree->root,
    tree->compare_func,
    tree->data,
    tree->key_destroy_func,
    tree->value_destroy_func,
    key,
    value
  );
  if (result == true) {
    ++tree->nnodes;
  }
}

bool
p_tree_remove(tree_t *tree, const_ptr_t key) {
  bool result;

  if (P_UNLIKELY (tree == NULL || tree->root == NULL)) {
    return false;
  }
  result = tree->remove_node_func(
    &tree->root,
    tree->compare_func,
    tree->data,
    tree->key_destroy_func,
    tree->value_destroy_func,
    key
  );
  if (result == true) {
    --tree->nnodes;
  }
  return result;
}

ptr_t
p_tree_lookup(tree_t *tree, const_ptr_t key) {
  PTreeBaseNode *cur_node;
  int_t cmp_result;

  if (P_UNLIKELY (tree == NULL)) {
    return NULL;
  }
  cur_node = tree->root;
  while (cur_node != NULL) {
    cmp_result = tree->compare_func(key, cur_node->key, tree->data);
    if (cmp_result < 0) {
      cur_node = cur_node->left;
    } else if (cmp_result > 0) {
      cur_node = cur_node->right;
    } else {
      return cur_node->value;
    }
  }
  return NULL;
}

void
p_tree_foreach(tree_t *tree, traverse_fn_t traverse_func, ptr_t user_data) {
  PTreeBaseNode *cur_node;
  PTreeBaseNode *prev_node;
  int_t mod_counter;
  bool need_stop;

  if (P_UNLIKELY (tree == NULL || traverse_func == NULL)) {
    return;
  }
  if (P_UNLIKELY (tree->root == NULL)) {
    return;
  }
  cur_node = tree->root;
  mod_counter = 0;
  need_stop = false;
  while (cur_node != NULL) {
    if (cur_node->left == NULL) {
      if (need_stop == false) {
        need_stop = traverse_func(
          cur_node->key,
          cur_node->value,
          user_data
        );
      }
      cur_node = cur_node->right;
    } else {
      prev_node = cur_node->left;
      while (prev_node->right != NULL && prev_node->right != cur_node) {
        prev_node = prev_node->right;
      }
      if (prev_node->right == NULL) {
        prev_node->right = cur_node;
        cur_node = cur_node->left;
        ++mod_counter;
      } else {
        if (need_stop == false) {
          need_stop = traverse_func(
            cur_node->key,
            cur_node->value,
            user_data
          );
        }
        cur_node = cur_node->right;
        prev_node->right = NULL;
        --mod_counter;
        if (need_stop == true && mod_counter == 0) {
          return;
        }
      }
    }
  }
}

void
p_tree_clear(tree_t *tree) {
  PTreeBaseNode *cur_node;
  PTreeBaseNode *prev_node;
  PTreeBaseNode *next_node;

  if (P_UNLIKELY (tree == NULL || tree->root == NULL)) {
    return;
  }
  cur_node = tree->root;
  while (cur_node != NULL) {
    if (cur_node->left == NULL) {
      next_node = cur_node->right;
      if (tree->key_destroy_func != NULL) {
        tree->key_destroy_func(cur_node->key);
      }
      if (tree->value_destroy_func != NULL) {
        tree->value_destroy_func(cur_node->value);
      }
      tree->free_node_func(cur_node);
      --tree->nnodes;
      cur_node = next_node;
    } else {
      prev_node = cur_node->left;
      while (prev_node->right != NULL) {
        prev_node = prev_node->right;
      }
      prev_node->right = cur_node;
      next_node = cur_node->left;
      cur_node->left = NULL;
      cur_node = next_node;
    }
  }
  tree->root = NULL;
}

tree_kind_t
p_tree_get_type(const tree_t *tree) {
  if (P_UNLIKELY (tree == NULL)) {
    return (tree_kind_t) -1;
  }
  return tree->type;
}

int_t
p_tree_get_nnodes(const tree_t *tree) {
  if (P_UNLIKELY (tree == NULL)) {
    return 0;
  }
  return tree->nnodes;
}

void
p_tree_free(tree_t *tree) {
  p_tree_clear(tree);
  p_free(tree);
}
