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
#include <errno.h>
#include <sys/sem.h>

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/sema.h"
#include "unic/string.h"
#include "err-private.h"
#include "ipc-private.h"

#define U_SEM_SUFFIX    "_p_sem_object"
#define U_SEM_INVALID_HDL  (-1)

struct sembuf sem_lock = {0, -1, SEM_UNDO};

struct sembuf sem_unlock = {0, 1, SEM_UNDO};

typedef union semun {
  int val;
  struct semid_ds *buf;
  ushort_t *array;
} semun_t;

typedef int sema_hdl_t;

struct sema {
  bool file_created;
  bool sem_created;
  key_t unix_key;
  byte_t *platform_key;
  sema_hdl_t sem_hdl;
  sema_access_t mode;
  int init_val;
};

static bool
pp_sema_create_handle(sema_t *sem, err_t **error);

static void
pp_sema_clean_handle(sema_t *sem);

static bool
pp_sema_create_handle(sema_t *sem, err_t **error) {
  int built;
  semun_t semun_op;
  
  if (U_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (U_UNLIKELY (
    (built = u_ipc_unix_create_key_file(sem->platform_key)) == -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to create key file"
    );
    pp_sema_clean_handle(sem);
    return false;
  } else if (built == 0) {
    sem->file_created = true;
  }
  if (U_UNLIKELY (
    (sem->unix_key = u_ipc_unix_get_ftok_key(sem->platform_key)) == -1)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to get unique IPC key"
    );
    pp_sema_clean_handle(sem);
    return false;
  }
  if ((
    sem->sem_hdl = semget(
      sem->unix_key,
      1,
      IPC_CREAT | IPC_EXCL | 0660
    )) == U_SEM_INVALID_HDL) {
    if (u_err_get_last_system() == EEXIST) {
      sem->sem_hdl = semget(sem->unix_key, 1, 0660);
    }
  } else {
    sem->sem_created = true;

    /* Maybe key file left after the crash, so take it */
    sem->file_created = (built == 1);
  }
  if (U_UNLIKELY (sem->sem_hdl == U_SEM_INVALID_HDL)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call semget() to create semaphore"
    );
    pp_sema_clean_handle(sem);
    return false;
  }
  if (sem->sem_created == true || sem->mode == U_SEMA_CREATE) {
    semun_op.val = sem->init_val;
    if (U_UNLIKELY (semctl(sem->sem_hdl, 0, SETVAL, semun_op) == -1)) {
      u_err_set_err_p(
        error,
        (int) u_err_get_last_ipc(),
        u_err_get_last_system(),
        "Failed to set semaphore initial value with semctl()"
      );
      pp_sema_clean_handle(sem);
      return false;
    }
  }
  return true;
}

static void
pp_sema_clean_handle(sema_t *sem) {
  if (sem->sem_hdl != U_SEM_INVALID_HDL
    && sem->sem_created == true 
    && semctl(sem->sem_hdl, 0, IPC_RMID) == -1)
    U_ERROR ("sema_t::pp_sema_clean_handle: semctl() with IPC_RMID failed");
  if (sem->file_created == true && sem->platform_key != NULL 
    && unlink(sem->platform_key) == -1)
    U_ERROR ("sema_t::pp_sema_clean_handle: unlink() failed");
  sem->file_created = false;
  sem->sem_created = false;
  sem->unix_key = -1;
  sem->sem_hdl = U_SEM_INVALID_HDL;
}

sema_t *
u_sema_new(const byte_t *name, int init_val, sema_access_t mode, err_t **err) {
  sema_t *ret;
  byte_t *new_name;
  
  if (U_UNLIKELY (name == NULL || init_val < 0)) {
    u_err_set_err_p(
      err,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(sema_t))) == NULL)) {
    u_err_set_err_p(
      err,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    return NULL;
  }
  if (U_UNLIKELY (
    (new_name = u_malloc0(strlen(name) + strlen(U_SEM_SUFFIX) + 1)) == NULL)) {
    u_err_set_err_p(
      err,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for semaphore"
    );
    u_free(ret);
    return NULL;
  }
  strcpy(new_name, name);
  strcat(new_name, U_SEM_SUFFIX);
  ret->platform_key = u_ipc_get_platform_key(new_name, false);
  ret->init_val = init_val;
  ret->mode = mode;
  u_free(new_name);
  if (U_UNLIKELY (pp_sema_create_handle(ret, err) == false)) {
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
  while ((res = semop(sem->sem_hdl, &sem_lock, 1)) == -1 &&
    u_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (U_UNLIKELY (ret == false &&
    (
      u_err_get_last_system() == EIDRM ||
        u_err_get_last_system() == EINVAL
    ))) {
    U_WARNING ("sema_t::u_sema_acquire: trying to recreate");
    pp_sema_clean_handle(sem);
    if (U_UNLIKELY (pp_sema_create_handle(sem, error) == false)) {
      return false;
    }
    while ((res = semop(sem->sem_hdl, &sem_lock, 1)) == -1 &&
      u_err_get_last_system() == EINTR) {}
    ret = (res == 0);
  }
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call semop() on semaphore"
    );
  }
  return ret;
}

bool
u_sema_release(sema_t *sem, err_t **error) {
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
  while ((res = semop(sem->sem_hdl, &sem_unlock, 1)) == -1 &&
    u_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (U_UNLIKELY (ret == false &&
    (
      u_err_get_last_system() == EIDRM ||
        u_err_get_last_system() == EINVAL
    ))) {
    U_WARNING ("sema_t::u_sema_release: trying to recreate");
    pp_sema_clean_handle(sem);
    if (U_UNLIKELY (pp_sema_create_handle(sem, error) == false)) {
      return false;
    }
    return true;
  }
  if (U_UNLIKELY (ret == false)) {
    u_err_set_err_p(
      error,
      (int) u_err_get_last_ipc(),
      u_err_get_last_system(),
      "Failed to call semop() on semaphore"
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
