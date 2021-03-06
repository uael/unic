/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
 * Illustrations have been taken from the Linux kernel rbtree.c
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
#include "tree-rb.h"

typedef enum PTreeRBColor_ {
  U_TREE_RB_COLOR_RED = 0x01,
  U_TREE_RB_COLOR_BLACK = 0x02
} PTreeRBColor;

typedef struct PTreeRBNode_ {
  struct PTreeBaseNode_ base;
  struct PTreeRBNode_ *parent;
  PTreeRBColor color;
} PTreeRBNode;

static bool
pp_tree_rb_is_black(PTreeRBNode *node);

static bool
pp_tree_rb_is_red(PTreeRBNode *node);

static PTreeRBNode *
pp_tree_rb_get_gparent(PTreeRBNode *node);

static PTreeRBNode *
pp_tree_rb_get_uncle(PTreeRBNode *node);

static PTreeRBNode *
pp_tree_rb_get_sibling(PTreeRBNode *node);

static void
pp_tree_rb_rotate_left(PTreeRBNode *node, PTreeBaseNode **root);

static void
pp_tree_rb_rotate_right(PTreeRBNode *node, PTreeBaseNode **root);

static void
pp_tree_rb_balance_insert(PTreeRBNode *node, PTreeBaseNode **root);

static void
pp_tree_rb_balance_remove(PTreeRBNode *node, PTreeBaseNode **root);

static bool
pp_tree_rb_is_black(PTreeRBNode *node) {
  if (node == NULL) {
    return true;
  }
  return ((node->color) & U_TREE_RB_COLOR_BLACK) > 0 ? true : false;
}

static bool
pp_tree_rb_is_red(PTreeRBNode *node) {
  return ((node->color) & U_TREE_RB_COLOR_RED) > 0 ? true : false;
}

static PTreeRBNode *
pp_tree_rb_get_gparent(PTreeRBNode *node) {
  return node->parent->parent;
}

static PTreeRBNode *
pp_tree_rb_get_uncle(PTreeRBNode *node) {
  PTreeRBNode *gparent = pp_tree_rb_get_gparent(node);
  if ((PTreeRBNode *) gparent->base.left == node->parent) {
    return (PTreeRBNode *) gparent->base.right;
  } else {
    return (PTreeRBNode *) gparent->base.left;
  }
}

static PTreeRBNode *
pp_tree_rb_get_sibling(PTreeRBNode *node) {
  if (node->parent->base.left == (PTreeBaseNode *) node) {
    return (PTreeRBNode *) node->parent->base.right;
  } else {
    return (PTreeRBNode *) node->parent->base.left;
  }
}

static void
pp_tree_rb_rotate_left(PTreeRBNode *node, PTreeBaseNode **root) {
  PTreeBaseNode *tmp_node;
  tmp_node = node->base.right;
  if (U_LIKELY (node->parent != NULL)) {
    if (node->parent->base.left == (PTreeBaseNode *) node) {
      node->parent->base.left = tmp_node;
    } else {
      node->parent->base.right = tmp_node;
    }
  }
  node->base.right = tmp_node->left;
  if (tmp_node->left != NULL) {
    ((PTreeRBNode *) tmp_node->left)->parent = node;
  }
  tmp_node->left = (PTreeBaseNode *) node;
  ((PTreeRBNode *) tmp_node)->parent = node->parent;
  node->parent = (PTreeRBNode *) tmp_node;
  if (U_UNLIKELY (((PTreeRBNode *) tmp_node)->parent == NULL)) {
    *root = tmp_node;
  }
}

static void
pp_tree_rb_rotate_right(PTreeRBNode *node, PTreeBaseNode **root) {
  PTreeBaseNode *tmp_node;
  tmp_node = node->base.left;
  if (U_LIKELY (node->parent != NULL)) {
    if (node->parent->base.left == (PTreeBaseNode *) node) {
      node->parent->base.left = tmp_node;
    } else {
      node->parent->base.right = tmp_node;
    }
  }
  node->base.left = tmp_node->right;
  if (tmp_node->right != NULL) {
    ((PTreeRBNode *) tmp_node->right)->parent = node;
  }
  tmp_node->right = (PTreeBaseNode *) node;
  ((PTreeRBNode *) tmp_node)->parent = node->parent;
  node->parent = (PTreeRBNode *) tmp_node;
  if (U_UNLIKELY (((PTreeRBNode *) tmp_node)->parent == NULL)) {
    *root = tmp_node;
  }
}

