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
#include "p/mem.h"
#include "p/string.h"
#include "perror-private.h"

#include <stdlib.h>
#include <string.h>

struct p_dir {
  WIN32_FIND_DATAA find_data;
  HANDLE search_handle;
  bool cached;
  byte_t path[MAX_PATH + 3];
  byte_t *orig_path;
};

P_API p_dir_t *
p_dir_new(const byte_t *path,
  p_err_t **error) {
  p_dir_t *ret;
  byte_t *pathp;

  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return NULL;
  }

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_dir_t))) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory structure");
    return NULL;
  }

  if (P_UNLIKELY (!GetFullPathNameA(path, MAX_PATH, ret->path, NULL))) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_io(),
      p_error_get_last_system(),
      "Failed to call GetFullPathNameA() to get directory path");
    p_free(ret);
    return NULL;
  }

  /* Append the search pattern "\\*\0" to the directory name */
  pathp = strchr(ret->path, '\0');

  if (ret->path < pathp && *(pathp - 1) != '\\' && *(pathp - 1) != ':')
    *pathp++ = '\\';

  *pathp++ = '*';
  *pathp = '\0';

  /* Open directory stream and retrieve the first entry */
  ret->search_handle = FindFirstFileA(ret->path, &ret->find_data);

  if (P_UNLIKELY (ret->search_handle == INVALID_HANDLE_VALUE)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_io(),
      p_error_get_last_system(),
      "Failed to call FindFirstFileA() to open directory stream");
    p_free(ret);
    return NULL;
  }

  ret->cached = true;
  ret->orig_path = p_strdup(path);

  return ret;
}

P_API bool
p_dir_create(const byte_t *path,
  int_t mode,
  p_err_t **error) {
  P_UNUSED (mode);

  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  if (p_dir_is_exists(path))
    return true;

  if (P_UNLIKELY (CreateDirectoryA(path, NULL) == 0)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_io(),
      p_error_get_last_system(),
      "Failed to call CreateDirectoryA() to create directory");
    return false;
  } else
    return true;
}

P_API bool
p_dir_remove(const byte_t *path,
  p_err_t **error) {
  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  if (!p_dir_is_exists(path)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_NOT_EXISTS,
      0,
      "Specified directory doesn't exist");
    return false;
  }

  if (P_UNLIKELY (RemoveDirectoryA(path) == 0)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_io(),
      p_error_get_last_system(),
      "Failed to call RemoveDirectoryA() to remove directory");
    return false;
  } else
    return true;
}

P_API bool
p_dir_is_exists(const byte_t *path) {
  DWORD dwAttrs;

  if (P_UNLIKELY (path == NULL))
    return false;

  dwAttrs = GetFileAttributesA(path);

  return (dwAttrs != INVALID_FILE_ATTRIBUTES)
    && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}

P_API byte_t *
p_dir_get_path(const p_dir_t *dir) {
  if (P_UNLIKELY (dir == NULL))
    return NULL;

  return p_strdup(dir->orig_path);
}

P_API p_dirent_t *
p_dir_get_next_entry(p_dir_t *dir,
  p_err_t **error) {
  p_dirent_t *ret;
  DWORD dwAttrs;

  if (P_UNLIKELY (dir == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return NULL;
  }

  if (dir->cached == true)
    dir->cached = false;
  else {
    if (P_UNLIKELY (dir->search_handle == INVALID_HANDLE_VALUE)) {
      p_error_set_error_p(error,
        (int_t) P_ERR_IO_INVALID_ARGUMENT,
        0,
        "Not a valid (or closed) directory stream");
      return NULL;
    }

    if (P_UNLIKELY (!FindNextFileA(dir->search_handle, &dir->find_data))) {
      p_error_set_error_p(error,
        (int_t) p_error_get_last_io(),
        p_error_get_last_system(),
        "Failed to call FindNextFileA() to read directory stream");
      FindClose(dir->search_handle);
      dir->search_handle = INVALID_HANDLE_VALUE;
      return NULL;
    }
  }

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(p_dirent_t))) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory entry");
    return NULL;
  }

  ret->name = p_strdup(dir->find_data.cFileName);

  dwAttrs = dir->find_data.dwFileAttributes;

  if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
    ret->type = P_DIR_ENTRY_TYPE_DIR;
  else if (dwAttrs & FILE_ATTRIBUTE_DEVICE)
    ret->type = P_DIR_ENTRY_TYPE_OTHER;
  else
    ret->type = P_DIR_ENTRY_TYPE_FILE;

  return ret;
}

P_API bool
p_dir_rewind(p_dir_t *dir,
  p_err_t **error) {
  if (P_UNLIKELY (dir == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  if (dir->search_handle != INVALID_HANDLE_VALUE) {
    if (P_UNLIKELY (FindClose(dir->search_handle) == 0)) {
      p_error_set_error_p(error,
        (int_t) p_error_get_last_io(),
        p_error_get_last_system(),
        "Failed to call FindClose() to close directory stream");
      return false;
    }
  }

  dir->search_handle = FindFirstFileA(dir->path, &dir->find_data);

  if (P_UNLIKELY (dir->search_handle == INVALID_HANDLE_VALUE)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_io(),
      p_error_get_last_system(),
      "Failed to call FindFirstFileA() to open directory stream");
    dir->cached = false;
    return false;
  } else {
    dir->cached = true;
    return true;
  }
}

P_API void
p_dir_free(p_dir_t *dir) {
  if (dir == NULL)
    return;

  if (P_LIKELY (dir->search_handle != INVALID_HANDLE_VALUE)) {
    if (P_UNLIKELY (!FindClose(dir->search_handle)))
      P_ERROR ("p_dir_t::p_dir_free: FindClose() failed");
  }

  p_free(dir->orig_path);
  p_free(dir);
}
