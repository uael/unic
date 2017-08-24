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

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/sema.h"
#include "unic/string.h"
#include "err-private.h"
#include "ipc-private.h"

#define U_SEM_SUFFIX    "_p_sem_object"
#define U_SEM_INVALID_HDL  NULL

typedef HANDLE sema_hdl_t;

struct sema {
  byte_t *platform_key;
  sema_hdl_t sem_hdl;
  int init_val;
};

static bool
pp_sema_create_handle(sema_t *sem, err_t **error);

static void
pp_sema_clean_handle(sema_t *sem);

static bool
pp_sema_create_handle(sema_t *sem, err_t **error) {
  if (U_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }

  /* Multibyte character set must be enabled */
  if (U_UNLIKELY ((
    sem->sem_hdl = CreateSemaphoreA(
      NULL,
      sem->init_val,
      MAXLONG,
      sem->platform_key
    )) == U_SEM_INVALID_HDL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call CreateSemaphore() to create semaphore"
    );
    return false;
  }
  return true;
}

static void
pp_sema_clean_handle(sema_t *sem) {
  if (U_UNLIKELY (
    sem->sem_hdl != U_SEM_INVALID_HDL && CloseHandle(sem->sem_hdl) == 0))
    U_ERROR ("sema_t::pp_sema_clean_handle: CloseHandle() failed");
  sem->sem_hdl = U_SEM_INVALID_HDL;
}

sema_t *
u_sema_new(const byte_t *name,
  int init_val,
  sema_access_t mode,
  err_t **error) {
  sema_t *ret;
  byte_t *new_name;
  U_UNUSED (mode);
  if (U_UNLIKELY (name == NULL || init_val < 0)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(sema_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    return NULL;
  }
  if (U_UNLIKELY (
    (new_name = u_malloc0(strlen(name) + strlen(U_SEM_SUFFIX) + 1)) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    u_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcpy(new_name, U_SEM_SUFFIX);
  ret->platform_key = u_ipc_get_platform_key(new_name, false);
  ret->init_val = init_val;
  u_free(new_name);
  if (U_UNLIKELY (pp_sema_create_handle(ret, error) == false)) {
    u_sema_free(ret);
    return NULL;
  }
  return ret;
}

void
u_sema_take_ownership(sema_t *sem) {
  U_UNUSED (sem);
}

bool
u_sema_acquire(sema_t *sem,
  err_t **error) {
  bool ret;
  if (U_UNLIKELY (sem == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ret = (WaitForSingleObject(sem->sem_hdl, INFINITE) == WAIT_OBJECT_0) ? true
    : false;
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call WaitForSingleObject() on semaphore"
    );
  }
  return ret;
}

bool
u_sema_release(sema_t *sem,
  err_t **error) {
  bool ret;
  if (U_UNLIKELY (sem == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ret = ReleaseSemaphore(sem->sem_hdl, 1, NULL) ? true : false;
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call ReleaseSemaphore() on semaphore"
    );
  }
  return ret;
}

void
u_sema_free(sema_t *sem) {
  if (U_UNLIKELY (sem == NULL)) {
    return;
  }
  pp_sema_clean_handle(sem);
  if (U_LIKELY (sem->platform_key != NULL)) {
    u_free(sem->platform_key);
  }
  u_free(sem);
}
