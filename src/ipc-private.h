/*
 * Copyright (C) 2016-2017 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef UNIC_HEADER_PIPC_PRIVATE_H
# define UNIC_HEADER_PIPC_PRIVATE_H

#include "unic/macros.h"
#include "unic/types.h"

#if !defined (U_OS_WIN) && !defined (U_OS_OS2)
/*!@brief Gets a temporary directory on UNIX systems.
 * @return Temporary directory.
 */
byte_t *u_ipc_unix_get_temp_dir(void);

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
/*!@brief Creates a file for System V IPC usage.
 * @param file_name File name to create.
 * @return -1 in case of error, 0 if all was OK, and 1 if the file already
 * exists.
 */
int u_ipc_unix_create_key_file(const byte_t *file_name);

/*!@brief Wrapps the ftok() UNIX call for a unique IPC key.
 * @param file_name File name for ftok() call.
 * @return Key in case of success, -1 otherwise.
 */
int u_ipc_unix_get_ftok_key(const byte_t *file_name);
#endif /* !U_OS_WIN && !U_OS_OS2 */

/*!@brief Generates a platform independent key for IPC usage, an object name for
 * Windows and a file name to use with ftok () for UNIX-like systems.
 * @param name Object name.
 * @param posix true if the key will be used for the POSIX IPC calls, otherwise
 * false. This parameter is not used on the Windows platform.
 * @return Platform independent key for IPC usage.
 */
byte_t *
u_ipc_get_platform_key(const byte_t *name,
  bool posix);

#endif /* UNIC_HEADER_PIPC_PRIVATE_H */
