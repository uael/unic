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

#include "p/sem.h"

#include <stdlib.h>

struct PSemaphore_ {
  int_t hdl;
};

P_API PSemaphore *
p_semaphore_new(const byte_t *name,
  int_t init_val,
  PSemaphoreAccessMode mode,
  p_err_t **error) {
  P_UNUSED (name);
  P_UNUSED (init_val);
  P_UNUSED (mode);

  p_error_set_error_p(error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation");

  return NULL;
}

P_API void
p_semaphore_take_ownership(PSemaphore *sem) {
  P_UNUSED (sem);
}

P_API bool
p_semaphore_acquire(PSemaphore *sem,
  p_err_t **error) {
  P_UNUSED (sem);

  p_error_set_error_p(error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation");

  return false;
}

P_API bool
p_semaphore_release(PSemaphore *sem,
  p_err_t **error) {
  P_UNUSED (sem);

  p_error_set_error_p(error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation");

  return false;
}

P_API void
p_semaphore_free(PSemaphore *sem) {
  P_UNUSED (sem);
}

