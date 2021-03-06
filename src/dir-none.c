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

#include "unic/dir.h"

struct dir {
  int hdl;
};

dir_t *
u_dir_new(const byte_t *path,
  err_t **error) {
  U_UNUSED (path);
  u_err_set_err_p(
    error,
    (int) U_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation"
  );
  return NULL;
}

bool
u_dir_create(const byte_t *path,
  int mode,
  err_t **error) {
  U_UNUSED (path);
  U_UNUSED (mode);
  u_err_set_err_p(
    error,
    (int) U_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation"
  );
  return false;
}

bool
u_dir_remove(const byte_t *path,
  err_t **error) {
  U_UNUSED (path);
  u_err_set_err_p(
    error,
    (int) U_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation"
  );
  return false;
}

bool
u_dir_is_exists(const byte_t *path) {
  U_UNUSED (path);
  return false;
}

byte_t *
u_dir_get_path(const dir_t *dir) {
  U_UNUSED (dir);
  return NULL;
}

dirent_t *
u_dir_get_next_entry(dir_t *dir,
  err_t **error) {
  U_UNUSED (dir);
  u_err_set_err_p(
    error,
    (int) U_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation"
  );
  return NULL;
}

bool
u_dir_rewind(dir_t *dir,
  err_t **error) {
  U_UNUSED (dir);
  u_err_set_err_p(
    error,
    (int) U_ERR_IO_NOT_IMPLEMENTED,
    0,
    "No directory implementation"
  );
  return false;
}

void
u_dir_free(dir_t *dir) {
  U_UNUSED (dir);
}
