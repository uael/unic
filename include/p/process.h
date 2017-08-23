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

/*!@file p/process.h
 * @brief Process information
 * @author Alexander Saprykin
 *
 * A process is an executing unit in an operating system with its own address
 * space. Every process can be identified with a unique identifier called PID.
 * To get a PID of the currently running process call
 * p_process_get_current_pid(). To check whether a process with a given PID is
 * running up use p_process_is_running().
 */
#ifndef P_PROCESS_H__
# define P_PROCESS_H__

#include "p/macros.h"
#include "p/types.h"

/*!@brief Gets a PID of the calling process.
 * @return PID of the calling process.
 * @since 0.0.1
 */
P_API u32_t
p_process_get_current_pid(void);

/*!@brief Checks whether a process with a given PID is running or not.
 * @param pid PID to check for.
 * @return TRUE if the process with the given PID exists and is running up,
 * FALSE otherwise.
 * @since 0.0.1
 */
P_API bool
p_process_is_running(u32_t pid);

#endif /* !P_PROCESS_H__ */

