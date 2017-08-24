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

#include "unic/mem.h"
#include "unic/shm.h"
#include "unic/shmbuf.h"

#define U_SHMBUF_READ_OFFSET  0
#define U_SHMBUF_WRITE_OFFSET  sizeof (size_t)
#define U_SHMBUF_DATA_OFFSET  sizeof (size_t) * 2

struct shmbuf {
  shm_t *shm;
  size_t size;
};

static size_t
pp_shmbuf_get_free_space(shmbuf_t *buf);

static size_t
pp_shmbuf_get_used_space(shmbuf_t *buf);

/* Warning: this function is not thread-safe, only for internal usage */
static size_t
pp_shmbuf_get_free_space(shmbuf_t *buf) {
  size_t read_pos, write_pos;
  ptr_t addr;
  addr = u_shm_get_address(buf->shm);
  memcpy(
    &read_pos, (byte_t *) addr + U_SHMBUF_READ_OFFSET,
    sizeof(read_pos));
  memcpy(
    &write_pos, (byte_t *) addr + U_SHMBUF_WRITE_OFFSET,
    sizeof(write_pos));
  if (write_pos < read_pos) {
    return read_pos - write_pos;
  } else if (write_pos > read_pos) {
    return buf->size - (write_pos - read_pos) - 1;
  } else {
    return buf->size - 1;
  }
}

static size_t
pp_shmbuf_get_used_space(shmbuf_t *buf) {
  size_t read_pos, write_pos;
  ptr_t addr;
  addr = u_shm_get_address(buf->shm);
  memcpy(
    &read_pos, (byte_t *) addr + U_SHMBUF_READ_OFFSET,
    sizeof(read_pos));
  memcpy(
    &write_pos, (byte_t *) addr + U_SHMBUF_WRITE_OFFSET,
    sizeof(write_pos));
  if (write_pos > read_pos) {
    return write_pos - read_pos;
  } else if (write_pos < read_pos) {
    return (buf->size - (read_pos - write_pos));
  } else {
    return 0;
  }
}

shmbuf_t *
u_shmbuf_new(const byte_t *name,
  size_t size,
  err_t **error) {
  shmbuf_t *ret;
  shm_t *shm;
  if (U_UNLIKELY (name == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (U_UNLIKELY ((
    shm = u_shm_new(
      name,
      (size != 0) ? size + U_SHMBUF_DATA_OFFSET + 1 : 0,
      U_SHM_ACCESS_READWRITE,
      error
    )) == NULL)) {
      return NULL;
  }
  if (U_UNLIKELY (u_shm_get_size(shm) <= U_SHMBUF_DATA_OFFSET + 1)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Too small memory segment to hold required data"
    );
    u_shm_free(shm);
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(shmbuf_t))) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared buffer"
    );
    u_shm_free(shm);
    return NULL;
  }
  ret->shm = shm;
  ret->size = u_shm_get_size(shm) - U_SHMBUF_DATA_OFFSET;
  return ret;
}

void
u_shmbuf_free(shmbuf_t *buf) {
  if (U_UNLIKELY (buf == NULL)) {
    return;
  }
  u_shm_free(buf->shm);
  u_free(buf);
}

void
u_shmbuf_take_ownership(shmbuf_t *buf) {
  if (U_UNLIKELY (buf == NULL)) {
    return;
  }
  u_shm_take_ownership(buf->shm);
}

