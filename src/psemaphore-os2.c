/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "p/sema.h"

struct sema {
  int_t hdl;
};

sema_t *
p_sema_new(const byte_t *name,
  int_t init_val,
  sema_access_t mode,
  err_t **error) {
  P_UNUSED (name);
  P_UNUSED (init_val);
  P_UNUSED (mode);
  p_error_set_error_p(
    error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return NULL;
}

void
p_sema_take_ownership(sema_t *sem) {
  P_UNUSED (sem);
}

bool
p_sema_acquire(sema_t *sem,
  err_t **error) {
  P_UNUSED (sem);
  p_error_set_error_p(
    error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return false;
}

bool
p_sema_release(sema_t *sem,
  err_t **error) {
  P_UNUSED (sem);
  p_error_set_error_p(
    error,
    (int_t) P_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return false;
}

void
p_sema_free(sema_t *sem) {
  P_UNUSED (sem);
}

