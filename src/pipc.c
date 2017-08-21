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

#include "p/pmem.h"
#include "p/pcryptohash.h"
#include "p/pstring.h"
#include "psysclose-private.h"

#include <stdlib.h>
#include <string.h>

#if !defined (P_OS_WIN) && !defined (P_OS_OS2)
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/ipc.h>
#endif

#if !defined (P_OS_WIN) && !defined (P_OS_OS2)
pchar *
p_ipc_unix_get_temp_dir(void) {
  pchar *str, *ret;
  psize len;

#ifdef P_tmpdir
  if (strlen(P_tmpdir) > 0)
    str = p_strdup(P_tmpdir);
  else
    return p_strdup("/tmp/");
#else
  const pchar *tmp_env;

  tmp_env = getenv ("TMPDIR");

  if (tmp_env != NULL)
    str = p_strdup (tmp_env);
  else
    return p_strdup ("/tmp/");
#endif /* P_tmpdir */

  /* Now we need to ensure that we have only the one trailing slash */
  len = strlen(str);
  while (*(str + --len) == '/');
  *(str + ++len) = '\0';

  /* len + / + zero symbol */
  if (P_UNLIKELY ((ret = p_malloc0(len + 2)) == NULL)) {
    p_free(str);
    return NULL;
  }

  strcpy(ret, str);
  strcat(ret, "/");

  return ret;
}

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
pint
p_ipc_unix_create_key_file(const pchar *file_name) {
  pint fd;

  if (P_UNLIKELY (file_name == NULL))
    return -1;

  if ((fd = open(file_name, O_CREAT | O_EXCL | O_RDONLY, 0640)) == -1)
    /* file already exists */
    return (errno == EEXIST) ? 1 : -1;
  else
    return p_sys_close(fd);
}

pint
p_ipc_unix_get_ftok_key(const pchar *file_name) {
  struct stat st_info;

  if (P_UNLIKELY (file_name == NULL))
    return -1;

  if (P_UNLIKELY (stat(file_name, &st_info) == -1))
    return -1;

  return ftok(file_name, 'P');
}
#endif /* !P_OS_WIN && !P_OS_OS2 */

/* Returns a platform-independent key for IPC usage, object name for Windows and
 * a file name to use with ftok () for UNIX-like systems */
pchar *
p_ipc_get_platform_key(const pchar *name, pboolean posix) {
  PCryptoHash *sha1;
  pchar *hash_str;

#if defined (P_OS_WIN) || defined (P_OS_OS2)
  P_UNUSED (posix);
#else
  pchar *path_name, *tmp_path;
#endif

  if (P_UNLIKELY (name == NULL))
    return NULL;

  if (P_UNLIKELY ((sha1 = p_crypto_hash_new(P_CRYPTO_HASH_TYPE_SHA1)) == NULL))
    return NULL;

  p_crypto_hash_update(sha1, (const puchar *) name, strlen(name));

  hash_str = p_crypto_hash_get_string(sha1);
  p_crypto_hash_free(sha1);

  if (P_UNLIKELY (hash_str == NULL))
    return NULL;

#if defined (P_OS_WIN) || defined (P_OS_OS2)
  return hash_str;
#else
  if (posix) {
    /* POSIX semaphores which are named kinda like '/semname'.
     * Some implementations of POSIX semaphores has restriction for
     * the name as of max 14 characters, best to use this limit */
    if (P_UNLIKELY ((path_name = p_malloc0(15)) == NULL)) {
      p_free(hash_str);
      return NULL;
    }

    strcpy(path_name, "/");
    strncat(path_name, hash_str, 13);
  } else {
    tmp_path = p_ipc_unix_get_temp_dir();

    /* tmp dir + filename + zero symbol */
    path_name = p_malloc0(strlen(tmp_path) + strlen(hash_str) + 1);

    if (P_UNLIKELY ((path_name) == NULL)) {
      p_free(tmp_path);
      p_free(hash_str);
      return NULL;
    }

    strcpy(path_name, tmp_path);
    strcat(path_name, hash_str);
    p_free(tmp_path);
  }

  p_free(hash_str);
  return path_name;
#endif
}
