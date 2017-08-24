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

#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/sema.h"
#include "err-private.h"
#include "ipc-private.h"

#define U_SEM_SUFFIX    "_p_sem_object"

typedef sem_t sema_hdl_t;

/* On some HP-UX versions it may not be defined */
#ifndef SEM_FAILED
# define SEM_FAILED ((sema_t *) -1)
#endif

#ifdef U_OS_SOLARIS
# define U_SEM_INVALID_HDL (sema_t *) -1
#else
# define U_SEM_INVALID_HDL  SEM_FAILED
#endif

struct sema {
  bool sem_created;
  byte_t *platform_key;
#if defined (U_OS_VMS) && (UNIC_SIZEOF_VOID_P == 4)
# pragma __pointer_size 64
#endif
  sema_hdl_t *sem_hdl;
#if defined (U_OS_VMS) && (UNIC_SIZEOF_VOID_P == 4)
# pragma __pointer_size 32
#endif
  sema_access_t mode;
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

  /* Solaris may interrupt sem_open() call */
  while ((
    sem->sem_hdl = sem_open(
      sem->platform_key,
      O_CREAT | O_EXCL,
      0660,
      sem->init_val
    )) == U_SEM_INVALID_HDL &&
    u_err_get_last_system() == EINTR) {}
  if (sem->sem_hdl == U_SEM_INVALID_HDL) {
    if (u_err_get_last_system() == EEXIST) {
      if (sem->mode == U_SEMA_CREATE) {
        sem_unlink(sem->platform_key);
      }
      while ((
        sem->sem_hdl = sem_open(
          sem->platform_key,
          0,
          0,
          0
        )) == U_SEM_INVALID_HDL &&
        u_err_get_last_system() == EINTR) {}
    }
  } else {
    sem->sem_created = true;
  }
  if (U_UNLIKELY (sem->sem_hdl == U_SEM_INVALID_HDL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call sem_open() to create semaphore"
    );
    pp_sema_clean_handle(sem);
    return false;
  }
  return true;
}

static void
pp_sema_clean_handle(sema_t *sem) {
  if (U_UNLIKELY (sem->sem_hdl != U_SEM_INVALID_HDL &&
    sem_close(sem->sem_hdl) == -1))
    U_ERROR ("sema_t::pp_sema_clean_handle: sem_close() failed");
  if (sem->sem_hdl != U_SEM_INVALID_HDL &&
    sem->sem_created == true &&
    sem_unlink(sem->platform_key) == -1)
    U_ERROR ("sema_t::pp_sema_clean_handle: sem_unlink() failed");
  sem->sem_created = false;
  sem->sem_hdl = U_SEM_INVALID_HDL;
}

sema_t *
u_sema_new(const byte_t *name, int init_val, sema_access_t mod, err_t **error) {
  sema_t *ret;
  byte_t *new_name;

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
  strcat(new_name, U_SEM_SUFFIX);
#if defined (U_OS_IRIX) || defined (U_OS_TRU64)
  /* IRIX and Tru64 prefer filename styled IPC names */
  ret->platform_key = u_ipc_get_platform_key(new_name, false);
#else
  ret->platform_key = u_ipc_get_platform_key(new_name, true);
#endif
  ret->init_val = init_val;
  ret->mode = mod;
  u_free(new_name);
  if (U_UNLIKELY (pp_sema_create_handle(ret, error) == false)) {
    u_sema_free(ret);
    return NULL;
  }
  return ret;
}

void
u_sema_take_ownership(sema_t *sem) {
  if (U_UNLIKELY (sem == NULL)) {
    return;
  }
  sem->sem_created = true;
}

bool
u_sema_acquire(sema_t *sem,
  err_t **error) {
  bool ret;
  int res;
  if (U_UNLIKELY (sem == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  while ((res = sem_wait(sem->sem_hdl)) == -1
    && u_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call sem_wait() on semaphore"
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
  ret = (sem_post(sem->sem_hdl) == 0);
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call sem_post() on semaphore"
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
