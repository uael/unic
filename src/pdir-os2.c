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

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include "p/dir.h"
#include "p/mem.h"
#include "p/string.h"
#include "perror-private.h"

struct dir {
  FILEFINDBUF3 find_data;
  HDIR search_handle;
  bool cached;
  byte_t path[CCHMAXPATH];
  byte_t *orig_path;
};

dir_t *
p_dir_new(const byte_t *path,
  err_t **error) {
  dir_t *ret;
  byte_t *pathp;
  byte_t *adj_path;
  int_t path_len;
  APIRET ulrc;
  ULONG find_count;
  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(dir_t))) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory structure"
    );
    return NULL;
  }
  adj_path = NULL;
  path_len = strlen(path);
  if (P_UNLIKELY (path[path_len - 1] == '\\' || path[path_len - 1] == '/')
    && path_len > 1) {
    while ((path[path_len - 1] == '\\' || path[path_len - 1] == '/')
      && path_len > 1) {
        --path_len;
    }
    if (P_UNLIKELY ((adj_path = p_malloc0(path_len + 1)) == NULL)) {
      p_free(ret);
      p_error_set_error_p(
        error,
        (int_t) P_ERR_IO_NO_RESOURCES,
        0,
        "Failed to allocate memory for directory path"
      );
      return NULL;
    }
    memcpy(adj_path, path, path_len);
    adj_path[path_len] = '\0';
    ret->orig_path = p_strdup(path);
    path = (const byte_t *) adj_path;
  }
  ret->search_handle = HDIR_CREATE;
  ulrc = DosQueryPathInfo((PSZ) path, FIL_QUERYFULLNAME, ret->path,
    sizeof(ret->path) - 2
  );
  if (P_UNLIKELY (ulrc != NO_ERROR)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_io_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosQueryPathInfo() to get directory path"
    );
    if (P_UNLIKELY (adj_path != NULL)) {
      p_free(adj_path);
      p_free(ret->orig_path);
    }
    p_free(ret);
    return NULL;
  }

  /* Append the search pattern "\\*\0" to the directory name */
  pathp = strchr(ret->path, '\0');
  if (ret->path < pathp && *(pathp - 1) != '\\' && *(pathp - 1) != ':') {
    *pathp++ = '\\';
  }
  *pathp++ = '*';
  *pathp = '\0';
  find_count = 1;

  /* Open directory stream and retrieve the first entry */
  ulrc = DosFindFirst(
    ret->path,
    &ret->search_handle,
    FILE_NORMAL | FILE_DIRECTORY,
    &ret->find_data,
    sizeof(ret->find_data),
    &find_count,
    FIL_STANDARD
  );
  if (P_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_NO_MORE_FILES)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_io_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosFindFirst() to open directory stream"
    );
    if (P_UNLIKELY (adj_path != NULL)) {
      p_free(adj_path);
      p_free(ret->orig_path);
    }
    p_free(ret);
    return NULL;
  }
  ret->cached = true;
  if (P_UNLIKELY (adj_path != NULL)) {
    p_free(adj_path);
  } else {
    ret->orig_path = p_strdup(path);
  }
  return ret;
}

