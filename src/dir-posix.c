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

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "unic/dir.h"
#include "unic/mem.h"
#include "unic/string.h"
#include "err-private.h"

#if defined(U_OS_SOLARIS) || defined(U_OS_QNX6) || defined(U_OS_UNIXWARE) || defined(U_OS_SCO) || \
    defined(U_OS_IRIX) || defined(U_OS_HAIKU)
# define U_DIR_NEED_BUF_ALLOC 1
#endif
#ifdef U_DIR_NEED_BUF_ALLOC
# if defined(U_OS_SCO)
#   define U_DIR_NEED_SIMPLE_R 1
# endif
#else
# if defined(U_OS_BEOS)
#   define U_DIR_NON_REENTRANT 1
# endif
#endif
struct dir {
  DIR *dir;
  struct dirent *dir_result;
  byte_t *path;
  byte_t *orig_path;
};

dir_t *
u_dir_new(const byte_t *path,
  err_t **error) {
  dir_t *ret;
  DIR *dir;
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
  if (U_UNLIKELY ((dir = opendir(path)) == NULL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call opendir() to open directory stream"
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
    closedir(dir);
    return NULL;
  }
  ret->dir = dir;
  ret->path = u_strdup(path);
  ret->orig_path = u_strdup(path);
  pathp = ret->path + strlen(ret->path) - 1;
  if (*pathp == '/' || *pathp == '\\') {
    *pathp = '\0';
  }
  return ret;
}

bool
u_dir_create(const byte_t *path,
  int mode,
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
  if (u_dir_is_exists(path)) {
    return true;
  }
  if (U_UNLIKELY (mkdir(path, (mode_t) mode) != 0)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call mkdir() to create directory"
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
  if (U_UNLIKELY (rmdir(path) != 0)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call rmdir() to remove directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
u_dir_is_exists(const byte_t *path) {
  struct stat sb;
  if (U_UNLIKELY (path == NULL)) {
    return false;
  }
  return (stat(path, &sb) == 0 && S_ISDIR (sb.st_mode)) ? true : false;
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
#ifdef U_DIR_NEED_BUF_ALLOC
  struct u_dirent *dirent_st;
#elif !defined(U_DIR_NON_REENTRANT)
  struct dirent dirent_st;
#endif
  struct stat sb;
  byte_t *entry_path;
  size_t path_len;
#ifdef U_DIR_NEED_BUF_ALLOC
  int  name_max;
#endif
  if (U_UNLIKELY (dir == NULL || dir->dir == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
#if defined(U_OS_SOLARIS)
  name_max = (int) (FILENAME_MAX);
#elif defined(U_OS_SCO) || defined(U_OS_IRIX)
  name_max = (int) pathconf (dir->orig_path, _PC_NAME_MAX);

  if (name_max == -1) {
    if (u_err_get_last_system () == 0)
      name_max = _POSIX_PATH_MAX;
    else {
      u_err_set_err_p (error,
               (int) U_ERR_IO_FAILED,
               0,
               "Failed to get NAME_MAX using pathconf()");
      return NULL;
    }
  }
#elif defined(U_OS_QNX6) || defined(U_OS_UNIXWARE) || defined(U_OS_HAIKU)
  name_max = (int) (NAME_MAX);
#endif
#ifdef U_DIR_NEED_BUF_ALLOC
  if (U_UNLIKELY ((dirent_st = u_malloc0 (sizeof (struct u_dirent) + name_max + 1)) == NULL)) {
    u_err_set_err_p (error,
             (int) U_ERR_IO_NO_RESOURCES,
             0,
             "Failed to allocate memory for internal directory entry");
    return NULL;
  }

# ifdef U_DIR_NEED_SIMPLE_R
  u_err_set_last_system (0);

  if ((dir->dir_result = readdir_r (dir->dir, dirent_st)) == NULL) {
    if (U_UNLIKELY (u_err_get_last_system () != 0)) {
      u_err_set_err_p (error,
               (int) u_err_get_last_io (),
               u_err_get_last_system (),
               "Failed to call readdir_r() to read directory stream");
      u_free (dirent_st);
      return NULL;
    }
  }
# else
  if (U_UNLIKELY (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0)) {
    u_err_set_err_p (error,
             (int) u_err_get_last_io (),
             u_err_get_last_system (),
             "Failed to call readdir_r() to read directory stream");
    u_free (dirent_st);
    return NULL;
  }
# endif
#else
# ifdef U_DIR_NON_REENTRANT
  u_err_set_last_system (0);

  if ((dir->dir_result = readdir (dir->dir)) == NULL) {
    if (U_UNLIKELY (u_err_get_last_system () != 0)) {
      u_err_set_err_p (error,
               (int) u_err_get_last_io (),
               u_err_get_last_system (),
               "Failed to call readdir() to read directory stream");
      return NULL;
    }
  }
# else
  if (U_UNLIKELY (readdir_r(dir->dir, &dirent_st, &dir->dir_result) != 0)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_io(),
      u_err_get_last_system(),
      "Failed to call readdir_r() to read directory stream"
    );
    return NULL;
  }
# endif
#endif
  if (dir->dir_result == NULL) {
#ifdef U_DIR_NEED_BUF_ALLOC
    u_free (dirent_st);
#endif
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(dirent_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory entry"
    );
#ifdef U_DIR_NEED_BUF_ALLOC
    u_free (dirent_st);
#endif
    return NULL;
  }
#ifdef U_DIR_NEED_BUF_ALLOC
  ret->name = u_strdup (dirent_st->d_name);
  u_free (dirent_st);
#else
# ifdef U_DIR_NON_REENTRANT
  ret->name = u_strdup (dir->dir_result->d_name);
# else
  ret->name = u_strdup(dirent_st.d_name);
# endif
#endif
  path_len = strlen(dir->path);
  if (U_UNLIKELY (
    (entry_path = u_malloc0(path_len + strlen(ret->name) + 2)) == NULL)) {
    U_WARNING (
      "dir_t::u_dir_get_next_entry: failed to allocate memory for stat()");
    ret->type = U_DIRENT_OTHER;
    return ret;
  }
  strcat(entry_path, dir->path);
  *(entry_path + path_len) = '/';
  strcat(entry_path + path_len + 1, ret->name);
  if (U_UNLIKELY (stat(entry_path, &sb) != 0)) {
    U_WARNING ("dir_t::u_dir_get_next_entry: stat() failed");
    ret->type = U_DIRENT_OTHER;
    u_free(entry_path);
    return ret;
  }
  u_free(entry_path);
  if (S_ISDIR (sb.st_mode)) {
    ret->type = U_DIRENT_DIR;
  } else if (S_ISREG (sb.st_mode)) {
    ret->type = U_DIRENT_FILE;
  } else {
    ret->type = U_DIRENT_OTHER;
  }
  return ret;
}

bool
u_dir_rewind(dir_t *dir,
  err_t **error) {
  if (U_UNLIKELY (dir == NULL || dir->dir == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  rewinddir(dir->dir);
  return true;
}

void
u_dir_free(dir_t *dir) {
  if (U_UNLIKELY (dir == NULL)) {
    return;
  }
  if (U_LIKELY (dir->dir != NULL)) {
    if (U_UNLIKELY (closedir(dir->dir) != 0))
      U_ERROR ("dir_t::u_dir_free: closedir() failed");
  }
  u_free(dir->path);
  u_free(dir->orig_path);
  u_free(dir);
}
