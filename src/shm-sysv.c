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
#include <sys/shm.h>
#include <errno.h>

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/sema.h"
#include "unic/string.h"
#include "unic/shm.h"
#include "err-private.h"
#include "ipc-private.h"

#define U_SHM_SUFFIX    "_p_shm_object"
#define U_SHM_INVALID_HDL  -1

typedef int shm_hdl_t;

struct shm {
  bool file_created;
  key_t unix_key;
  byte_t *platform_key;
  shm_hdl_t shm_hdl;
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
  int flags, built;
  struct shmid_ds shm_stat;
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
  if (U_UNLIKELY (
    (built = u_ipc_unix_create_key_file(shm->platform_key)) == -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to create key file"
    );
    pp_shm_clean_handle(shm);
    return false;
  } else if (built == 0) {
    shm->file_created = true;
  }
  if (U_UNLIKELY (
    (shm->unix_key = u_ipc_unix_get_ftok_key(shm->platform_key)) == -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to get unique IPC key"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
  flags = (shm->perms == U_SHM_ACCESS_READONLY) ? 0444 : 0660;
  if ((
    shm->shm_hdl = shmget(
      shm->unix_key,
      shm->size,
      IPC_CREAT | IPC_EXCL | flags
    )) == U_SHM_INVALID_HDL) {
    if (u_err_get_last_system() == EEXIST) {
      is_exists = true;
      shm->shm_hdl = shmget(shm->unix_key, 0, flags);
    }
  } else {
    shm->file_created = (built == 1);
  }
  if (U_UNLIKELY (shm->shm_hdl == U_SHM_INVALID_HDL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call shmget() to create memory segment"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
  if (U_UNLIKELY (shmctl(shm->shm_hdl, IPC_STAT, &shm_stat) == -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call shmctl() to get memory segment size"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
  shm->size = shm_stat.shm_segsz;
  flags = (shm->perms == U_SHM_ACCESS_READONLY) ? SHM_RDONLY : 0;
  if (U_UNLIKELY ((shm->addr = shmat(shm->shm_hdl, 0, flags)) == (void *) -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call shmat() to attach to the memory segment"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
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
  struct shmid_ds shm_stat;
  if (U_LIKELY (shm->addr != NULL)) {
    if (U_UNLIKELY (shmdt(shm->addr) == -1))
      U_ERROR ("shm_t::pp_shm_clean_handle: shmdt() failed");
    if (U_UNLIKELY (shmctl(shm->shm_hdl, IPC_STAT, &shm_stat) == -1))
      U_ERROR ("shm_t::pp_shm_clean_handle: shmctl() with IPC_STAT failed");
    if (U_UNLIKELY (
      shm_stat.shm_nattch == 0 && shmctl(shm->shm_hdl, IPC_RMID, 0) == -1))
      U_ERROR ("shm_t::pp_shm_clean_handle: shmctl() with IPC_RMID failed");
  }
  if (shm->file_created == true && unlink(shm->platform_key) == -1)
    U_ERROR ("shm_t::pp_shm_clean_handle: unlink() failed");
  if (U_LIKELY (shm->sem != NULL)) {
    u_sema_free(shm->sem);
    shm->sem = NULL;
  }
  shm->file_created = false;
  shm->unix_key = -1;
  shm->shm_hdl = U_SHM_INVALID_HDL;
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
  ret->platform_key = u_ipc_get_platform_key(new_name, false);
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
  shm->file_created = true;
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
