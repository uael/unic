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

#include <stdlib.h>

struct PShm_ {
  int_t hdl;
};

P_API PShm *
p_shm_new(const byte_t *name,
  size_t size,
  PShmAccessPerms perms,
  p_err_t **error) {
  P_UNUSED (name);
  P_UNUSED (size);
  P_UNUSED (perms);
  P_UNUSED (error);

  return NULL;
}

P_API void
p_shm_take_ownership(PShm *shm) {
  P_UNUSED (shm);
}

P_API void
p_shm_free(PShm *shm) {
  P_UNUSED (shm);
}

P_API bool
p_shm_lock(PShm *shm,
  p_err_t **error) {
  P_UNUSED (shm);
  P_UNUSED (error);

  return false;
}

P_API bool
p_shm_unlock(PShm *shm,
  p_err_t **error) {
  P_UNUSED (shm);
  P_UNUSED (error);

  return false;
}

P_API ptr_t
p_shm_get_address(const PShm *shm) {
  P_UNUSED (shm);

  return NULL;
}

P_API size_t
p_shm_get_size(const PShm *shm) {
  P_UNUSED (shm);

  return 0;
}
