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

#include "p/error.h"
#include "p/mem.h"
#include "p/sem.h"
#include "p/shm.h"
#include "perror-private.h"
#include "pipc-private.h"

#include <stdlib.h>
#include <string.h>

#define P_SHM_INVALID_HDL  NULL
#define P_SHM_SUFFIX    "_p_shm_object"

typedef HANDLE pshm_hdl;

struct PShm_ {
  byte_t *platform_key;
  pshm_hdl shm_hdl;
  ptr_t addr;
  size_t size;
  PSemaphore *sem;
  PShmAccessPerms perms;
};

static bool pp_shm_create_handle(PShm *shm, p_err_t **error);
static void pp_shm_clean_handle(PShm *shm);

static bool
pp_shm_create_handle(PShm *shm,
  p_err_t **error) {
  bool is_exists;
  MEMORY_BASIC_INFORMATION mem_stat;
  DWORD protect;

  if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  is_exists = false;

  protect =
    (shm->perms == P_SHM_ACCESS_READONLY) ? PAGE_READONLY : PAGE_READWRITE;

  /* Multibyte character set must be enabled */
  if (P_UNLIKELY ((shm->shm_hdl = CreateFileMappingA(INVALID_HANDLE_VALUE,
    NULL,
    protect,
    0,
    (DWORD) shm->size,
    shm->platform_key)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call CreateFileMapping() to create file mapping");
    pp_shm_clean_handle(shm);
    return false;
  }

  protect = (protect == PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;

  if (P_UNLIKELY (
    (shm->addr = MapViewOfFile(shm->shm_hdl, protect, 0, 0, 0)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call MapViewOfFile() to map file to memory");
    pp_shm_clean_handle(shm);
    return false;
  }

  if (p_error_get_last_system() == ERROR_ALREADY_EXISTS)
    is_exists = true;

  if (P_UNLIKELY (VirtualQuery(shm->addr, &mem_stat, sizeof(mem_stat)) == 0)) {
    p_error_set_error_p(error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call VirtualQuery() to get memory map info");
    pp_shm_clean_handle(shm);
    return false;
  }

  shm->size = mem_stat.RegionSize;

  if (P_UNLIKELY ((shm->sem = p_semaphore_new(shm->platform_key, 1,
    is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE,
    error)) == NULL)) {
    pp_shm_clean_handle(shm);
    return false;
  }

  return true;
}

static void
pp_shm_clean_handle(PShm *shm) {
  if (P_UNLIKELY (
    shm->addr != NULL && UnmapViewOfFile((char *) shm->addr) == 0))
    P_ERROR ("PShm::pp_shm_clean_handle: UnmapViewOfFile() failed");

  if (P_UNLIKELY (
    shm->shm_hdl != P_SHM_INVALID_HDL && CloseHandle(shm->shm_hdl) == 0))
    P_ERROR ("PShm::pp_shm_clean_handle: CloseHandle() failed");

  if (P_LIKELY (shm->sem != NULL)) {
    p_semaphore_free(shm->sem);
    shm->sem = NULL;
  }

  shm->shm_hdl = P_SHM_INVALID_HDL;
  shm->addr = NULL;
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
  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  return p_semaphore_acquire(shm->sem, error);
}

P_API bool
p_shm_unlock(PShm *shm,
  p_err_t **error) {
  if (P_UNLIKELY (shm == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  return p_semaphore_release(shm->sem, error);
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
