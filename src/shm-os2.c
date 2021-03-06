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

#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/shm.h"
#include "unic/string.h"
#include "err-private.h"
#include "ipc-private.h"

#define U_SHM_MEM_PREFIX  "\\SHAREMEM\\"
#define U_SHM_SEM_PREFIX  "\\SEM32\\"
#define U_SHM_SUFFIX    "_p_shm_object"

struct shm {
  byte_t *platform_key;
  ptr_t addr;
  size_t size;
  HMTX sem;
  shm_access_t perms;
};

static bool
pp_shm_create_handle(shm_t *shm, err_t **error);

static void
pp_shm_clean_handle(shm_t *shm);

static bool
pp_shm_create_handle(shm_t *shm,
  err_t **error) {
  byte_t *mem_name;
  byte_t *sem_name;
  APIRET ulrc;
  ULONG flags;
  if (U_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  flags = PAG_COMMIT | PAG_READ;
  if (shm->perms != U_SHM_ACCESS_READONLY) {
    flags |= PAG_WRITE;
  }
  if (U_UNLIKELY ((
    mem_name = u_malloc0(
      strlen(shm->platform_key) +
        strlen(U_SHM_MEM_PREFIX) + 1
    )) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared memory name"
    );
    return false;
  }
  strcpy(mem_name, U_SHM_MEM_PREFIX);
  strcat(mem_name, shm->platform_key);
  while ((
    ulrc = DosAllocSharedMem((PPVOID) & shm->addr,
      (PSZ) mem_name,
      shm->size,
      flags
    )) == ERROR_INTERRUPT) {}
  if (U_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_ALREADY_EXISTS)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_ipc_from_system((int) ulrc),
      (int) ulrc,
      "Failed to call DosAllocSharedMem() to allocate shared memory"
    );
    u_free(mem_name);
    pp_shm_clean_handle(shm);
    return false;
  }
  if (ulrc == ERROR_ALREADY_EXISTS) {
    ULONG real_size;
    ULONG real_flags;
    flags =
      (shm->perms == U_SHM_ACCESS_READONLY) ? PAG_READ : (PAG_WRITE | PAG_READ);
    while ((
      ulrc = DosGetNamedSharedMem((PPVOID) & shm->addr,
        (PSZ) mem_name,
        flags
      )) == ERROR_INTERRUPT) {}
    u_free(mem_name);
    if (U_UNLIKELY (ulrc != NO_ERROR)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_ipc_from_system((int) ulrc),
        (int) ulrc,
        "Failed to call DosGetNamedSharedMem() to get shared memory"
      );
      pp_shm_clean_handle(shm);
      return false;
    }
    real_size = (ULONG) shm->size;
    while ((
      ulrc = DosQueryMem((PVOID) shm->addr,
        &real_size,
        &real_flags
      )) == ERROR_INTERRUPT) {}
    if (U_UNLIKELY (ulrc != NO_ERROR)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_ipc_from_system((int) ulrc),
        (int) ulrc,
        "Failed to call DosQueryMem() to get memory info"
      );
      pp_shm_clean_handle(shm);
      return false;
    }
    shm->size = (size_t) real_size;
  } else {
    u_free(mem_name);
  }
  if (U_UNLIKELY ((
    sem_name = u_malloc0(
      strlen(shm->platform_key) +
        strlen(U_SHM_SEM_PREFIX) + 1
    )) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared memory name"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
  strcpy(sem_name, U_SHM_SEM_PREFIX);
  strcat(sem_name, shm->platform_key);
  ulrc = DosCreateMutexSem((PSZ) sem_name, &shm->sem, 0, false);
  if (ulrc == ERROR_DUPLICATE_NAME) {
    ulrc = DosOpenMutexSem((PSZ) sem_name, &shm->sem);
  }
  u_free(sem_name);
  if (U_UNLIKELY (ulrc != NO_ERROR)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_ipc_from_system((int) ulrc),
      (int) ulrc,
      "Failed to call DosCreateMutexSem() to create a lock"
    );
    pp_shm_clean_handle(shm);
    return false;
  }
  return true;
}

static void
pp_shm_clean_handle(shm_t *shm) {
  APIRET ulrc;
  if (U_UNLIKELY (shm->addr != NULL)) {
    while ((ulrc = DosFreeMem((PVOID) shm->addr)) == ERROR_INTERRUPT);
    if (U_UNLIKELY (ulrc != NO_ERROR))
      U_ERROR ("shm_t::pp_shm_clean_handle: DosFreeMem() failed");
    shm->addr = NULL;
  }
  if (U_LIKELY (shm->sem != NULLHANDLE)) {
    if (U_UNLIKELY (DosCloseMutexSem(shm->sem) != NO_ERROR))
      U_ERROR ("shm_t::pp_shm_clean_handle: DosCloseMutexSem() failed");
    shm->sem = NULLHANDLE;
  }
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
  U_UNUSED (shm);
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
  APIRET ulrc;
  if (U_UNLIKELY (shm == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  while ((
    ulrc = DosRequestMutexSem(
      shm->sem,
      (ULONG) SEM_INDEFINITE_WAIT
    )) == ERROR_INTERRUPT) {}
  if (U_UNLIKELY (ulrc != NO_ERROR)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_ipc_from_system((int) ulrc),
      (int) ulrc,
      "Failed to lock memory segment"
    );
    return false;
  }
  return true;
}

bool
u_shm_unlock(shm_t *shm,
  err_t **error) {
  APIRET ulrc;
  if (U_UNLIKELY (shm == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ulrc = DosReleaseMutexSem(shm->sem);
  if (U_UNLIKELY (ulrc != NO_ERROR)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_ipc_from_system((int) ulrc),
      (int) ulrc,
      "Failed to unlock memory segment"
    );
    return false;
  }
  return true;
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
