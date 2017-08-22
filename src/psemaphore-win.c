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

#include "p/err.h"
#include "p/mem.h"
#include "p/sema.h"
#include "p/string.h"
#include "perror-private.h"
#include "pipc-private.h"

#define P_SEM_SUFFIX    "_p_sem_object"
#define P_SEM_INVALID_HDL  NULL

typedef HANDLE psem_hdl;

struct sema {
  byte_t *platform_key;
  psem_hdl sem_hdl;
  int_t init_val;
};

static bool
pp_semaphore_create_handle(sema_t *sem, err_t **error);

static void
pp_semaphore_clean_handle(sema_t *sem);

static bool
pp_semaphore_create_handle(sema_t *sem, err_t **error) {
  if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }

  /* Multibyte character set must be enabled */
  if (P_UNLIKELY ((
    sem->sem_hdl = CreateSemaphoreA(
      NULL,
      sem->init_val,
      MAXLONG,
      sem->platform_key
    )) == P_SEM_INVALID_HDL)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call CreateSemaphore() to create semaphore"
    );
    return false;
  }
  return true;
}

static void
pp_semaphore_clean_handle(sema_t *sem) {
  if (P_UNLIKELY (
    sem->sem_hdl != P_SEM_INVALID_HDL && CloseHandle(sem->sem_hdl) == 0))
    P_ERROR ("sema_t::pp_semaphore_clean_handle: CloseHandle() failed");
  sem->sem_hdl = P_SEM_INVALID_HDL;
}

sema_t *
p_sema_new(const byte_t *name,
  int_t init_val,
  sema_access_t mode,
  err_t **error) {
  sema_t *ret;
  byte_t *new_name;
  P_UNUSED (mode);
  if (P_UNLIKELY (name == NULL || init_val < 0)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(sema_t))) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    return NULL;
  }
  if (P_UNLIKELY (
    (new_name = p_malloc0(strlen(name) + strlen(P_SEM_SUFFIX) + 1)) == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    p_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcpy(new_name, P_SEM_SUFFIX);
  ret->platform_key = p_ipc_get_platform_key(new_name, false);
  ret->init_val = init_val;
  p_free(new_name);
  if (P_UNLIKELY (pp_semaphore_create_handle(ret, error) == false)) {
    p_sema_free(ret);
    return NULL;
  }
  return ret;
}

void
p_sema_take_ownership(sema_t *sem) {
  P_UNUSED (sem);
}

bool
p_sema_acquire(sema_t *sem,
  err_t **error) {
  bool ret;
  if (P_UNLIKELY (sem == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ret = (WaitForSingleObject(sem->sem_hdl, INFINITE) == WAIT_OBJECT_0) ? true
    : false;
  if (P_UNLIKELY (ret == false)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call WaitForSingleObject() on semaphore"
    );
  }
  return ret;
}

bool
p_sema_release(sema_t *sem,
  err_t **error) {
  bool ret;
  if (P_UNLIKELY (sem == NULL)) {
    p_error_set_error_p(
      error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ret = ReleaseSemaphore(sem->sem_hdl, 1, NULL) ? true : false;
  if (P_UNLIKELY (ret == false)) {
    p_error_set_error_p(
      error,
      (int_t) p_error_get_last_ipc(),
      p_error_get_last_system(),
      "Failed to call ReleaseSemaphore() on semaphore"
    );
  }
  return ret;
}

void
p_sema_free(sema_t *sem) {
  if (P_UNLIKELY (sem == NULL)) {
    return;
  }
  pp_semaphore_clean_handle(sem);
  if (P_LIKELY (sem->platform_key != NULL)) {
    p_free(sem->platform_key);
  }
  p_free(sem);
}
