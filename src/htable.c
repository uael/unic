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

/* Hash table organized like this: table[hash key]->[list with values]
 * Note: this implementation is not intended to use on huge loads */

#include "p/mem.h"
#include "p/htable.h"

typedef struct bucket bucket_t;

struct bucket {
  bucket_t *next;
  ptr_t key;
  ptr_t value;
};

struct htable {
  bucket_t **table;
  size_t size;
};

/* Size of unique hash keys in hash table */
#define P_HASH_TABLE_SIZE 101

static uint_t
pp_hash_table_calc_hash(const_ptr_t pointer, size_t modulo);

static bucket_t *
pp_hash_table_find_node(const htable_t *table,
  const_ptr_t key);

static uint_t
pp_hash_table_calc_hash(const_ptr_t pointer, size_t modulo) {
  /* As simple as we can :) */
  return (uint_t) (((size_t) (P_POINTER_TO_INT(pointer) + 37)) % modulo);
}

static bucket_t *
pp_hash_table_find_node(const htable_t *table, const_ptr_t key) {
  uint_t hash;
  bucket_t *ret;

  hash = pp_hash_table_calc_hash(key, table->size);
  for (ret = table->table[hash]; ret != NULL; ret = ret->next) {
    if (ret->key == key) {
      return ret;
    }
  }
  return NULL;
}

htable_t *
p_hash_table_new(void) {
  htable_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(htable_t))) == NULL)) {
    P_ERROR ("htable_t::p_hash_table_new: failed(1) to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY (
    (ret->table = p_malloc0(P_HASH_TABLE_SIZE * sizeof(bucket_t *)))
      == NULL)) {
    P_ERROR ("htable_t::p_hash_table_new: failed(2) to allocate memory");
    p_free(ret);
    return NULL;
  }
  ret->size = P_HASH_TABLE_SIZE;
  return ret;
}

void
p_hash_table_insert(htable_t *table, ptr_t key, ptr_t value) {
  bucket_t *node;
  uint_t hash;

  if (P_UNLIKELY (table == NULL)) {
    return;
  }
  if ((node = pp_hash_table_find_node(table, key)) == NULL) {
    if (P_UNLIKELY ((node = p_malloc0(sizeof(bucket_t))) == NULL)) {
      P_ERROR ("htable_t::p_hash_table_insert: failed to allocate memory");
      return;
    }
    hash = pp_hash_table_calc_hash(key, table->size);

    /* Insert a new node in front of others */
    node->key = key;
    node->value = value;
    node->next = table->table[hash];
    table->table[hash] = node;
  } else {
    node->value = value;
  }
}

ptr_t
p_hash_table_lookup(const htable_t *table, const_ptr_t key) {
  bucket_t *node;

  if (P_UNLIKELY (table == NULL)) {
    return NULL;
  }
  return ((node = pp_hash_table_find_node(table, key)) == NULL)
    ? (ptr_t) (-1) : node->value;
}

list_t *
p_hash_table_keys(const htable_t *table) {
  list_t *ret = NULL;
  bucket_t *node;
  uint_t i;

  if (P_UNLIKELY (table == NULL)) {
    return NULL;
  }
  for (i = 0; i < table->size; ++i) {
    for (node = table->table[i]; node != NULL; node = node->next) {
      ret = p_list_append(ret, node->key);
    }
  }
  return ret;
}

list_t *
p_hash_table_values(const htable_t *table) {
  list_t *ret = NULL;
  bucket_t *node;
  uint_t i;

  if (P_UNLIKELY (table == NULL)) {
    return NULL;
  }
  for (i = 0; i < table->size; ++i) {
    for (node = table->table[i]; node != NULL; node = node->next) {
      ret = p_list_append(ret, node->value);
    }
  }
  return ret;
}

void
p_hash_table_free(htable_t *table) {
  bucket_t *node, *next_node;
  uint_t i;

  if (P_UNLIKELY (table == NULL)) {
    return;
  }
  for (i = 0; i < table->size; ++i) {
    for (node = table->table[i]; node != NULL;) {
      next_node = node->next;
      p_free(node);
      node = next_node;
    }
  }
  p_free(table->table);
  p_free(table);
}

void
p_hash_table_remove(htable_t *table, const_ptr_t key) {
  bucket_t *node, *prev_node;
  uint_t hash;

  if (P_UNLIKELY (table == NULL)) {
    return;
  }
  if (pp_hash_table_find_node(table, key) != NULL) {
    hash = pp_hash_table_calc_hash(key, table->size);
    node = table->table[hash];
    prev_node = NULL;
    while (node != NULL) {
      if (node->key == key) {
        if (prev_node == NULL) {
          table->table[hash] = node->next;
        } else {
          prev_node->next = node->next;
        }
        p_free(node);
        break;
      } else {
        prev_node = node;
        node = node->next;
      }
    }
  }
}

list_t *
p_hash_table_lookup_by_value(const htable_t *table, const_ptr_t val,
  cmp_fn_t func) {
  list_t *ret = NULL;
  bucket_t *node;
  uint_t i;
  bool res;

  if (P_UNLIKELY (table == NULL)) {
    return NULL;
  }
  for (i = 0; i < table->size; ++i) {
    for (node = table->table[i]; node != NULL; node = node->next) {
      if (func == NULL) {
        res = (node->value == val);
      } else {
        res = (func(node->value, val) == 0);
      }
      if (res) {
        ret = p_list_append(ret, node->key);
      }
    }
  }
  return ret;
}
