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
#include "p/uthread.h"
#include "puthread-private.h"

struct uthread {
  PUThreadBase base;
  int_t hdl;
};

void
p_uthread_init_internal(void) {
}

void
p_uthread_shutdown_internal(void) {
}

void
p_uthread_win32_thread_detach(void) {
}

uthread_t *
p_uthread_create_internal(uthread_fn_t func,
  bool joinable,
  uthread_prio_t prio,
  size_t stack_size) {
  uthread_t *ret;
  P_UNUSED (stack_size);
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(uthread_t))) == NULL)) {
    P_ERROR ("uthread_t::p_uthread_create_internal: failed to allocate memory");
    return NULL;
  }
  ret->hdl = -1;
  ret->base.joinable = joinable;
  ret->base.prio = prio;
  ret->base.func(ret);
  return ret;
}

void
p_uthread_exit_internal(void) {
}

void
p_uthread_wait_internal(uthread_t *thread) {
  P_UNUSED (thread);
}

void
p_uthread_free_internal(uthread_t *thread) {
  p_free(thread);
}

void
p_uthread_yield(void) {
}

bool
p_uthread_set_priority(uthread_t *thread,
  uthread_prio_t prio) {
  if (P_UNLIKELY (thread == NULL)) {
    return false;
  }
  thread->base.prio = prio;
  return false;
}

P_HANDLE
p_uthread_current_id(void) {
  return (P_HANDLE) 0;
}

uthread_key_t *
p_uthread_local_new(destroy_fn_t free_func) {
  P_UNUSED (free_func);
  return NULL;
}

void
p_uthread_local_free(uthread_key_t *key) {
  P_UNUSED (key);
}

ptr_t
p_uthread_get_local(uthread_key_t *key) {
  P_UNUSED (key);
}

void
p_uthread_set_local(uthread_key_t *key,
  ptr_t value) {
  P_UNUSED (key);
  P_UNUSED (value);
}

void
p_uthread_replace_local(uthread_key_t *key,
  ptr_t value) {
  P_UNUSED (key);
  P_UNUSED (value);
}
