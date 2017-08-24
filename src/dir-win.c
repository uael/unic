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
#include "unic/mem.h"
#include "unic/string.h"
#include "err-private.h"

struct dir {
  WIN32_FIND_DATAA find_data;
  HANDLE search_handle;
  bool cached;
  byte_t path[MAX_PATH + 3];
  byte_t *orig_path;
};

dir_t *
u_dir_new(const byte_t *path, err_t **error) {
  dir_t *ret;
  byte_t *pathp;
  if (U_UNLIKELY (path == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(dir_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory structure"
    );
    return NULL;
  }
  if (U_UNLIKELY (!GetFullPathNameA(path, MAX_PATH, ret->path, NULL))) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call GetFullPathNameA() to get directory path"
    );
    u_free(ret);
    return NULL;
  }

  /* Append the search pattern "\\*\0" to the directory name */
  pathp = strchr(ret->path, '\0');
  if (ret->path < pathp && *(pathp - 1) != '\\' && *(pathp - 1) != ':') {
    *pathp++ = '\\';
  }
  *pathp++ = '*';
  *pathp = '\0';

  /* Open directory stream and retrieve the first entry */
  ret->search_handle = FindFirstFileA(ret->path, &ret->find_data);
  if (U_UNLIKELY (ret->search_handle == INVALID_HANDLE_VALUE)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call FindFirstFileA() to open directory stream"
    );
    u_free(ret);
    return NULL;
  }
  ret->cached = true;
  ret->orig_path = u_strdup(path);
  return ret;
}

bool
u_dir_create(const byte_t *path,
  int mode,
  err_t **error) {
  U_UNUSED (mode);
  if (U_UNLIKELY (path == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (u_dir_is_exists(path)) {
    return true;
  }
  if (U_UNLIKELY (CreateDirectoryA(path, NULL) == 0)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call CreateDirectoryA() to create directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
u_dir_remove(const byte_t *path,
  err_t **error) {
  if (U_UNLIKELY (path == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (!u_dir_is_exists(path)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_NOT_EXISTS,
      0,
      "Specified directory doesn't exist"
    );
    return false;
  }
  if (U_UNLIKELY (RemoveDirectoryA(path) == 0)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call RemoveDirectoryA() to remove directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
u_dir_is_exists(const byte_t *path) {
  DWORD dwAttrs;
  if (U_UNLIKELY (path == NULL)) {
    return false;
  }
  dwAttrs = GetFileAttributesA(path);
  return (dwAttrs != INVALID_FILE_ATTRIBUTES)
    && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}

byte_t *
u_dir_get_path(const dir_t *dir) {
  if (U_UNLIKELY (dir == NULL)) {
    return NULL;
  }
  return u_strdup(dir->orig_path);
}

dirent_t *
u_dir_get_next_entry(dir_t *dir,
  err_t **error) {
  dirent_t *ret;
  DWORD dwAttrs;
  if (U_UNLIKELY (dir == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (dir->cached == true) {
    dir->cached = false;
  } else {
    if (U_UNLIKELY (dir->search_handle == INVALID_HANDLE_VALUE)) {
      u_err_set_err_p(
        error,
        (int) U_ERR_IO_INVALID_ARGUMENT,
        0,
        "Not a valid (or closed) directory stream"
      );
      return NULL;
    }
    if (U_UNLIKELY (!FindNextFileA(dir->search_handle, &dir->find_data))) {
      u_err_set_err_p(
        error,
        (int) u_err_get_last_io(),
        u_err_get_last_system(),
        "Failed to call FindNextFileA() to read directory stream"
      );
      FindClose(dir->search_handle);
      dir->search_handle = INVALID_HANDLE_VALUE;
      return NULL;
    }
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(dirent_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory entry"
    );
    return NULL;
  }
  ret->name = u_strdup(dir->find_data.cFileName);
  dwAttrs = dir->find_data.dwFileAttributes;
  if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
    ret->type = U_DIRENT_DIR;
  } else if (dwAttrs & FILE_ATTRIBUTE_DEVICE) {
    ret->type = U_DIRENT_OTHER;
  } else {
    ret->type = U_DIRENT_FILE;
  }
  return ret;
}

bool
u_dir_rewind(dir_t *dir,
  err_t **error) {
  if (U_UNLIKELY (dir == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (dir->search_handle != INVALID_HANDLE_VALUE) {
    if (U_UNLIKELY (FindClose(dir->search_handle) == 0)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_last_io(),
        u_err_get_last_system(),
        "Failed to call FindClose() to close directory stream"
      );
      return false;
    }
  }
  dir->search_handle = FindFirstFileA(dir->path, &dir->find_data);
  if (U_UNLIKELY (dir->search_handle == INVALID_HANDLE_VALUE)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call FindFirstFileA() to open directory stream"
    );
    dir->cached = false;
    return false;
  } else {
    dir->cached = true;
    return true;
  }
}

void
u_dir_free(dir_t *dir) {
  if (dir == NULL) {
    return;
  }
  if (U_LIKELY (dir->search_handle != INVALID_HANDLE_VALUE)) {
    if (U_UNLIKELY (!FindClose(dir->search_handle)))
      U_ERROR ("dir_t::u_dir_free: FindClose() failed");
  }
  u_free(dir->orig_path);
  u_free(dir);
}
