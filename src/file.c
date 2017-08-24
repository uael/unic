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

#include "unic/err.h"
#include "unic/file.h"
#include "err-private.h"

#ifndef U_OS_WIN
# include <unistd.h>
#endif

bool
u_file_is_exists(const byte_t *file) {
#ifdef U_OS_WIN
  DWORD attrs;
#endif

  if (U_UNLIKELY (file == NULL)) {
    return false;
  }
#ifdef U_OS_WIN
  attrs = GetFileAttributesA((LPCSTR) file);
  return (
    attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0
  );
#else
  return access(file, F_OK) == 0;
#endif
}

bool
u_file_remove(const byte_t *file, err_t **error) {
  bool result;

  if (U_UNLIKELY (file == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
#ifdef U_OS_WIN
  result = (bool) (DeleteFileA((LPCSTR) file) != 0);
#else
  result = (bool) (unlink(file) == 0);
#endif
  if (U_UNLIKELY (!result)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to remove file"
    );
  }
  return result ? true : false;
}
