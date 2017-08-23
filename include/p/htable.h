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

/*!@file p/htable.h
 * @brief Hash table
 * @author Alexander Saprykin
 *
 * A hash table is a data structure used to map keys to values. The hash table
 * consists of several internal slots which hold a list of values. A hash
 * function is used to compute an index in the array of the slots from a given
 * key. The hash function itself is fast and it takes a constant time to compute
 * the internal slot index.
 *
 * When the number of pairs in the hash table is small the lookup and insert
 * (remove) operations are very fast and have average complexity O(1), because
 * every slot holds almost the only one pair. As the number of internal slots is
 * fixed, the increasing number of pairs will lead to degraded performance and
 * the average complexity of the operations can drop to O(N) in the worst case.
 * This is because the more pairs are inserted the more longer the list of
 * values is placed in every slot.
 *
 * This is a simple hash table implementation which is not intended for heavy
 * usage (several thousands), see #PTree if you need the best performance on
 * large data sets. This implementation doesn't support multi-inserts when
 * several values belong to the same key.
 *
 * Note that #PHashTable stores keys and values only as pointers, so you need
 * to free used memory manually, p_htable_free() will not do it in any way.
 *
 * Integers (up to 32 bits) can be stored in pointers using #P_POINTER_TO_INT
 * and #P_INT_TO_POINTER macros.
 */
#ifndef P_HTABLE_H__
# define P_HTABLE_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/list.h"

/*!@brief Opaque data structure for a hash table. */
typedef struct htable htable_t;

/*!@brief Initializes a new hash table.
 * @return Pointer to a  newly initialized #PHashTable structure in case of
 * success, NULL otherwise.
 * @since 0.0.1
 * @note Free with p_htable_free() after usage.
 */
P_API htable_t *
p_htable_new(void);

/*!@brief Inserts a new key-value pair into a hash table.
 * @param table Initialized hash table.
 * @param key Key to insert.
 * @param value Value to insert.
 * @since 0.0.1
 *
 * This function only stores pointers, so you need to manually free pointed
 * data after using the hash table.
 */
P_API void
p_htable_insert(htable_t *table, ptr_t key, ptr_t value);

/*!@brief Searches for a specifed key in the hash table.
 * @param table Hash table to lookup in.
 * @param key Key to lookup for.
 * @return Value related to its key pair (can be NULL), (#ptr_t) -1 if no
 * value was found.
 * @since 0.0.1
 */
P_API ptr_t
p_htable_lookup(const htable_t *table, const_ptr_t key);

/*!@brief Gives a list of all the stored keys in the hash table.
 * @param table Hash table to collect the keys from.
 * @return List of all the stored keys, the list can be empty if no keys were
 * found.
 * @since 0.0.1
 * @note You should manually free the returned list with p_list_free() after
 * using it.
 */
P_API list_t *
p_htable_keys(const htable_t *table);

/*!@brief Gives a list of all the stored values in the hash table.
 * @param table Hash table to collect the values from.
 * @return List of all the stored values, the list can be empty if no keys were
 * found.
 * @since 0.0.1
 * @note You should manually free the returned list with p_list_free() after
 * using it.
 */
P_API list_t *
p_htable_values(const htable_t *table);

/*!@brief Frees a previously initialized #PHashTable.
 * @param table Hash table to free.
 * @since 0.0.1
 */
P_API void
p_htable_free(htable_t *table);

/*!@brief Removes @a key from a hash table.
 * @param table Hash table to remove the key from.
 * @param key Key to remove (if exists).
 * @since 0.0.1
 */
P_API void
p_htable_remove(htable_t *table, const_ptr_t key);

/*!@brief Searches for a specifed key in the hash table by its value.
 * @param table Hash table to lookup in.
 * @param val Value to lookup keys for.
 * @param func Function to compare table's values with @a val, if NULL then
 * values will be compared as pointers.
 * @return List of the keys with @a val (can be NULL), NULL if no keys were
 * found.
 * @since 0.0.1
 * @note Caller is responsible to call p_list_free() on the returned list after
 * usage.
 *
 * The compare function should return 0 if a value from the hash table (the
 * first parameter) is accepted related to the given lookup value (the second
 * parameter), and -1 or 1 otherwise.
 */
P_API list_t *
p_htable_lookup_by_value(const htable_t *table, const_ptr_t val,
  cmp_fn_t func);

#endif /* !P_HTABLE_H__ */
