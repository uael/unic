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

#include "p/dir.h"
#include "p/mem.h"
#include "p/string.h"
#include "err-private.h"

#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6) || defined(P_OS_UNIXWARE) || defined(P_OS_SCO) || \
    defined(P_OS_IRIX) || defined(P_OS_HAIKU)
# define P_DIR_NEED_BUF_ALLOC 1
#endif
#ifdef P_DIR_NEED_BUF_ALLOC
# if defined(P_OS_SCO)
#   define P_DIR_NEED_SIMPLE_R 1
# endif
#else
# if defined(P_OS_BEOS)
#   define P_DIR_NON_REENTRANT 1
# endif
#endif
struct dir {
  DIR *dir;
  struct dirent *dir_result;
  byte_t *path;
  byte_t *orig_path;
};

dir_t *
p_dir_new(const byte_t *path,
  err_t **error) {
  dir_t *ret;
  DIR *dir;
  byte_t *pathp;
  if (P_UNLIKELY (path == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY ((dir = opendir(path)) == NULL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call opendir() to open directory stream"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(dir_t))) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory structure"
    );
    closedir(dir);
    return NULL;
  }
  ret->dir = dir;
  ret->path = p_strdup(path);
  ret->orig_path = p_strdup(path);
  pathp = ret->path + strlen(ret->path) - 1;
  if (*pathp == '/' || *pathp == '\\') {
    *pathp = '\0';
  }
  return ret;
}

bool
p_dir_create(const byte_t *path,
  int mode,
  err_t **error) {
  if (P_UNLIKELY (path == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (p_dir_is_exists(path)) {
    return true;
  }
  if (P_UNLIKELY (mkdir(path, (mode_t) mode) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call mkdir() to create directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
p_dir_remove(const byte_t *path,
  err_t **error) {
  if (P_UNLIKELY (path == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (!p_dir_is_exists(path)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NOT_EXISTS,
      0,
      "Specified directory doesn't exist"
    );
    return false;
  }
  if (P_UNLIKELY (rmdir(path) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call rmdir() to remove directory"
    );
    return false;
  } else {
    return true;
  }
}

bool
p_dir_is_exists(const byte_t *path) {
  struct stat sb;
  if (P_UNLIKELY (path == NULL)) {
    return false;
  }
  return (stat(path, &sb) == 0 && S_ISDIR (sb.st_mode)) ? true : false;
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
#ifdef P_DIR_NEED_BUF_ALLOC
  struct p_dirent *dirent_st;
#elif !defined(P_DIR_NON_REENTRANT)
  struct dirent dirent_st;
#endif
  struct stat sb;
  byte_t *entry_path;
  size_t path_len;
#ifdef P_DIR_NEED_BUF_ALLOC
  int  name_max;
#endif
  if (P_UNLIKELY (dir == NULL || dir->dir == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
#if defined(P_OS_SOLARIS)
  name_max = (int) (FILENAME_MAX);
#elif defined(P_OS_SCO) || defined(P_OS_IRIX)
  name_max = (int) pathconf (dir->orig_path, _PC_NAME_MAX);

  if (name_max == -1) {
    if (p_err_get_last_system () == 0)
      name_max = _POSIX_PATH_MAX;
    else {
      p_err_set_err_p (error,
               (int) P_ERR_IO_FAILED,
               0,
               "Failed to get NAME_MAX using pathconf()");
      return NULL;
    }
  }
#elif defined(P_OS_QNX6) || defined(P_OS_UNIXWARE) || defined(P_OS_HAIKU)
  name_max = (int) (NAME_MAX);
#endif
#ifdef P_DIR_NEED_BUF_ALLOC
  if (P_UNLIKELY ((dirent_st = p_malloc0 (sizeof (struct p_dirent) + name_max + 1)) == NULL)) {
    p_err_set_err_p (error,
             (int) P_ERR_IO_NO_RESOURCES,
             0,
             "Failed to allocate memory for internal directory entry");
    return NULL;
  }

# ifdef P_DIR_NEED_SIMPLE_R
  p_err_set_last_system (0);

  if ((dir->dir_result = readdir_r (dir->dir, dirent_st)) == NULL) {
    if (P_UNLIKELY (p_err_get_last_system () != 0)) {
      p_err_set_err_p (error,
               (int) p_err_get_last_io (),
               p_err_get_last_system (),
               "Failed to call readdir_r() to read directory stream");
      p_free (dirent_st);
      return NULL;
    }
  }
# else
  if (P_UNLIKELY (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0)) {
    p_err_set_err_p (error,
             (int) p_err_get_last_io (),
             p_err_get_last_system (),
             "Failed to call readdir_r() to read directory stream");
    p_free (dirent_st);
    return NULL;
  }
# endif
#else
# ifdef P_DIR_NON_REENTRANT
  p_err_set_last_system (0);

  if ((dir->dir_result = readdir (dir->dir)) == NULL) {
    if (P_UNLIKELY (p_err_get_last_system () != 0)) {
      p_err_set_err_p (error,
               (int) p_err_get_last_io (),
               p_err_get_last_system (),
               "Failed to call readdir() to read directory stream");
      return NULL;
    }
  }
# else
  if (P_UNLIKELY (readdir_r(dir->dir, &dirent_st, &dir->dir_result) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call readdir_r() to read directory stream"
    );
    return NULL;
  }
# endif
#endif
  if (dir->dir_result == NULL) {
#ifdef P_DIR_NEED_BUF_ALLOC
    p_free (dirent_st);
#endif
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(dirent_t))) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for directory entry"
    );
#ifdef P_DIR_NEED_BUF_ALLOC
    p_free (dirent_st);
#endif
    return NULL;
  }
#ifdef P_DIR_NEED_BUF_ALLOC
  ret->name = p_strdup (dirent_st->d_name);
  p_free (dirent_st);
#else
# ifdef P_DIR_NON_REENTRANT
  ret->name = p_strdup (dir->dir_result->d_name);
# else
  ret->name = p_strdup(dirent_st.d_name);
# endif
#endif
  path_len = strlen(dir->path);
  if (P_UNLIKELY (
    (entry_path = p_malloc0(path_len + strlen(ret->name) + 2)) == NULL)) {
    P_WARNING (
      "dir_t::p_dir_get_next_entry: failed to allocate memory for stat()");
    ret->type = P_DIRENT_OTHER;
    return ret;
  }
  strcat(entry_path, dir->path);
  *(entry_path + path_len) = '/';
  strcat(entry_path + path_len + 1, ret->name);
  if (P_UNLIKELY (stat(entry_path, &sb) != 0)) {
    P_WARNING ("dir_t::p_dir_get_next_entry: stat() failed");
    ret->type = P_DIRENT_OTHER;
    p_free(entry_path);
    return ret;
  }
  p_free(entry_path);
  if (S_ISDIR (sb.st_mode)) {
    ret->type = P_DIRENT_DIR;
  } else if (S_ISREG (sb.st_mode)) {
    ret->type = P_DIRENT_FILE;
  } else {
    ret->type = P_DIRENT_OTHER;
  }
  return ret;
}

bool
p_dir_rewind(dir_t *dir,
  err_t **error) {
  if (P_UNLIKELY (dir == NULL || dir->dir == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  rewinddir(dir->dir);
  return true;
}

void
p_dir_free(dir_t *dir) {
  if (P_UNLIKELY (dir == NULL)) {
    return;
  }
  if (P_LIKELY (dir->dir != NULL)) {
    if (P_UNLIKELY (closedir(dir->dir) != 0))
      P_ERROR ("dir_t::p_dir_free: closedir() failed");
  }
  p_free(dir->path);
  p_free(dir->orig_path);
  p_free(dir);
}
