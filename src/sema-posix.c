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

#include "p/err.h"
#include "p/mem.h"
#include "p/sema.h"
#include "err-private.h"
#include "ipc-private.h"

#define P_SEM_SUFFIX    "_p_sem_object"

typedef sem_t sema_hdl_t;

/* On some HP-UX versions it may not be defined */
#ifndef SEM_FAILED
# define SEM_FAILED ((sema_t *) -1)
#endif

#ifdef P_OS_SOLARIS
# define P_SEM_INVALID_HDL (sema_t *) -1
#else
# define P_SEM_INVALID_HDL  SEM_FAILED
#endif

struct sema {
  bool sem_created;
  byte_t *platform_key;
#if defined (P_OS_VMS) && (PLIBSYS_SIZEOF_VOID_P == 4)
# pragma __pointer_size 64
#endif
  sema_hdl_t *sem_hdl;
#if defined (P_OS_VMS) && (PLIBSYS_SIZEOF_VOID_P == 4)
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
  if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_INVALID_ARGUMENT,
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
    )) == P_SEM_INVALID_HDL &&
    p_err_get_last_system() == EINTR) {}
  if (sem->sem_hdl == P_SEM_INVALID_HDL) {
    if (p_err_get_last_system() == EEXIST) {
      if (sem->mode == P_SEMA_CREATE) {
        sem_unlink(sem->platform_key);
      }
      while ((
        sem->sem_hdl = sem_open(
          sem->platform_key,
          0,
          0,
          0
        )) == P_SEM_INVALID_HDL &&
        p_err_get_last_system() == EINTR) {}
    }
  } else {
    sem->sem_created = true;
  }
  if (P_UNLIKELY (sem->sem_hdl == P_SEM_INVALID_HDL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call sem_open() to create semaphore"
    );
    pp_sema_clean_handle(sem);
    return false;
  }
  return true;
}

static void
pp_sema_clean_handle(sema_t *sem) {
  if (P_UNLIKELY (sem->sem_hdl != P_SEM_INVALID_HDL &&
    sem_close(sem->sem_hdl) == -1))
    P_ERROR ("sema_t::pp_sema_clean_handle: sem_close() failed");
  if (sem->sem_hdl != P_SEM_INVALID_HDL &&
    sem->sem_created == true &&
    sem_unlink(sem->platform_key) == -1)
    P_ERROR ("sema_t::pp_sema_clean_handle: sem_unlink() failed");
  sem->sem_created = false;
  sem->sem_hdl = P_SEM_INVALID_HDL;
}

sema_t *
p_sema_new(const byte_t *name, int init_val, sema_access_t mod, err_t **error) {
  sema_t *ret;
  byte_t *new_name;

  if (P_UNLIKELY (name == NULL || init_val < 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(sema_t))) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    return NULL;
  }
  if (P_UNLIKELY (
    (new_name = p_malloc0(strlen(name) + strlen(P_SEM_SUFFIX) + 1)) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    p_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcat(new_name, P_SEM_SUFFIX);
#if defined (P_OS_IRIX) || defined (P_OS_TRU64)
  /* IRIX and Tru64 prefer filename styled IPC names */
  ret->platform_key = p_ipc_get_platform_key(new_name, false);
#else
  ret->platform_key = p_ipc_get_platform_key(new_name, true);
#endif
  ret->init_val = init_val;
  ret->mode = mod;
  p_free(new_name);
  if (P_UNLIKELY (pp_sema_create_handle(ret, error) == false)) {
    p_sema_free(ret);
    return NULL;
  }
  return ret;
}

void
p_sema_take_ownership(sema_t *sem) {
  if (P_UNLIKELY (sem == NULL)) {
    return;
  }
  sem->sem_created = true;
}

bool
p_sema_acquire(sema_t *sem,
  err_t **error) {
  bool ret;
  int res;
  if (P_UNLIKELY (sem == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  while ((res = sem_wait(sem->sem_hdl)) == -1
    && p_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (P_UNLIKELY (ret == false)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call sem_wait() on semaphore"
    );
  }
  return ret;
}

bool
p_sema_release(sema_t *sem,
  err_t **error) {
  bool ret;
  if (P_UNLIKELY (sem == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  ret = (sem_post(sem->sem_hdl) == 0);
  if (P_UNLIKELY (ret == false)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call sem_post() on semaphore"
    );
  }
  return ret;
}

void
p_sema_free(sema_t *sem) {
  if (P_UNLIKELY (sem == NULL)) {
    return;
  }
  pp_sema_clean_handle(sem);
  if (P_LIKELY (sem->platform_key != NULL)) {
    p_free(sem->platform_key);
  }
  p_free(sem);
}
