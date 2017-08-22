/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file p/shmbuffer.h
 * @brief Shared memory buffer
 * @author Alexander Saprykin
 *
 * A shared memory buffer works like any other buffer but it is built upon a
 * shared memory region instead of the process-only address space. Thus it
 * inherits all the advantages and disadvantages of shared memory behavior. You
 * should read about #PShm before using this buffer implementation to understand
 * underlying restrictions.
 *
 * The shared memory buffer is process-wide and identified by its name across
 * the system, thus it can be opened by any process if it knows its name. Use
 * p_shm_buffer_new() to open the shared memory buffer and p_shm_buffer_free()
 * to close it.
 *
 * All read/write operations are completely thread- and process-safe, which
 * means that no other synchronization primitive is required, even for inter-
 * process access. A #PShm locking mechanism is used for access synchronization.
 *
 * The buffer is cyclic and non-overridable which means that you wouldn't get
 * buffer overflow and wouldn't override previously written data until reading
 * it.
 *
 * The read operation checks whether there is any data available and reads it in
 * case of successful check. After reading the data used space in the buffer is
 * marked as free and any subsequent write operation may overwrite it. Thus you
 * couldn't read the same data twice. The read operation is performed with the
 * p_shm_buffer_read() call.
 *
 * The write operation checks whether there is enough free space available and
 * writes a given memory block only if the buffer has enough free space.
 * Otherwise no data is written. The write operation is performed with the
 * p_shm_buffer_write() call.
 *
 * Data can be read and written into the buffer only sequentially. There is no
 * way to access an arbitrary address inside the buffer.
 *
 * You can take ownership of the shared memory buffer with
 * p_shm_buffer_take_ownership() to explicitly remove it from the system after
 * closing. Please refer to the #PShm description to understand the intention of
 * this action.
 */
#ifndef P_SHMBUF_H__
# define P_SHMBUF_H__

#include "p/types.h"
#include "p/macros.h"
#include "p/err.h"

/*!@brief Shared memory buffer opaque data structure. */
typedef struct shmbuf shmbuf_t;

/*!@brief Creates a new #PShmBuffer structure.
 * @param name Unique buffer name.
 * @param size Buffer size in bytes, can't be changed later.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to the #PShmBuffer structure in case of success, NULL
 * otherwise.
 * @since 0.0.1
 *
 * If a buffer with the same name already exists then the @a size will be
 * ignored and the existing buffer will be returned.
 */
P_API shmbuf_t *
p_shm_buffer_new(const byte_t *name, size_t size, err_t **error);

/*!@brief Frees #PShmBuffer structure.
 * @param buf #PShmBuffer to free.
 * @since 0.0.1
 *
 * Note that a buffer will be completely removed from the system only after the
 * last instance of the buffer with the same name is closed.
 */
P_API void
p_shm_buffer_free(shmbuf_t *buf);

/*!@brief Takes ownership of a shared memory buffer.
 * @param buf Shared memory buffer.
 * @since 0.0.1
 *
 * If you take ownership of the shared memory buffer, p_shm_buffer_free() will
 * try to completely unlink it and remove from the system. This is useful on
 * UNIX systems, where shared memory can survive an application crash. On the
 * Windows and OS/2 platforms this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * memory segment from the previous crash can be removed from the system. To do
 * that, call p_shm_buffer_new() and check if its condition is normal (used
 * space, free space). If not, take ownership of the shared memory buffer object
 * and remove it with the p_shm_buffer_free() call. After that, create it again.
 */
P_API void
p_shm_buffer_take_ownership(shmbuf_t *buf);

/*!@brief Tries to read data from a shared memory buffer.
 * @param buf #PShmBuffer to read data from.
 * @param[out] storage Output buffer to put data in.
 * @param len Storage size in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Number of read bytes (can be 0 if buffer is empty), or -1 if error
 * occured.
 * @since 0.0.1
 */
P_API int_t
p_shm_buffer_read(shmbuf_t *buf, ptr_t storage, size_t len, err_t **error);

/*!@brief Tries to write data into a shared memory buffer.
 * @param buf #PShmBuffer to write data into.
 * @param data Data to write.
 * @param len Data size in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Number of written bytes (can be 0 if buffer is full), or -1 if error
 * occured.
 * @since 0.0.1
 * @note Write operation is performed only if the buffer has enough space for
 * the given data size.
 */
P_API ssize_t
p_shm_buffer_write(shmbuf_t *buf, ptr_t data, size_t len, err_t **error);

/*!@brief Gets free space in the shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @param[out] error Error report object, NULL to ignore.
 * @return Free space in bytes in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_API ssize_t
p_shm_buffer_get_free_space(shmbuf_t *buf, err_t **error);

/*!@brief Gets used space in the shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @param[out] error Error report object, NULL to ignore.
 * @return Used space in bytes in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_API ssize_t
p_shm_buffer_get_used_space(shmbuf_t *buf, err_t **error);

/*!@brief Clears all data in the buffer and fills it with zeros.
 * @param buf #PShmBuffer to clear.
 * @since 0.0.1
 */
P_API void
p_shm_buffer_clear(shmbuf_t *buf);

#endif /* !P_SHMBUF_H__ */
