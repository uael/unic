/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file unic/list.h
 * @brief Singly linked list
 * @author Alexander Saprykin
 *
 * A singly linked list is a data structure consists of the nodes which
 * represent a sequence. Each node contains a data pointer and a pointer to the
 * next node. Every node has a link only to the next node, hence list is a
 * singly linked (in a single direction).
 *
 * As the singly linked list is a linear collection of the nodes with the
 * sequential access, it has an O(N) average complexity for appending, removing
 * and searching operations. Prepending a node takes O(1) constant time. Thus it
 * is not intended for heavy usage, please refer to #htable_t or #tree_t if you
 * are working with large data sets.
 *
 * Before the first usage you must initialize a #list_t variable to NULL. After
 * that you can use the u_list_append(), u_list_prepend(), u_list_remove() and
 * u_list_reverse() routines to update that variable:
 * @code
 * PList       *list;
 * ppointer    data;
 *
 * list = NULL;
 * data = my_obj_new ();
 *
 * list = u_list_append (list, data);
 * @endcode
 * #list_t stores only the pointers to the data, so you must free used memory
 * manually, u_list_free() only frees list's internal memory, not the data it
 * stores the pointers for. The best approach to free used memory is the
 * u_list_foreach() routine:
 * @code
 * PList    *list;
 * ...
 * u_list_foreach (list, (PFunc) my_free_func, my_data);
 * u_list_free (list);
 * @endcode
 * Also you can use #U_INT_TO_POINTER and #U_POINTER_TO_INT macros to store
 * integers (up to 32-bit) without allocating memory for them:
 * @code
 * PList    *list;
 * pint     a;
 *
 * list = u_list_append (list, U_INT_TO_POINTER (12));
 * a = U_POINTER_TO_INT (list->data);
 * @endcode
 * #list_t can store several nodes with the same pointer value, but
 * u_list_remove() will remove only the first matching node.
 *
 * If you need to add large amount of nodes at once it is better to prepend them
 * and then reverse the list.
 */
#ifndef U_LIST_H__
# define U_LIST_H__

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Typedef for a list node. */
typedef struct list list_t;

/*!@brief Node for a singly linked list. */
struct list {

  /*!@brief Pointer to the node data. */
  ptr_t data;

  /*!@brief Next list node. */
  list_t *next;
};

/*!@brief Appends data to a list.
 * @param list #list_t for appending the data.
 * @param data Data to append.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before appending the first node to the list, @a list argument must be
 * initialized with NULL. Otherwise behavior is unpredictable.
 */
U_API list_t *
u_list_append(list_t *list, ptr_t data) U_GNUC_WARN_UNUSED_RESULT;

/*!@brief Removes data from a list.
 * @param list List to remove the data from.
 * @param data Data to remove.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * It searches for the first matching occurrence in the @a list and removes
 * that node. Note that it removes only the pointer from the @a list, not the
 * data it pointers to, so you need to free the data manually.
 */
U_API list_t *
u_list_remove(list_t *list, ptr_t data) U_GNUC_WARN_UNUSED_RESULT;

/*!@brief Calls a specified function for each list node.
 * @param list List to go through.
 * @param func Pointer for the callback function.
 * @param user_data User defined data, may be NULL.
 * @since 0.0.1
 *
 * This function goes through the whole @a list and calls @a func for each node.
 * The @a func will receive pointer to the node's data and @a user_data. You can
 * use it to free the data:
 * @code
 * u_list_foreach (list, (PFunc) free, NULL);
 * u_list_free (list);
 * @endcode
 */
U_API void
u_list_foreach(list_t *list, fn_t func, ptr_t user_data);

/*!@brief Frees list memory.
 * @param list List to free.
 * @since 0.0.1
 *
 * This function frees only the list's internal memory, not the data in the
 * pointers stored in the nodes. Don't forget to free all the data stored in the
 * list manually.
 */
U_API void
u_list_free(list_t *list);

/*!@brief Gets the last node from the list.
 * @param list List to get the node from.
 * @return Pointer to the last @a list node, NULL if the @a list is empty.
 * @since 0.0.1
 */
U_API list_t *
u_list_last(list_t *list);

/*!@brief Gets the number of list nodes.
 * @param list List to count nodes in.
 * @return Number of nodes in the @a list.
 * @since 0.0.1
 * @note This function will iterate through the whole @a list, so don't use it
 * in condition of the for-loop or in the code which is repeated a lot of times.
 */
U_API size_t
u_list_length(const list_t *list);

/*!@brief Prepends data to a list.
 * @param list #list_t for prepending the data.
 * @param data Data to prepend.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before prepending the first node to the list, @a list argument must be
 * initialized with NULL. Otherwise behavior is unpredictable.
 */
U_API list_t *
u_list_prepend(list_t *list, ptr_t data) U_GNUC_WARN_UNUSED_RESULT;

/*!@brief Reverses the list order.
 * @param list #list_t to reverse the order.
 * @return Pointer to the top of the reversed list.
 * @since 0.0.1
 */
U_API list_t *
u_list_reverse(list_t *list) U_GNUC_WARN_UNUSED_RESULT;

#endif /* !U_LIST_H__ */
