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

#include "p/mem.h"
#include "p/shm.h"
#include "p/shmbuffer.h"

#include <stdlib.h>
#include <string.h>

#define P_SHM_BUFFER_READ_OFFSET  0
#define P_SHM_BUFFER_WRITE_OFFSET  sizeof (size_t)
#define P_SHM_BUFFER_DATA_OFFSET  sizeof (size_t) * 2

struct PShmBuffer_ {
  PShm *shm;
  size_t size;
};

static size_t pp_shm_buffer_get_free_space(PShmBuffer *buf);
static size_t pp_shm_buffer_get_used_space(PShmBuffer *buf);

/* Warning: this function is not thread-safe, only for internal usage */
static size_t
pp_shm_buffer_get_free_space(PShmBuffer *buf) {
  size_t read_pos, write_pos;
  ptr_t addr;

  addr = p_shm_get_address(buf->shm);

  memcpy(&read_pos, (byte_t *) addr + P_SHM_BUFFER_READ_OFFSET,
    sizeof(read_pos));
  memcpy(&write_pos, (byte_t *) addr + P_SHM_BUFFER_WRITE_OFFSET,
    sizeof(write_pos));

  if (write_pos < read_pos)
    return read_pos - write_pos;
  else if (write_pos > read_pos)
    return buf->size - (write_pos - read_pos) - 1;
  else
    return buf->size - 1;
}

static size_t
pp_shm_buffer_get_used_space(PShmBuffer *buf) {
  size_t read_pos, write_pos;
  ptr_t addr;

  addr = p_shm_get_address(buf->shm);

  memcpy(&read_pos, (byte_t *) addr + P_SHM_BUFFER_READ_OFFSET,
    sizeof(read_pos));
  memcpy(&write_pos, (byte_t *) addr + P_SHM_BUFFER_WRITE_OFFSET,
    sizeof(write_pos));

  if (write_pos > read_pos)
    return write_pos - read_pos;
  else if (write_pos < read_pos)
    return (buf->size - (read_pos - write_pos));
  else
    return 0;
}

P_API PShmBuffer *
p_shm_buffer_new(const byte_t *name,
  size_t size,
  p_err_t **error) {
  PShmBuffer *ret;
  PShm *shm;

  if (P_UNLIKELY (name == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return NULL;
  }

  if (P_UNLIKELY ((shm = p_shm_new(name,
    (size != 0) ? size + P_SHM_BUFFER_DATA_OFFSET + 1 : 0,
    P_SHM_ACCESS_READWRITE,
    error)) == NULL))
    return NULL;

  if (P_UNLIKELY (p_shm_get_size(shm) <= P_SHM_BUFFER_DATA_OFFSET + 1)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Too small memory segment to hold required data");
    p_shm_free(shm);
    return NULL;
  }

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PShmBuffer))) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_NO_RESOURCES,
      0,
      "Failed to allocate memory for shared buffer");
    p_shm_free(shm);
    return NULL;
  }

  ret->shm = shm;
  ret->size = p_shm_get_size(shm) - P_SHM_BUFFER_DATA_OFFSET;

  return ret;
}

P_API void
p_shm_buffer_free(PShmBuffer *buf) {
  if (P_UNLIKELY (buf == NULL))
    return;

  p_shm_free(buf->shm);
  p_free(buf);
}

P_API void
p_shm_buffer_take_ownership(PShmBuffer *buf) {
  if (P_UNLIKELY (buf == NULL))
    return;

  p_shm_take_ownership(buf->shm);
}

