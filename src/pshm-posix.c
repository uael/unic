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

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#include "p/err.h"
#include "p/mem.h"
#include "p/sema.h"
#include "p/string.h"
#include "p/shm.h"
#include "perror-private.h"
#include "pipc-private.h"
#include "psysclose-private.h"

#define P_SHM_SUFFIX    "_p_shm_object"
#define P_SHM_INVALID_HDL  -1
struct shm {
  bool shm_created;
  byte_t *platform_key;
  ptr_t addr;
  size_t size;
  sema_t *sem;
  shm_access_t perms;
};

static bool
pp_shm_create_handle(shm_t *shm, err_t **error);

static void
pp_shm_clean_handle(shm_t *shm);

static bool
pp_shm_create_handle(shm_t *shm,
  err_t **error) {
  bool is_exists;
  int_t fd, flags;
  struct stat stat_buf;
  if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  is_exists = false;
  while ((
    fd = shm_open(
      shm->platform_key,
      O_CREAT | O_EXCL | O_RDWR,
      0660
    )) == P_SHM_INVALID_HDL &&
    p_error_get_last_system() == EINTR) {}
  if (fd == P_SHM_INVALID_HDL) {
    if (p_error_get_last_system() == EEXIST) {
      is_exists = true;
      while ((
        fd = shm_open(
          shm->platform_key,
          O_RDWR,
          0660
        )) == P_SHM_INVALID_HDL &&
        p_error_get_last_system() == EINTR) {}
    }
  } else {
    shm->shm_created = true;
  }
  if (P_UNLIKELY (fd == P_SHM_INVALID_HDL)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call shm_open() to create memory segment"
    );
    pp_shm_clean_handle(shm);
    return false;
  }

  /* Try to get size of the existing file descriptor */
  if (is_exists) {
    if (P_UNLIKELY (fstat(fd, &stat_buf) == -1)) {
      p_error_set_error_p(
        error,
        (int_t) p_error_get_last_ipc(),
        p_error_get_last_system(),
        "Failed to call fstat() to get memory segment size"
      );
      if (P_UNLIKELY (p_sys_close(fd) != 0))
        P_WARNING ("shm_t::pp_shm_create_handle: p_sys_close() failed(1)");
      pp_shm_clean_handle(shm);
      return false;
    }
    shm->size = (size_t) stat_buf.st_size;
  } else {
    if (P_UNLIKELY ((ftruncate(fd, (off_t) shm->size)) == -1)) {
      p_error_set_error_p(
        error,
        (int_t) p_error_get_last_ipc(),
        p_error_get_last_system(),
        "Failed to call ftruncate() to set memory segment size"
      );
      if (P_UNLIKELY (p_sys_close(fd) != 0))
        P_WARNING ("shm_t::pp_shm_create_handle: p_sys_close() failed(2)");
      pp_shm_clean_handle(shm);
      return false;
    }
  }
  flags =
    (shm->perms == P_SHM_ACCESS_READONLY) ? PROT_READ : PROT_READ | PROT_WRITE;
  if (P_UNLIKELY ((shm->addr = mmap(NULL, shm->size, flags, MAP_SHARED, fd, 0))
    == (void *) -1)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call mmap() to map memory segment"
    );
    shm->addr = NULL;
    if (P_UNLIKELY (p_sys_close(fd) != 0))
      P_WARNING ("shm_t::pp_shm_create_handle: p_sys_close() failed(3)");
    pp_shm_clean_handle(shm);
    return false;
  }
  if (P_UNLIKELY (p_sys_close(fd) != 0))
    P_WARNING ("shm_t::pp_shm_create_handle: p_sys_close() failed(4)");
  if (P_UNLIKELY ((
    shm->sem = p_sema_new(
      shm->platform_key, 1,
      is_exists ? P_SEMA_OPEN : P_SEMA_CREATE,
      error
    )) == NULL)) {
    pp_shm_clean_handle(shm);
    return false;
  }
  return true;
}

static void
pp_shm_clean_handle(shm_t *shm) {
  if (P_UNLIKELY (shm->addr != NULL && munmap(shm->addr, shm->size) == -1))
    P_ERROR ("shm_t::pp_shm_clean_handle: munmap () failed");
  if (shm->shm_created == true && shm_unlink(shm->platform_key) == -1)
    P_ERROR ("shm_t::pp_shm_clean_handle: shm_unlink() failed");
  if (P_LIKELY (shm->sem != NULL)) {
    p_sema_free(shm->sem);
    shm->sem = NULL;
  }
  shm->shm_created = false;
  shm->addr = NULL;
  shm->size = 0;
}

shm_t *
p_shm_new(const byte_t *name,
  size_t size,
  shm_access_t perms,
  err_t **error) {
  shm_t *ret;
  byte_t *new_name;
  if (P_UNLIKELY (name == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(shm_t))) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared segment"
    );
    return NULL;
  }
  if (P_UNLIKELY (
    (new_name = p_malloc0(strlen(name) + strlen(P_SHM_SUFFIX) + 1)) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for segment name"
    );
    p_shm_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcat(new_name, P_SHM_SUFFIX);
#if defined (P_OS_IRIX) || defined (P_OS_TRU64)
  /* IRIX and Tru64 prefer filename styled IPC names */
  ret->platform_key = p_ipc_get_platform_key (new_name, false);
#else
  ret->platform_key = p_ipc_get_platform_key(new_name, true);
#endif
  ret->perms = perms;
  ret->size = size;
  p_free(new_name);
  if (P_UNLIKELY (pp_shm_create_handle(ret, error) == false)) {
    p_shm_free(ret);
    return NULL;
  }
  if (P_LIKELY (ret->size > size && size != 0)) {
    ret->size = size;
  }
  return ret;
}

void
p_shm_take_ownership(shm_t *shm) {
  if (P_UNLIKELY (shm == NULL)) {
    return;
  }
  shm->shm_created = true;
  p_sema_take_ownership(shm->sem);
}

void
p_shm_free(shm_t *shm) {
  if (P_UNLIKELY (shm == NULL)) {
    return;
  }
  pp_shm_clean_handle(shm);
  if (P_LIKELY (shm->platform_key != NULL)) {
    p_free(shm->platform_key);
  }
  p_free(shm);
}

bool
p_shm_lock(shm_t *shm,
  err_t **error) {
  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  return p_sema_acquire(shm->sem, error);
}

bool
p_shm_unlock(shm_t *shm,
  err_t **error) {
  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  return p_sema_release(shm->sem, error);
}

ptr_t
p_shm_get_address(const shm_t *shm) {
  if (P_UNLIKELY (shm == NULL)) {
    return NULL;
  }
  return shm->addr;
}

size_t
p_shm_get_size(const shm_t *shm) {
  if (P_UNLIKELY (shm == NULL)) {
    return 0;
  }
  return shm->size;
}
