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
#include "unic/thread.h"
#include "thread-private.h"

struct thread {
  thread_base_t base;
  int hdl;
};

void
u_thread_init_internal(void) {
}

void
u_thread_shutdown_internal(void) {
}

void
u_thread_win32_thread_detach(void) {
}

thread_t *
u_thread_create_internal(thread_fn_t func,
  bool joinable,
  thread_prio_t prio,
  size_t stack_size) {
  thread_t *ret;
  U_UNUSED (stack_size);
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(thread_t))) == NULL)) {
    U_ERROR ("thread_t::u_thread_create_internal: failed to allocate memory");
    return NULL;
  }
  ret->hdl = -1;
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  ret->base.func(ret);
  return ret;
}

void
u_thread_exit_internal(void) {
}

void
u_thread_wait_internal(thread_t *thread) {
  U_UNUSED (thread);
}

void
u_thread_free_internal(thread_t *thread) {
  u_free(thread);
}

void
u_thread_yield(void) {
}

bool
u_thread_set_priority(thread_t *thread,
  thread_prio_t prio) {
  if (U_UNLIKELY (thread == NULL)) {
    return false;
  }
  thread->base.prio = prio;
  return false;
}

U_HANDLE
u_thread_current_id(void) {
  return (U_HANDLE) 0;
}

thread_key_t *
u_thread_local_new(destroy_fn_t free_func) {
  U_UNUSED (free_func);
  return NULL;
}

void
u_thread_local_free(thread_key_t *key) {
  U_UNUSED (key);
}

ptr_t
u_thread_get_local(thread_key_t *key) {
  U_UNUSED (key);
}

void
u_thread_set_local(thread_key_t *key,
  ptr_t value) {
  U_UNUSED (key);
  U_UNUSED (value);
}

void
u_thread_replace_local(thread_key_t *key,
  ptr_t value) {
  U_UNUSED (key);
  U_UNUSED (value);
}