P_API int_t
p_shm_buffer_read(PShmBuffer *buf,
  ptr_t storage,
  size_t len,
  p_err_t **error) {
  size_t read_pos, write_pos;
  size_t data_aval, to_copy;
  uint_t i;
  ptr_t addr;

  if (P_UNLIKELY (buf == NULL || storage == NULL || len == 0)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return -1;
  }

  if (P_UNLIKELY ((addr = p_shm_get_address(buf->shm)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Unable to get shared memory address");
    return -1;
  }

  if (P_UNLIKELY (p_shm_lock(buf->shm, error) == false))
    return -1;

  memcpy(&read_pos, (byte_t *) addr + P_SHM_BUFFER_READ_OFFSET,
    sizeof(read_pos));
  memcpy(&write_pos, (byte_t *) addr + P_SHM_BUFFER_WRITE_OFFSET,
    sizeof(write_pos));

  if (read_pos == write_pos) {
    if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
      return -1;

    return 0;
  }

  data_aval = pp_shm_buffer_get_used_space(buf);
  to_copy = (data_aval <= len) ? data_aval : len;

  for (i = 0; i < to_copy; ++i)
    memcpy((byte_t *) storage + i,
      (byte_t *) addr + P_SHM_BUFFER_DATA_OFFSET + ((read_pos + i) % buf->size),
      1);

  read_pos = (read_pos + to_copy) % buf->size;
  memcpy((byte_t *) addr + P_SHM_BUFFER_READ_OFFSET, &read_pos,
    sizeof(read_pos));

  if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
    return -1;

  return (int_t) to_copy;
}

P_API ssize_t
p_shm_buffer_write(PShmBuffer *buf,
  ptr_t data,
  size_t len,
  p_err_t **error) {
  size_t read_pos, write_pos;
  uint_t i;
  ptr_t addr;

  if (P_UNLIKELY (buf == NULL || data == NULL || len == 0)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return -1;
  }

  if (P_UNLIKELY ((addr = p_shm_get_address(buf->shm)) == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Unable to get shared memory address");
    return -1;
  }

  if (P_UNLIKELY (p_shm_lock(buf->shm, error) == false))
    return -1;

  memcpy(&read_pos, (byte_t *) addr + P_SHM_BUFFER_READ_OFFSET,
    sizeof(read_pos));
  memcpy(&write_pos, (byte_t *) addr + P_SHM_BUFFER_WRITE_OFFSET,
    sizeof(write_pos));

  if (pp_shm_buffer_get_free_space(buf) < len) {
    if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
      return -1;

    return 0;
  }

  for (i = 0; i < len; ++i)
    memcpy(
      (byte_t *) addr + P_SHM_BUFFER_DATA_OFFSET + ((write_pos + i) % buf->size),
      (byte_t *) data + i,
      1);

  write_pos = (write_pos + len) % buf->size;
  memcpy((byte_t *) addr + P_SHM_BUFFER_WRITE_OFFSET, &write_pos,
    sizeof(write_pos));

  if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
    return -1;

  return (ssize_t) len;
}

P_API ssize_t
p_shm_buffer_get_free_space(PShmBuffer *buf,
  p_err_t **error) {
  size_t space;

  if (P_UNLIKELY (buf == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return -1;
  }

  if (P_UNLIKELY (p_shm_lock(buf->shm, error) == false))
    return -1;

  space = pp_shm_buffer_get_free_space(buf);

  if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
    return -1;

  return (ssize_t) space;
}

P_API ssize_t
p_shm_buffer_get_used_space(PShmBuffer *buf,
  p_err_t **error) {
  size_t space;

  if (P_UNLIKELY (buf == NULL)) {
    p_error_set_error_p(error,
      (int_t) P_ERR_IPC_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return -1;
  }

  if (P_UNLIKELY (p_shm_lock(buf->shm, error) == false))
    return -1;

  space = pp_shm_buffer_get_used_space(buf);

  if (P_UNLIKELY (p_shm_unlock(buf->shm, error) == false))
    return -1;

  return (ssize_t) space;
}

P_API void
p_shm_buffer_clear(PShmBuffer *buf) {
  ptr_t addr;
  size_t size;

  if (P_UNLIKELY (buf == NULL))
    return;

  if (P_UNLIKELY ((addr = p_shm_get_address(buf->shm)) == NULL)) {
    P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_get_address() failed");
    return;
  }

  size = p_shm_get_size(buf->shm);

  if (P_UNLIKELY (p_shm_lock(buf->shm, NULL) == false)) {
    P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_lock() failed");
    return;
  }

  memset(addr, 0, size);

  if (P_UNLIKELY (p_shm_unlock(buf->shm, NULL) == false))
    P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_unlock() failed");
}