int
u_shmbuf_read(shmbuf_t *buf,
  ptr_t storage,
  size_t len,
  err_t **error) {
  size_t read_pos, write_pos;
  size_t data_aval, to_copy;
  uint_t i;
  ptr_t addr;
  if (U_UNLIKELY (buf == NULL || storage == NULL || len == 0)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (U_UNLIKELY ((addr = u_shm_get_address(buf->shm)) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Unable to get shared memory address"
    );
    return -1;
  }
  if (U_UNLIKELY (u_shm_lock(buf->shm, error) == false)) {
    return -1;
  }
  memcpy(
    &read_pos, (byte_t *) addr + U_SHMBUF_READ_OFFSET,
    sizeof(read_pos));
  memcpy(
    &write_pos, (byte_t *) addr + U_SHMBUF_WRITE_OFFSET,
    sizeof(write_pos));
  if (read_pos == write_pos) {
    if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
      return -1;
    }
    return 0;
  }
  data_aval = pp_shmbuf_get_used_space(buf);
  to_copy = (data_aval <= len) ? data_aval : len;
  for (i = 0; i < to_copy; ++i) {
    memcpy((byte_t *) storage + i,
      (byte_t *) addr + U_SHMBUF_DATA_OFFSET + ((read_pos + i) % buf->size),
      1
    );
  }
  read_pos = (read_pos + to_copy) % buf->size;
  memcpy((byte_t *) addr + U_SHMBUF_READ_OFFSET, &read_pos,
    sizeof(read_pos));
  if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
    return -1;
  }
  return (int) to_copy;
}

ssize_t
u_shmbuf_write(shmbuf_t *buf,
  ptr_t data,
  size_t len,
  err_t **error) {
  size_t read_pos, write_pos;
  uint_t i;
  ptr_t addr;
  if (U_UNLIKELY (buf == NULL || data == NULL || len == 0)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (U_UNLIKELY ((addr = u_shm_get_address(buf->shm)) == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Unable to get shared memory address"
    );
    return -1;
  }
  if (U_UNLIKELY (u_shm_lock(buf->shm, error) == false)) {
    return -1;
  }
  memcpy(
    &read_pos, (byte_t *) addr + U_SHMBUF_READ_OFFSET,
    sizeof(read_pos));
  memcpy(
    &write_pos, (byte_t *) addr + U_SHMBUF_WRITE_OFFSET,
    sizeof(write_pos));
  if (pp_shmbuf_get_free_space(buf) < len) {
    if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
      return -1;
    }
    return 0;
  }
  for (i = 0; i < len; ++i) {
    memcpy(
      (byte_t *) addr + U_SHMBUF_DATA_OFFSET +
        ((write_pos + i) % buf->size),
      (byte_t *) data + i,
      1
    );
  }
  write_pos = (write_pos + len) % buf->size;
  memcpy((byte_t *) addr + U_SHMBUF_WRITE_OFFSET, &write_pos,
    sizeof(write_pos));
  if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
    return -1;
  }
  return (ssize_t) len;
}

ssize_t
u_shmbuf_get_free_space(shmbuf_t *buf,
  err_t **error) {
  size_t space;
  if (U_UNLIKELY (buf == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (U_UNLIKELY (u_shm_lock(buf->shm, error) == false)) {
    return -1;
  }
  space = pp_shmbuf_get_free_space(buf);
  if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
    return -1;
  }
  return (ssize_t) space;
}

ssize_t
u_shmbuf_get_used_space(shmbuf_t *buf,
  err_t **error) {
  size_t space;
  if (U_UNLIKELY (buf == NULL)) {
    u_err_set_err_p(
      error,
      (int) U_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (U_UNLIKELY (u_shm_lock(buf->shm, error) == false)) {
    return -1;
  }
  space = pp_shmbuf_get_used_space(buf);
  if (U_UNLIKELY (u_shm_unlock(buf->shm, error) == false)) {
    return -1;
  }
  return (ssize_t) space;
}

void
u_shmbuf_clear(shmbuf_t *buf) {
  ptr_t addr;
  size_t size;
  if (U_UNLIKELY (buf == NULL)) {
    return;
  }
  if (U_UNLIKELY ((addr = u_shm_get_address(buf->shm)) == NULL)) {
    U_ERROR ("shmbuf_t::u_shmbuf_clear: u_shm_get_address() failed");
    return;
  }
  size = u_shm_get_size(buf->shm);
  if (U_UNLIKELY (u_shm_lock(buf->shm, NULL) == false)) {
    U_ERROR ("shmbuf_t::u_shmbuf_clear: u_shm_lock() failed");
    return;
  }
  memset(addr, 0, size);
  if (U_UNLIKELY (u_shm_unlock(buf->shm, NULL) == false))
    U_ERROR ("shmbuf_t::u_shmbuf_clear: u_shm_unlock() failed");
}
