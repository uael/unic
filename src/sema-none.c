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

#include "unic/sema.h"

struct sema {
  int hdl;
};

sema_t *
u_sema_new(const byte_t *name,
  int init_val,
  sema_access_t mode,
  err_t **error) {
  U_UNUSED (name);
  U_UNUSED (init_val);
  U_UNUSED (mode);
  u_err_set_err_p(
    error,
    (int) U_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return NULL;
}

void
u_sema_take_ownership(sema_t *sem) {
  U_UNUSED (sem);
}

bool
u_sema_acquire(sema_t *sem,
  err_t **error) {
  U_UNUSED (sem);
  u_err_set_err_p(
    error,
    (int) U_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return false;
}

bool
u_sema_release(sema_t *sem,
  err_t **error) {
  U_UNUSED (sem);
  u_err_set_err_p(
    error,
    (int) U_ERR_IPC_NOT_IMPLEMENTED,
    0,
    "No semaphore implementation"
  );
  return false;
}

void
u_sema_free(sema_t *sem) {
  U_UNUSED (sem);
}

