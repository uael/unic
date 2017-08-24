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

#include "unic/mem.h"
#include "unic/list.h"

list_t *
u_list_append(list_t *list, ptr_t data) {
  list_t *item, *cur;
  if (U_UNLIKELY ((item = u_malloc0(sizeof(list_t))) == NULL)) {
    U_ERROR ("list_t::u_list_append: failed to allocate memory");
    return list;
  }
  item->data = data;

  /* List is empty */
  if (U_UNLIKELY (list == NULL)) {
    return item;
  }
  for (cur = list; cur->next != NULL; cur = cur->next);
  cur->next = item;
  return list;
}

list_t *
u_list_remove(list_t *list, ptr_t data) {
  list_t *cur, *prev, *head;
  if (U_UNLIKELY (list == NULL)) {
    return NULL;
  }
  for (head = list, prev = NULL, cur = list; cur != NULL;
    prev = cur, cur = cur->next) {
    if (cur->data == data) {
      if (prev == NULL) {
        head = cur->next;
      } else {
        prev->next = cur->next;
      }
      u_free(cur);
      break;
    }
  }
  return head;
}

void
u_list_foreach(list_t *list, fn_t func, ptr_t user_data) {
  list_t *cur;
  if (U_UNLIKELY (list == NULL || func == NULL)) {
    return;
  }
  for (cur = list; cur != NULL; cur = cur->next) {
    func(cur->data, user_data);
  }
}

void
u_list_free(list_t *list) {
  list_t *cur, *next;
  if (U_UNLIKELY (list == NULL)) {
    return;
  }
  for (next = cur = list; cur != NULL && next != NULL; cur = next) {
    next = cur->next;
    u_free(cur);
  }
}

list_t *
u_list_last(list_t *list) {
  list_t *cur;
  if (U_UNLIKELY (list == NULL)) {
    return NULL;
  }
  for (cur = list; cur->next != NULL; cur = cur->next);
  return cur;
}

size_t
u_list_length(const list_t *list) {
  const list_t *cur;
  size_t ret;
  if (U_UNLIKELY (list == NULL)) {
    return 0;
  }
  for (cur = list, ret = 1; cur->next != NULL; cur = cur->next, ++ret);
  return ret;
}

list_t *
u_list_prepend(list_t *list, ptr_t data) {
  list_t *item;
  if (U_UNLIKELY ((item = u_malloc0(sizeof(list_t))) == NULL)) {
    U_ERROR ("list_t::u_list_prepend: failed to allocate memory");
    return list;
  }
  item->data = data;

  /* List is empty */
  if (U_UNLIKELY (list == NULL)) {
    return item;
  }
  item->next = list;
  return item;
}

list_t *
u_list_reverse(list_t *list) {
  list_t *prev, *cur, *tmp;
  if (U_UNLIKELY (list == NULL)) {
    return NULL;
  }
  prev = list;
  cur = list->next;
  prev->next = NULL;
  while (cur != NULL) {
    tmp = cur->next;
    cur->next = prev;
    prev = cur;
    cur = tmp;
  }
  return prev;
}
