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

#include "p/err.h"
#include "p/mem.h"
#include "p/sema.h"
#include "p/string.h"
#include "err-private.h"
#include "ipc-private.h"

#define P_SEM_SUFFIX    "_p_sem_object"
#define P_SEM_INVALID_HDL  -1

struct sembuf sem_lock = {0, -1, SEM_UNDO};

struct sembuf sem_unlock = {0, 1, SEM_UNDO};

typedef union p_semun_ {
  int val;
  struct semid_ds *buf;
  ushort_t *array;
} p_semun;

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
  p_semun semun_op;
  if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (
    (built = p_ipc_unix_create_key_file(sem->platform_key)) == -1)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to create key file"
    );
    pp_sema_clean_handle(sem);
    return false;
  } else if (built == 0) {
    sem->file_created = true;
  }
  if (P_UNLIKELY (
    (sem->unix_key = p_ipc_unix_get_ftok_key(sem->platform_key)) == -1)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
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
    )) == P_SEM_INVALID_HDL) {
    if (p_err_get_last_system() == EEXIST) {
      sem->sem_hdl = semget(sem->unix_key, 1, 0660);
    }
  } else {
    sem->sem_created = true;

    /* Maybe key file left after the crash, so take it */
    sem->file_created = (built == 1);
  }
  if (P_UNLIKELY (sem->sem_hdl == P_SEM_INVALID_HDL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call semget() to create semaphore"
    );
    pp_sema_clean_handle(sem);
    return false;
  }
  if (sem->sem_created == true || sem->mode == P_SEMA_CREATE) {
    semun_op.val = sem->init_val;
    if (P_UNLIKELY (semctl(sem->sem_hdl, 0, SETVAL, semun_op) == -1)) {
      p_err_set_err_p(
        error,
        (int) p_err_get_last_ipc(),
        p_err_get_last_system(),
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
  if (sem->sem_hdl != P_SEM_INVALID_HDL &&
    sem->sem_created == true &&
    semctl(sem->sem_hdl, 0, IPC_RMID) == -1)
    P_ERROR (
      "sema_t::pp_sema_clean_handle: semctl() with IPC_RMID failed");
  if (sem->file_created == true &&
    sem->platform_key != NULL &&
    unlink(sem->platform_key) == -1)
    P_ERROR ("sema_t::pp_sema_clean_handle: unlink() failed");
  sem->file_created = false;
  sem->sem_created = false;
  sem->unix_key = -1;
  sem->sem_hdl = P_SEM_INVALID_HDL;
}

sema_t *
p_sema_new(const byte_t *name,
  int init_val,
  sema_access_t mode,
  err_t **error) {
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
  ret->platform_key = p_ipc_get_platform_key(new_name, false);
  ret->init_val = init_val;
  ret->mode = mode;
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
  while ((res = semop(sem->sem_hdl, &sem_lock, 1)) == -1 &&
    p_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (P_UNLIKELY (ret == false &&
    (
      p_err_get_last_system() == EIDRM ||
        p_err_get_last_system() == EINVAL
    ))) {
    P_WARNING ("sema_t::p_sema_acquire: trying to recreate");
    pp_sema_clean_handle(sem);
    if (P_UNLIKELY (pp_sema_create_handle(sem, error) == false)) {
      return false;
    }
    while ((res = semop(sem->sem_hdl, &sem_lock, 1)) == -1 &&
      p_err_get_last_system() == EINTR) {}
    ret = (res == 0);
  }
  if (P_UNLIKELY (ret == false)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call semop() on semaphore"
    );
  }
  return ret;
}

bool
p_sema_release(sema_t *sem,
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
  while ((res = semop(sem->sem_hdl, &sem_unlock, 1)) == -1 &&
    p_err_get_last_system() == EINTR) {}
  ret = (res == 0);
  if (P_UNLIKELY (ret == false &&
    (
      p_err_get_last_system() == EIDRM ||
        p_err_get_last_system() == EINVAL
    ))) {
    P_WARNING ("sema_t::p_sema_release: trying to recreate");
    pp_sema_clean_handle(sem);
    if (P_UNLIKELY (pp_sema_create_handle(sem, error) == false)) {
      return false;
    }
    return true;
  }
  if (P_UNLIKELY (ret == false)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_ipc(),
      p_err_get_last_system(),
      "Failed to call semop() on semaphore"
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
