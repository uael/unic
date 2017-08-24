/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "unic/mem.h"
#include "unic/hash.h"
#include "unic/string.h"
#include "sysclose-private.h"

#if !defined (U_OS_WIN) && !defined (U_OS_OS2)
# include <errno.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/ipc.h>
#endif

#if !defined (U_OS_WIN) && !defined (U_OS_OS2)
byte_t *
u_ipc_unix_get_temp_dir(void) {
  byte_t *str, *ret;
  size_t len;

#ifdef U_tmpdir
  if (strlen(U_tmpdir) > 0)
    str = u_strdup(U_tmpdir);
  else
    return u_strdup("/tmp/");
#else
  const byte_t *tmp_env;

  tmp_env = getenv ("TMPDIR");

  if (tmp_env != NULL)
    str = u_strdup (tmp_env);
  else
    return u_strdup ("/tmp/");
#endif /* U_tmpdir */

  /* Now we need to ensure that we have only the one trailing slash */
  len = strlen(str);
  while (*(str + --len) == '/');
  *(str + ++len) = '\0';

  /* len + / + zero symbol */
  if (U_UNLIKELY ((ret = u_malloc0(len + 2)) == NULL)) {
    u_free(str);
    return NULL;
  }

  strcpy(ret, str);
  strcat(ret, "/");

  return ret;
}

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
int
u_ipc_unix_create_key_file(const byte_t *file_name) {
  int fd;

  if (U_UNLIKELY (file_name == NULL))
    return -1;

  if ((fd = open(file_name, O_CREAT | O_EXCL | O_RDONLY, 0640)) == -1)
    /* file already exists */
    return (errno == EEXIST) ? 1 : -1;
  else
    return u_sys_close(fd);
}

int
u_ipc_unix_get_ftok_key(const byte_t *file_name) {
  struct stat st_info;

  if (U_UNLIKELY (file_name == NULL))
    return -1;

  if (U_UNLIKELY (stat(file_name, &st_info) == -1))
    return -1;

  return ftok(file_name, 'P');
}
#endif /* !U_OS_WIN && !U_OS_OS2 */

/* Returns a platform-independent key for IPC usage, object name for Windows and
 * a file name to use with ftok () for UNIX-like systems */
byte_t *
u_ipc_get_platform_key(const byte_t *name, bool posix) {
  hash_t *sha1;
  byte_t *hash_str;
#if defined (U_OS_WIN) || defined (U_OS_OS2)
  U_UNUSED (posix);
#else
  byte_t *path_name, *tmp_path;
#endif
  if (U_UNLIKELY (name == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((sha1 = u_crypto_hash_new(U_HASH_SHA1)) == NULL)) {
    return NULL;
  }
  u_crypto_hash_update(sha1, (const ubyte_t *) name, strlen(name));
  hash_str = u_crypto_hash_get_string(sha1);
  u_crypto_hash_free(sha1);
  if (U_UNLIKELY (hash_str == NULL)) {
    return NULL;
  }
#if defined (U_OS_WIN) || defined (U_OS_OS2)
  return hash_str;
#else
  if (posix) {
    /* POSIX semaphores which are named kinda like '/semname'.
     * Some implementations of POSIX semaphores has restriction for
     * the name as of max 14 characters, best to use this limit */
    if (U_UNLIKELY ((path_name = u_malloc0(15)) == NULL)) {
      u_free(hash_str);
      return NULL;
    }

    strcpy(path_name, "/");
    strncat(path_name, hash_str, 13);
  } else {
    tmp_path = u_ipc_unix_get_temp_dir();

    /* tmp dir + filename + zero symbol */
    path_name = u_malloc0(strlen(tmp_path) + strlen(hash_str) + 1);

    if (U_UNLIKELY ((path_name) == NULL)) {
      u_free(tmp_path);
      u_free(hash_str);
      return NULL;
    }

    strcpy(path_name, tmp_path);
    strcat(path_name, hash_str);
    u_free(tmp_path);
  }

  u_free(hash_str);
  return path_name;
#endif
}
