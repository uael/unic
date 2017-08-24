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

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/sema.h"
#include "unic/string.h"
#include "unic/shm.h"
#include "err-private.h"
#include "ipc-private.h"
#include "sysclose-private.h"

#define U_SHM_SUFFIX    "_p_shm_object"
#define U_SHM_INVALID_HDL  -1
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
  int fd, flags;
  struct stat stat_buf;
  if (U_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
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
    )) == U_SHM_INVALID_HDL &&
    u_err_get_last_system() == EINTR) {}
  if (fd == U_SHM_INVALID_HDL) {
    if (u_err_get_last_system() == EEXIST) {
      is_exists = true;
      while ((
        fd = shm_open(
          shm->platform_key,
          O_RDWR,
          0660
        )) == U_SHM_INVALID_HDL &&
        u_err_get_last_system() == EINTR) {}
    }
  } else {
    shm->shm_created = true;
  }
  if (U_UNLIKELY (fd == U_SHM_INVALID_HDL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call shm_open() to create memory segment"
    );
    pp_shm_clean_handle(shm);
    return false;
  }

  /* Try to get size of the existing file descriptor */
  if (is_exists) {
    if (U_UNLIKELY (fstat(fd, &stat_buf) == -1)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_last_ipc(),
        u_err_get_last_system(),
        "Failed to call fstat() to get memory segment size"
      );
      if (U_UNLIKELY (u_sys_close(fd) != 0))
        U_WARNING ("shm_t::pp_shm_create_handle: u_sys_close() failed(1)");
      pp_shm_clean_handle(shm);
      return false;
    }
    shm->size = (size_t) stat_buf.st_size;
  } else {
    if (U_UNLIKELY ((ftruncate(fd, (off_t) shm->size)) == -1)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_last_ipc(),
        u_err_get_last_system(),
        "Failed to call ftruncate() to set memory segment size"
      );
      if (U_UNLIKELY (u_sys_close(fd) != 0))
        U_WARNING ("shm_t::pp_shm_create_handle: u_sys_close() failed(2)");
      pp_shm_clean_handle(shm);
      return false;
    }
  }
  flags =
    (shm->perms == U_SHM_ACCESS_READONLY) ? PROT_READ : PROT_READ | PROT_WRITE;
  if (U_UNLIKELY ((shm->addr = mmap(NULL, shm->size, flags, MAP_SHARED, fd, 0))
    == (void *) -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call mmap() to map memory segment"
    );
    shm->addr = NULL;
    if (U_UNLIKELY (u_sys_close(fd) != 0))
      U_WARNING ("shm_t::pp_shm_create_handle: u_sys_close() failed(3)");
    pp_shm_clean_handle(shm);
    return false;
  }
  if (U_UNLIKELY (u_sys_close(fd) != 0))
    U_WARNING ("shm_t::pp_shm_create_handle: u_sys_close() failed(4)");
  if (U_UNLIKELY ((
    shm->sem = u_sema_new(
      shm->platform_key, 1,
      is_exists ? U_SEMA_OPEN : U_SEMA_CREATE,
      error
    )) == NULL)) {
    pp_shm_clean_handle(shm);
    return false;
  }
  return true;
}

static void
pp_shm_clean_handle(shm_t *shm) {
  if (U_UNLIKELY (shm->addr != NULL && munmap(shm->addr, shm->size) == -1))
    U_ERROR ("shm_t::pp_shm_clean_handle: munmap () failed");
  if (shm->shm_created == true && shm_unlink(shm->platform_key) == -1)
    U_ERROR ("shm_t::pp_shm_clean_handle: shm_unlink() failed");
  if (U_LIKELY (shm->sem != NULL)) {
    u_sema_free(shm->sem);
    shm->sem = NULL;
  }
  shm->shm_created = false;
  shm->addr = NULL;
  shm->size = 0;
}

shm_t *
u_shm_new(const byte_t *name,
  size_t size,
  shm_access_t perms,
  err_t **error) {
  shm_t *ret;
  byte_t *new_name;
  if (U_UNLIKELY (name == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(shm_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared segment"
    );
    return NULL;
  }
  if (U_UNLIKELY (
    (new_name = u_malloc0(strlen(name) + strlen(U_SHM_SUFFIX) + 1)) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for segment name"
    );
    u_shm_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcat(new_name, U_SHM_SUFFIX);
#if defined (U_OS_IRIX) || defined (U_OS_TRU64)
  /* IRIX and Tru64 prefer filename styled IPC names */
  ret->platform_key = u_ipc_get_platform_key (new_name, false);
#else
  ret->platform_key = u_ipc_get_platform_key(new_name, true);
#endif
  ret->perms = perms;
  ret->size = size;
  u_free(new_name);
  if (U_UNLIKELY (pp_shm_create_handle(ret, error) == false)) {
    u_shm_free(ret);
    return NULL;
  }
  if (U_LIKELY (ret->size > size && size != 0)) {
    ret->size = size;
  }
  return ret;
}

void
u_shm_take_ownership(shm_t *shm) {
  if (U_UNLIKELY (shm == NULL)) {
    return;
  }
  shm->shm_created = true;
  u_sema_take_ownership(shm->sem);
}

void
u_shm_free(shm_t *shm) {
  if (U_UNLIKELY (shm == NULL)) {
    return;
  }
  pp_shm_clean_handle(shm);
  if (U_LIKELY (shm->platform_key != NULL)) {
    u_free(shm->platform_key);
  }
  u_free(shm);
}

bool
u_shm_lock(shm_t *shm,
  err_t **error) {
  if (U_UNLIKELY (shm == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  return u_sema_acquire(shm->sem, error);
}

bool
u_shm_unlock(shm_t *shm,
  err_t **error) {
  if (U_UNLIKELY (shm == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  return u_sema_release(shm->sem, error);
}

ptr_t
u_shm_get_address(const shm_t *shm) {
  if (U_UNLIKELY (shm == NULL)) {
    return NULL;
  }
  return shm->addr;
}

size_t
u_shm_get_size(const shm_t *shm) {
  if (U_UNLIKELY (shm == NULL)) {
    return 0;
  }
  return shm->size;
}
