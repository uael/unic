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

#include "p/shm.h"

struct shm {
  int_t hdl;
};

shm_t *
p_shm_new(const byte_t *name,
  size_t size,
  shm_access_t perms,
  err_t **error) {
  P_UNUSED (name);
  P_UNUSED (size);
  P_UNUSED (perms);
  P_UNUSED (error);
  return NULL;
}

void
p_shm_take_ownership(shm_t *shm) {
  P_UNUSED (shm);
}

void
p_shm_free(shm_t *shm) {
  P_UNUSED (shm);
}

bool
p_shm_lock(shm_t *shm,
  err_t **error) {
  P_UNUSED (shm);
  P_UNUSED (error);
  return false;
}

bool
p_shm_unlock(shm_t *shm,
  err_t **error) {
  P_UNUSED (shm);
  P_UNUSED (error);
  return false;
}

ptr_t
p_shm_get_address(const shm_t *shm) {
  P_UNUSED (shm);
  return NULL;
}

size_t
p_shm_get_size(const shm_t *shm) {
  P_UNUSED (shm);
  return 0;
}
