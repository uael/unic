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

/**
 * @file p/err.h
 * @brief Error data types
 * @author Alexander Saprykin
 *
 * All error codes are splitted into the several domains. Every error should
 * belong to one of the domains described in #PErrorDomain. Think of an error
 * domain as a logical subsystem.
 *
 * Every error domain has its own enumeration with the list of possible error
 * codes. System error codes are converted to specified domains using internal
 * routines.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef P_ERR_H__
#define P_ERR_H__

#include "p/macros.h"

/** Enum with error domains. */
typedef enum p_err_domain {
  P_ERR_DOMAIN_NONE = 0,  /**< No domain was specified.				*/
  P_ERR_DOMAIN_IO = 500,  /**< Input/output domain.				*/
  P_ERR_DOMAIN_IPC = 600  /**< Interprocess communication domain.			*/
} p_err_domain_t;

/** Enum with IO errors. */
typedef enum p_err_io {
  P_ERR_IO_NONE = 500,  /**< No error.						*/
  P_ERR_IO_NO_RESOURCES =
  501,  /**< Operating system hasn't enough resources.		*/
  P_ERR_IO_NOT_AVAILABLE = 502,  /**< Resource isn't available.				*/
  P_ERR_IO_ACCESS_DENIED = 503,  /**< Access denied.					*/
  P_ERR_IO_CONNECTED = 504,  /**< Already connected.					*/
  P_ERR_IO_IN_PROGRESS = 505,  /**< Operation in progress.				*/
  P_ERR_IO_ABORTED = 506,  /**< Operation aborted.					*/
  P_ERR_IO_INVALID_ARGUMENT = 507,  /**< Invalid argument specified.			*/
  P_ERR_IO_NOT_SUPPORTED = 508,  /**< Operation not supported.				*/
  P_ERR_IO_TIMED_OUT = 509,  /**< Operation timed out.				*/
  P_ERR_IO_WOULD_BLOCK =
  510,  /**< Operation cannot be completed immediatly.		*/
  P_ERR_IO_ADDRESS_IN_USE = 511,  /**< Address is already under usage.			*/
  P_ERR_IO_CONNECTION_REFUSED = 512,  /**< Connection refused.				*/
  P_ERR_IO_NOT_CONNECTED = 513,  /**< Connection required first.				*/
  P_ERR_IO_QUOTA = 514,  /**< User quota exceeded.				*/
  P_ERR_IO_IS_DIRECTORY = 515,  /**< Trying to open directory for writing.		*/
  P_ERR_IO_NOT_DIRECTORY =
  516,  /**< Component of the path prefix is not a directory.	*/
  P_ERR_IO_NAMETOOLONG = 517,  /**< Specified name is too long.			*/
  P_ERR_IO_EXISTS = 518,  /**< Specified entry already exists.			*/
  P_ERR_IO_NOT_EXISTS = 519,  /**< Specified entry doesn't exist.			*/
  P_ERR_IO_NO_MORE = 520,  /**< No more data left.					*/
  P_ERR_IO_NOT_IMPLEMENTED = 521,  /**< Operation is not implemented.			*/
  P_ERR_IO_FAILED = 522  /**< General error.					*/
} p_err_io_t;

/** Enum with IPC errors */
typedef enum p_err_ipc {
  P_ERR_IPC_NONE = 600,  /**< No error.						*/
  P_ERR_IPC_ACCESS =
  601,  /**< Not enough rights to access object or its key.	*/
  P_ERR_IPC_EXISTS = 602,  /**< Object already exists and no proper open flags
						     were specified.					*/
  P_ERR_IPC_NOT_EXISTS = 603,  /**< Object doesn't exist or was removed before, and
						     no proper create flags were specified.		*/
  P_ERR_IPC_NO_RESOURCES = 604,  /**< Not enough system resources or memory to perform
						     operation.						*/
  P_ERR_IPC_OVERFLOW = 605,  /**< Semaphore value overflow.				*/
  P_ERR_IPC_NAMETOOLONG = 606,  /**< Object name is too long.				*/
  P_ERR_IPC_INVALID_ARGUMENT =
  607,  /**< Invalid argument (parameter) specified.		*/
  P_ERR_IPC_NOT_IMPLEMENTED = 608,  /**< Operation is not implemented (for example when
						     using some filesystems).				*/
  P_ERR_IPC_DEADLOCK = 609,  /**< Deadlock detected.					*/
  P_ERR_IPC_FAILED = 610  /**< General error.					*/
} p_err_ipc_t;

#endif /* P_ERR_H__ */
