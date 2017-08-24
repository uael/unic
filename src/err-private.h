/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef UNIC_HEADER_PERROR_PRIVATE_H
# define UNIC_HEADER_PERROR_PRIVATE_H

#include "unic/err.h"
#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Gets an IO error code from a system error code.
 * @param err_code System error code.
 * @return IO error code.
 */
err_io_t
u_err_get_io_from_system(int err_code);

/*!@brief Gets an IO error code from the last call result.
 * @return IO error code.
 */
err_io_t
u_err_get_last_io(void);

/*!@brief Gets an IPC error code from a system error code
 * @param err_code System error code.
 * @return IPC error code.
 */
err_ipc_t
u_err_get_ipc_from_system(int err_code);

/*!@brief Gets an IPC error code from the last call result.
 * @return IPC error code.
 */
U_API err_ipc_t
u_err_get_last_ipc(void);

#endif /* UNIC_HEADER_PERROR_PRIVATE_H */