bool
p_dir_create(const byte_t *path,
  int_t mode,
  err_t **error) {
  APIRET ulrc;
  P_UNUSED (mode);
  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (p_dir_is_exists(path)) {
    return true;
  }
  if (P_UNLIKELY ((ulrc = DosCreateDir((PSZ) path, NULL)) != NO_ERROR)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_io_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosCreateDir() to create directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
p_dir_remove(const byte_t *path,
  err_t **error) {
  APIRET ulrc;
  if (P_UNLIKELY (path == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (!p_dir_is_exists(path)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_NOT_EXISTS,
      0,
      "Specified directory doesn't exist"
    );
    return false;
  }
  if (P_UNLIKELY ((ulrc = DosDeleteDir((PSZ) path)) != NO_ERROR)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_io_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosDeleteDir() to remove directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
p_dir_is_exists(const byte_t *path) {
  FILESTATUS3 status;
  if (P_UNLIKELY (path == NULL)) {
    return false;
  }
  if (
    DosQueryPathInfo((PSZ) path, FIL_STANDARD, (PVOID) &status, sizeof(status))
      != NO_ERROR) {
        return false;
  }
  return (status.attrFile & FILE_DIRECTORY) != 0;
}

byte_t *
p_dir_get_path(const dir_t *dir) {
  if (P_UNLIKELY (dir == NULL)) {
    return NULL;
  }
  return p_strdup(dir->orig_path);
}

dirent_t *
p_dir_get_next_entry(dir_t *dir,
  err_t **error) {
  dirent_t *ret;
  APIRET ulrc;
  ULONG find_count;
  if (P_UNLIKELY (dir == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (dir->cached == true) {
    dir->cached = false;

    /* Opened directory is empty */
    if (P_UNLIKELY (dir->search_handle == HDIR_CREATE)) {
      p_error_set_error_p(
        error,
        (int_t) P_ERR_IO_NO_MORE,
        (int_t) ERROR_NO_MORE_FILES,
        "Directory is empty to get the next entry"
      );
      return NULL;
    }
  } else {
    if (P_UNLIKELY (dir->search_handle == HDIR_CREATE)) {
      p_error_set_error_p(
        error,
        (int_t) P_ERR_IO_INVALID_ARGUMENT,
        0,
        "Not a valid (or closed) directory stream"
      );
      return NULL;
    }
    find_count = 1;
    ulrc = DosFindNext(
      dir->search_handle,
      (PVOID) &dir->find_data,
      sizeof(dir->find_data),
      &find_count
    );
    if (P_UNLIKELY (ulrc != NO_ERROR)) {
      p_error_set_error_p(
        error,
        (int_t) p_error_get_io_from_system((int_t) ulrc),
        (int_t) ulrc,
        "Failed to call DosFindNext() to read directory stream"
      );
      DosFindClose(dir->search_handle);
      dir->search_handle = HDIR_CREATE;
      return NULL;
    }
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(dirent_t))) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory entry"
    );
    return NULL;
  }
  ret->name = p_strdup(dir->find_data.achName);
  if ((dir->find_data.attrFile & FILE_DIRECTORY) != 0) {
    ret->type = P_DIRENT_DIR;
  } else {
    ret->type = P_DIRENT_FILE;
  }
  return ret;
}

bool
p_dir_rewind(dir_t *dir,
  err_t **error) {
  APIRET ulrc;
  ULONG find_count;
  if (P_UNLIKELY (dir == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (dir->search_handle != HDIR_CREATE) {
    if (P_UNLIKELY ((ulrc = DosFindClose(dir->search_handle)) != NO_ERROR)) {
      p_error_set_error_p(
        error,
        (int_t) p_error_get_io_from_system((int_t) ulrc),
        (int_t) ulrc,
        "Failed to call DosFindClose() to close directory stream"
      );
      return false;
    }
    dir->search_handle = HDIR_CREATE;
  }
  find_count = 1;
  ulrc = DosFindFirst(
    dir->path,
    &dir->search_handle,
    FILE_NORMAL | FILE_DIRECTORY,
    &dir->find_data,
    sizeof(dir->find_data),
    &find_count,
    FIL_STANDARD
  );
  if (P_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_NO_MORE_FILES)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_io_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosFindFirst() to open directory stream"
    );
    dir->cached = false;
    return false;
  } else {
    dir->cached = true;
    return true;
  }
}

void
p_dir_free(dir_t *dir) {
  if (dir == NULL) {
    return;
  }
  if (P_LIKELY (dir->search_handle != HDIR_CREATE)) {
    if (P_UNLIKELY (DosFindClose(dir->search_handle) != NO_ERROR))
      P_ERROR ("dir_t::p_dir_free: DosFindClose() failed");
  }
  p_free(dir->orig_path);
  p_free(dir);
}