static void
pp_tree_rb_balance_insert(PTreeRBNode *node, PTreeBaseNode **root) {
  PTreeRBNode *uncle;
  PTreeRBNode *gparent;
  while (true) {
    /* Case 1: We are at the root  */
    if (U_UNLIKELY (node->parent == NULL)) {
      node->color = U_TREE_RB_COLOR_BLACK;
      break;
    }

    /* Case 2: We have a black parent */
    if (pp_tree_rb_is_black(node->parent) == true) {
      break;
    }
    uncle = pp_tree_rb_get_uncle(node);
    gparent = pp_tree_rb_get_gparent(node);

    /* Case 3: Both parent and uncle are red, flip colors
     *
     *       G            g
     *      / \          / \
     *     unic   u  -->   P   U
     *    /            /
     *   n            n
     */
    if (uncle != NULL && pp_tree_rb_is_red(uncle) == true) {
      node->parent->color = U_TREE_RB_COLOR_BLACK;
      uncle->color = U_TREE_RB_COLOR_BLACK;
      gparent->color = U_TREE_RB_COLOR_RED;

      /* Continue iteratively from gparent */
      node = gparent;
      continue;
    }
    if (node->parent == (PTreeRBNode *) gparent->base.left) {
      if (node == (PTreeRBNode *) node->parent->base.right) {
        /* Case 4a: Left rotate at parent
         *
         *      G             G
         *     / \           / \
         *    unic   U  -->    n   U
         * \           /
         *      n         unic
         */
        pp_tree_rb_rotate_left(node->parent, root);
        node = (PTreeRBNode *) node->base.left;
      }
      gparent->color = U_TREE_RB_COLOR_RED;
      node->parent->color = U_TREE_RB_COLOR_BLACK;

      /* Case 5a: Right rotate at gparent
       *
       *        G           P
       *       / \         / \
       *      unic   U  -->  n   g
       *     / \
       *    n                   U
       */
      pp_tree_rb_rotate_right(gparent, root);
      break;
    } else {
      if (node == (PTreeRBNode *) node->parent->base.left) {
        /* Case 4b: Right rotate at parent */
        pp_tree_rb_rotate_right(node->parent, root);
        node = (PTreeRBNode *) node->base.right;
      }
      gparent->color = U_TREE_RB_COLOR_RED;
      node->parent->color = U_TREE_RB_COLOR_BLACK;

      /* Case 5b: Left rotate at gparent*/
      pp_tree_rb_rotate_left(gparent, root);
      break;
    }
  }
}

static void
pp_tree_rb_balance_remove(PTreeRBNode *node, PTreeBaseNode **root) {
  PTreeRBNode *sibling;
  while (true) {
    /* Case 1: We are at the root */
    if (U_UNLIKELY (node->parent == NULL)) {
      break;
    }
    sibling = pp_tree_rb_get_sibling(node);
    if (pp_tree_rb_is_red(sibling) == true) {
      /*
       * Case 2: Left (right) rotate at parent
       *
       *     P               S
       *    / \             / \
       *   N   s    -->    unic   Sr
       *      / \         / \
       *     Sl  Sr      N   Sl
       */
      node->parent->color = U_TREE_RB_COLOR_RED;
      sibling->color = U_TREE_RB_COLOR_BLACK;
      if ((PTreeBaseNode *) node == node->parent->base.left) {
        pp_tree_rb_rotate_left(node->parent, root);
      } else {
        pp_tree_rb_rotate_right(node->parent, root);
      }
      sibling = pp_tree_rb_get_sibling(node);
    }

    /*
     * Case 3: Sibling (parent) color flip
     *
     *    (unic)           (unic)
     *    / \           / \
     *   N   S    -->  N   s
     *      / \           / \
     *     Sl  Sr        Sl  Sr
     */
    if (pp_tree_rb_is_black((PTreeRBNode *) sibling->base.left) == true &&
      pp_tree_rb_is_black((PTreeRBNode *) sibling->base.right) == true) {
      sibling->color = U_TREE_RB_COLOR_RED;
      if (pp_tree_rb_is_black(node->parent) == true) {
        node = node->parent;
        continue;
      } else {
        node->parent->color = U_TREE_RB_COLOR_BLACK;
        break;
      }
    }

    /*
     * Case 4: Right (left) rotate at sibling
     *
     *   (unic)           (unic)
     *   / \           / \
     *  N   S    -->  N   Sl
     *     / \ \
     *    sl  Sr            s
     * \
     *                        Sr
     */
    if ((PTreeBaseNode *) node == node->parent->base.left &&
      pp_tree_rb_is_black((PTreeRBNode *) sibling->base.right) == true) {
      sibling->color = U_TREE_RB_COLOR_RED;
      ((PTreeRBNode *) sibling->base.left)->color = U_TREE_RB_COLOR_BLACK;
      pp_tree_rb_rotate_right(sibling, root);
      sibling = pp_tree_rb_get_sibling(node);
    } else if ((PTreeBaseNode *) node == node->parent->base.right &&
      pp_tree_rb_is_black((PTreeRBNode *) sibling->base.left) == true) {
      sibling->color = U_TREE_RB_COLOR_RED;
      ((PTreeRBNode *) sibling->base.right)->color = U_TREE_RB_COLOR_BLACK;
      pp_tree_rb_rotate_left(sibling, root);
      sibling = pp_tree_rb_get_sibling(node);
    }

    /*
     * Case 5: Left (right) rotate at parent and color flips
     *
     *      (unic)             (s)
     *      / \             / \
     *     N   S     -->   P   Sr
     *        / \         / \
     *      (sl) sr      N  (sl)
     */
    sibling->color = node->parent->color;
    node->parent->color = U_TREE_RB_COLOR_BLACK;
    if ((PTreeBaseNode *) node == node->parent->base.left) {
      ((PTreeRBNode *) sibling->base.right)->color = U_TREE_RB_COLOR_BLACK;
      pp_tree_rb_rotate_left(node->parent, root);
    } else {
      ((PTreeRBNode *) sibling->base.left)->color = U_TREE_RB_COLOR_BLACK;
      pp_tree_rb_rotate_right(node->parent, root);
    }
    break;
  }
}

