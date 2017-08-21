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

#include "p/error.h"
#include "p/mem.h"
#include "p/shm.h"
#include "perror-private.h"
#include "pipc-private.h"

#include <stdlib.h>
#include <string.h>

#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#define P_SHM_MEM_PREFIX  "\\SHAREMEM\\"
#define P_SHM_SEM_PREFIX  "\\SEM32\\"
#define P_SHM_SUFFIX    "_p_shm_object"

struct PShm_ {
  byte_t *platform_key;
  ptr_t addr;
  size_t size;
  HMTX sem;
  PShmAccessPerms perms;
};

static bool pp_shm_create_handle(PShm *shm, p_err_t **error);
static void pp_shm_clean_handle(PShm *shm);

static bool
pp_shm_create_handle(PShm *shm,
  p_err_t **error) {
  byte_t *mem_name;
  byte_t *sem_name;
  APIRET ulrc;
  ULONG flags;

  if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  flags = PAG_COMMIT | PAG_READ;

  if (shm->perms != P_SHM_ACCESS_READONLY)
    flags |= PAG_WRITE;

  if (P_UNLIKELY ((mem_name = p_malloc0(strlen(shm->platform_key) +
    strlen(P_SHM_MEM_PREFIX) + 1)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared memory name");
    return false;
  }

  strcpy(mem_name, P_SHM_MEM_PREFIX);
  strcat(mem_name, shm->platform_key);

  while ((ulrc = DosAllocSharedMem((PPVOID) & shm->addr,
    (PSZ) mem_name,
    shm->size,
    flags)) == ERROR_INTERRUPT);

  if (P_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_ALREADY_EXISTS)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_ipc_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosAllocSharedMem() to allocate shared memory");
    p_free(mem_name);
    pp_shm_clean_handle(shm);
    return false;
  }

  if (ulrc == ERROR_ALREADY_EXISTS) {
    ULONG real_size;
    ULONG real_flags;

    flags =
      (shm->perms == P_SHM_ACCESS_READONLY) ? PAG_READ : (PAG_WRITE | PAG_READ);

    while ((ulrc = DosGetNamedSharedMem((PPVOID) & shm->addr,
      (PSZ) mem_name,
      flags)) == ERROR_INTERRUPT);

    p_free(mem_name);

    if (P_UNLIKELY (ulrc != NO_ERROR)) {
      p_error_set_error_p(error,
        (int_t) p_error_get_ipc_from_system((int_t) ulrc),
        (int_t) ulrc,
        "Failed to call DosGetNamedSharedMem() to get shared memory");
      pp_shm_clean_handle(shm);
      return false;
    }

    real_size = (ULONG) shm->size;

    while ((ulrc = DosQueryMem((PVOID) shm->addr,
      &real_size,
      &real_flags)) == ERROR_INTERRUPT);

    if (P_UNLIKELY (ulrc != NO_ERROR)) {
      p_error_set_error_p(error,
        (int_t) p_error_get_ipc_from_system((int_t) ulrc),
        (int_t) ulrc,
        "Failed to call DosQueryMem() to get memory info");
      pp_shm_clean_handle(shm);
      return false;
    }

    shm->size = (size_t) real_size;
  } else
    p_free(mem_name);

  if (P_UNLIKELY ((sem_name = p_malloc0(strlen(shm->platform_key) +
    strlen(P_SHM_SEM_PREFIX) + 1)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared memory name");
    pp_shm_clean_handle(shm);
    return false;
  }

  strcpy(sem_name, P_SHM_SEM_PREFIX);
  strcat(sem_name, shm->platform_key);

  ulrc = DosCreateMutexSem((PSZ) sem_name, &shm->sem, 0, false);

  if (ulrc == ERROR_DUPLICATE_NAME)
    ulrc = DosOpenMutexSem((PSZ) sem_name, &shm->sem);

  p_free(sem_name);

  if (P_UNLIKELY (ulrc != NO_ERROR)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_ipc_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to call DosCreateMutexSem() to create a lock");
    pp_shm_clean_handle(shm);
    return false;
  }

  return true;
}

static void
pp_shm_clean_handle(PShm *shm) {
  APIRET ulrc;

  if (P_UNLIKELY (shm->addr != NULL)) {
    while ((ulrc = DosFreeMem((PVOID) shm->addr)) == ERROR_INTERRUPT);

    if (P_UNLIKELY (ulrc != NO_ERROR))
      P_ERROR ("PShm::pp_shm_clean_handle: DosFreeMem() failed");

    shm->addr = NULL;
  }

  if (P_LIKELY (shm->sem != NULLHANDLE)) {
    if (P_UNLIKELY (DosCloseMutexSem(shm->sem) != NO_ERROR))
      P_ERROR ("PShm::pp_shm_clean_handle: DosCloseMutexSem() failed");

    shm->sem = NULLHANDLE;
  }

  shm->size = 0;
}

P_API PShm *
p_shm_new(const byte_t *name,
  size_t size,
  PShmAccessPerms perms,
  p_err_t **error) {
  PShm *ret;
  byte_t *new_name;

  if (P_UNLIKELY (name == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return NULL;
  }

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PShm))) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared segment");
    return NULL;
  }

  if (P_UNLIKELY (
    (new_name = p_malloc0(strlen(name) + strlen(P_SHM_SUFFIX) + 1)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for segment name");
    p_shm_free(ret);
    return NULL;
  }

  strcpy(new_name, name);
  strcat(new_name, P_SHM_SUFFIX);

  ret->platform_key = p_ipc_get_platform_key(new_name, false);
  ret->perms = perms;
  ret->size = size;

  p_free(new_name);

  if (P_UNLIKELY (pp_shm_create_handle(ret, error) == false)) {
    p_shm_free(ret);
    return NULL;
  }

  if (P_LIKELY (ret->size > size && size != 0))
    ret->size = size;

  return ret;
}

P_API void
p_shm_take_ownership(PShm *shm) {
  P_UNUSED (shm);
}

P_API void
p_shm_free(PShm *shm) {
  if (P_UNLIKELY (shm == NULL))
    return;

  pp_shm_clean_handle(shm);

  if (P_LIKELY (shm->platform_key != NULL))
    p_free(shm->platform_key);

  p_free(shm);
}

P_API bool
p_shm_lock(PShm *shm,
  p_err_t **error) {
  APIRET ulrc;

  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  while ((ulrc = DosRequestMutexSem(shm->sem,
    (ULONG) SEM_INDEFINITE_WAIT)) == ERROR_INTERRUPT);

  if (P_UNLIKELY (ulrc != NO_ERROR)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_ipc_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to lock memory segment");
    return false;
  }

  return true;
}

P_API bool
p_shm_unlock(PShm *shm,
  p_err_t **error) {
  APIRET ulrc;

  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  ulrc = DosReleaseMutexSem(shm->sem);

  if (P_UNLIKELY (ulrc != NO_ERROR)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_ipc_from_system((int_t) ulrc),
      (int_t) ulrc,
      "Failed to unlock memory segment");
    return false;
  }

  return true;
}

P_API ptr_t
p_shm_get_address(const PShm *shm) {
  if (P_UNLIKELY (shm == NULL))
    return NULL;

  return shm->addr;
}

P_API size_t
p_shm_get_size(const PShm *shm) {
  if (P_UNLIKELY (shm == NULL))
    return 0;

  return shm->size;
}
