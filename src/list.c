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

#include "p/mem.h"
#include "p/list.h"

#include <stdlib.h>

P_API PList *
p_list_append(PList *list, ptr_t data) {
  PList *item, *cur;

  if (P_UNLIKELY ((item = p_malloc0(sizeof(PList))) == NULL)) {
    P_ERROR ("PList::p_list_append: failed to allocate memory");
    return list;
  }

  item->data = data;

  /* List is empty */
  if (P_UNLIKELY (list == NULL))
    return item;

  for (cur = list; cur->next != NULL; cur = cur->next);
  cur->next = item;

  return list;
}

P_API PList *
p_list_remove(PList *list, ptr_t data) {
  PList *cur, *prev, *head;

  if (P_UNLIKELY (list == NULL))
    return NULL;

  for (head = list, prev = NULL, cur = list; cur != NULL;
    prev = cur, cur = cur->next) {
    if (cur->data == data) {
      if (prev == NULL)
        head = cur->next;
      else
        prev->next = cur->next;

      p_free(cur);

      break;
    }
  }

  return head;
}

P_API void
p_list_foreach(PList *list, PFunc func, ptr_t user_data) {
  PList *cur;

  if (P_UNLIKELY (list == NULL || func == NULL))
    return;

  for (cur = list; cur != NULL; cur = cur->next)
    func(cur->data, user_data);
}

P_API void
p_list_free(PList *list) {
  PList *cur, *next;

  if (P_UNLIKELY (list == NULL))
    return;

  for (next = cur = list; cur != NULL && next != NULL; cur = next) {
    next = cur->next;
    p_free(cur);
  }
}

P_API PList *
p_list_last(PList *list) {
  PList *cur;

  if (P_UNLIKELY (list == NULL))
    return NULL;

  for (cur = list; cur->next != NULL; cur = cur->next);

  return cur;
}

P_API size_t
p_list_length(const PList *list) {
  const PList *cur;
  size_t ret;

  if (P_UNLIKELY (list == NULL))
    return 0;

  for (cur = list, ret = 1; cur->next != NULL; cur = cur->next, ++ret);

  return ret;
}

P_API PList *
p_list_prepend(PList *list, ptr_t data) {
  PList *item;

  if (P_UNLIKELY ((item = p_malloc0(sizeof(PList))) == NULL)) {
    P_ERROR ("PList::p_list_prepend: failed to allocate memory");
    return list;
  }

  item->data = data;

  /* List is empty */
  if (P_UNLIKELY (list == NULL))
    return item;

  item->next = list;

  return item;
}

P_API PList *
p_list_reverse(PList *list) {
  PList *prev, *cur, *tmp;

  if (P_UNLIKELY (list == NULL))
    return NULL;

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