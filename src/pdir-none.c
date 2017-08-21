/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "p/dir.h"

#include <stdlib.h>

struct p_dir {
  int_t hdl;
};

P_API p_dir_t *
p_dir_new(const byte_t *path,
  p_err_t **error) {
  P_UNUSED (path);

  p_error_set_error_p(error,
    (int_t) P_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation");

  return NULL;
}

P_API bool
p_dir_create(const byte_t *path,
  int_t mode,
  p_err_t **error) {
  P_UNUSED (path);
  P_UNUSED (mode);

  p_error_set_error_p(error,
    (int_t) P_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation");

  return false;
}

P_API bool
p_dir_remove(const byte_t *path,
  p_err_t **error) {
  P_UNUSED (path);

  p_error_set_error_p(error,
    (int_t) P_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation");

  return false;
}

P_API bool
p_dir_is_exists(const byte_t *path) {
  P_UNUSED (path);
  return false;
}

P_API byte_t *
p_dir_get_path(const p_dir_t *dir) {
  P_UNUSED (dir);
  return NULL;
}

P_API p_dirent_t *
p_dir_get_next_entry(p_dir_t *dir,
  p_err_t **error) {
  P_UNUSED (dir);

  p_error_set_error_p(error,
    (int_t) P_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation");

  return NULL;
}

P_API bool
p_dir_rewind(p_dir_t *dir,
  p_err_t **error) {
  P_UNUSED (dir);

  p_error_set_error_p(error,
    (int_t) P_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation");

  return false;
}

P_API void
p_dir_free(p_dir_t *dir) {
  P_UNUSED (dir);
}
