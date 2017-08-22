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

/*!@file p/tree.h
 * @brief Binary tree data structure
 * @author Alexander Saprykin
 *
 * #PTree represents a binary search tree structure for faster lookup than
 * a plain array or a list. It has average O(logN) time complexity to search
 * a key-value pair, and O(N) in the worst case (when a tree is degenerated into
 * the list).
 *
 * Currently #PTree supports the following tree types:
 * - unbalanced binary search tree;
 * - red-black self-balancing tree;
 * - AVL self-balancing tree.
 *
 * Use p_tree_new(), or its detailed variations like p_tree_new_with_data() and
 * p_tree_new_full() to create a tree structure. Take attention that a caller
 * owns the key and the value data passed when inserting new nodes, so you
 * should manually free the memory after the tree usage. Or you can provide
 * destroy notification functions for the keys and the values separately.
 *
 * New key-value pairs can be inserted with p_tree_insert() and removed with
 * p_tree_remove().
 *
 * Use p_tree_lookup() to find the value by a given key. You can also traverse
 * the tree in-order with p_tree_foreach().
 *
 * Release memory with p_tree_free() or clear a tree with p_tree_clear(). Keys
 * and values would be destroyed only if the corresponding notification
 * functions were provided.
 *
 * Note: all operations with the tree are non-recursive, only iterative calls
 * are used.
 */
#ifndef P_TREE_H__
# define P_TREE_H__

#include "p/macros.h"
#include "p/types.h"

/*!@brief Tree opaque data structure. */
typedef struct tree tree_t;

/*!@brief Internal data organization algorithm for #PTree. */
enum tree_kind {

  /*!@brief Unbalanced binary tree. */
  P_TREE_TYPE_BINARY = 0,

  /*!@brief Red-black self-balancing tree. */
  P_TREE_TYPE_RB = 1,

  /*!@brief AVL self-balancing tree. */
  P_TREE_TYPE_AVL = 2
};

typedef enum tree_kind tree_kind_t;

/*!@brief Initializes new #PTree.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * The caller takes ownership of all the keys and the values passed to the tree.
 */
P_API tree_t *
p_tree_new(tree_kind_t type, cmp_fn_t func);

/*!@brief Initializes new #PTree with additional data.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @param data Data to be passed to @a func along with the keys.
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * The caller takes ownership of all the keys and the values passed to the tree.
 */
P_API tree_t *
p_tree_new_with_data(tree_kind_t type, cmp_data_fn_t func, ptr_t data);

/*!@brief Initializes new #PTree with additional data and memory management.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @param data Data to be passed to @a func along with the keys.
 * @param key_destroy Function to call on every key before the node destruction,
 * maybe NULL.
 * @param value_destroy Function to call on every value before the node
 * destruction, maybe NULL.
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Upon every node destruction the corresponding key and value functions would
 * be called.
 */
P_API tree_t *
p_tree_new_full(tree_kind_t type, cmp_data_fn_t func, ptr_t data,
  destroy_fn_t key_destroy, destroy_fn_t value_destroy);

/*!@brief Inserts a new key-value pair into a tree.
 * @param tree #PTree to insert a node in.
 * @param key Key to insert.
 * @param value Value corresponding to the given @a key.
 * @since 0.0.1
 *
 * If the @a key already exists in the tree then it will be replaced with the
 * new one. If a key destroy function was provided it would be called on the old
 * key. If a value destroy function was provided it would be called on the old
 * value.
 */
P_API void
p_tree_insert(tree_t *tree, ptr_t key, ptr_t value);

/*!@brief Removes a key from a tree.
 * @param tree #PTree to remove a key from.
 * @param key A key to lookup.
 * @return TRUE if the key was removed, FALSE if the key was not found.
 * @since 0.0.1
 *
 * If a key destroy function was provided it would be called on the key. If a
 * value destroy function was provided it would be called on the old value.
 */
P_API bool
p_tree_remove(tree_t *tree, const_ptr_t key);

/*!@brief Lookups a value by a given key.
 * @param tree #PTree to lookup in.
 * @param key Key to lookup.
 * @return Value for the given @a key in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_API ptr_t
p_tree_lookup(tree_t *tree, const_ptr_t key);

/*!@brief Iterates in-order through the tree nodes.
 * @param tree A tree to traverse.
 * @param traverse_func Function for traversing.
 * @param user_data Additional (maybe NULL) user-provided data for the
 * @a traverse_func.
 * @since 0.0.1
 * @note Morris (non-recursive, non-stack) traversing algorithm is being used.
 *
 * The tree should not be modified while traversing. The internal tree structure
 * can be modified along the traversing process, so keep it in mind for
 * concurrent access.
 */
P_API void
p_tree_foreach(tree_t *tree, traverse_fn_t traverse_func, ptr_t user_data);

/*!@brief Clears a tree.
 * @param tree #PTree to clear.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is
 * being used.
 *
 * All the keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_API void
p_tree_clear(tree_t *tree);

/*!@brief Gets a tree algorithm type.
 * @param tree #PTree object to get the type for.
 * @return Tree internal organization algorithm used for a given object.
 * @since 0.0.1
 */
P_API tree_kind_t
p_tree_get_type(const tree_t *tree);

/*!@brief Gets node count.
 * @param tree #PTree to get node count for.
 * @return Node count.
 * @since 0.0.1
 *
 * If the tree is empty or an invalid pointer is given it returns 0.
 */
P_API int_t
p_tree_get_nnodes(const tree_t *tree);

/*!@brief Frees a previously initialized tree object.
 * @param tree #PTree object to free.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is
 * being used.
 *
 * All the keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_API void
p_tree_free(tree_t *tree);

#endif /* !P_TREE_H__ */