bool
u_tree_rb_insert(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  ptr_t key,
  ptr_t value) {
  PTreeBaseNode **cur_node;
  PTreeBaseNode *parent_node;
  int cmp_result;
  cur_node = root_node;
  parent_node = *root_node;

  /* Find where to insert the node */
  while (*cur_node != NULL) {
    cmp_result = compare_func(key, (*cur_node)->key, data);
    if (cmp_result < 0) {
      parent_node = *cur_node;
      cur_node = &(*cur_node)->left;
    } else if (cmp_result > 0) {
      parent_node = *cur_node;
      cur_node = &(*cur_node)->right;
    } else {
      break;
    }
  }

  /* If we have existing one - replace a key-value pair */
  if (*cur_node != NULL) {
    if (key_destroy_func != NULL) {
      key_destroy_func((*cur_node)->key);
    }
    if (value_destroy_func != NULL) {
      value_destroy_func((*cur_node)->value);
    }
    (*cur_node)->key = key;
    (*cur_node)->value = value;
    return false;
  }
  if (U_UNLIKELY ((*cur_node = u_malloc0(sizeof(PTreeRBNode))) == NULL)) {
    return false;
  }
  (*cur_node)->key = key;
  (*cur_node)->value = value;
  ((PTreeRBNode *) *cur_node)->color = U_TREE_RB_COLOR_RED;
  ((PTreeRBNode *) *cur_node)->parent = (PTreeRBNode *) parent_node;

  /* Balance the tree */
  pp_tree_rb_balance_insert((PTreeRBNode *) *cur_node, root_node);
  return true;
}

bool
u_tree_rb_remove(PTreeBaseNode **root_node,
  cmp_data_fn_t compare_func,
  ptr_t data,
  destroy_fn_t key_destroy_func,
  destroy_fn_t value_destroy_func,
  const_ptr_t key) {
  PTreeBaseNode *cur_node;
  PTreeBaseNode *prev_node;
  PTreeBaseNode *child_node;
  PTreeRBNode *child_parent;
  int cmp_result;
  cur_node = *root_node;
  while (cur_node != NULL) {
    cmp_result = compare_func(key, cur_node->key, data);
    if (cmp_result < 0) {
      cur_node = cur_node->left;
    } else if (cmp_result > 0) {
      cur_node = cur_node->right;
    } else {
      break;
    }
  }
  if (U_UNLIKELY (cur_node == NULL)) {
    return false;
  }
  if (cur_node->left != NULL && cur_node->right != NULL) {
    prev_node = cur_node->left;
    while (prev_node->right != NULL) {
      prev_node = prev_node->right;
    }
    cur_node->key = prev_node->key;
    cur_node->value = prev_node->value;

    /* Mark node for removal */
    cur_node = prev_node;
  }
  child_node = cur_node->left == NULL ? cur_node->right : cur_node->left;
  if (child_node == NULL
    && pp_tree_rb_is_black((PTreeRBNode *) cur_node) == true) {
      pp_tree_rb_balance_remove((PTreeRBNode *) cur_node, root_node);
  }

  /* Replace node with its child */
  if (cur_node == *root_node) {
    *root_node = child_node;
    child_parent = NULL;
  } else {
    child_parent = ((PTreeRBNode *) cur_node)->parent;
    if (child_parent->base.left == cur_node) {
      child_parent->base.left = child_node;
    } else {
      child_parent->base.right = child_node;
    }
  }
  if (child_node != NULL) {
    ((PTreeRBNode *) child_node)->parent = child_parent;

    /* Check if we need to repaint the node */
    if (pp_tree_rb_is_black((PTreeRBNode *) cur_node) == true) {
      ((PTreeRBNode *) child_node)->color = U_TREE_RB_COLOR_BLACK;
    }
  }

  /* Free unused node */
  if (key_destroy_func != NULL) {
    key_destroy_func(cur_node->key);
  }
  if (value_destroy_func != NULL) {
    value_destroy_func(cur_node->value);
  }
  u_free(cur_node);
  return true;
}

void
u_tree_rb_node_free(PTreeBaseNode *node) {
  u_free(node);
}
