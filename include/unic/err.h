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

/*!@file unic/error.h
 * @brief Error report system
 * @author Alexander Saprykin
 *
 * An error report system is used to notify a caller about fatal situations
 * during the library API invocation. Usually the sequence is as following:
 * @code
 * err_t *error = NULL;
 * ...
 *
 * if (error != NULL) {
 *   ...
 *   u_err_free (error);
 * }
 * @endcode
 * Note that you should not initialize a new #err_t object before passing the
 * pointer into an API call. Simply initialize it with zero and check the result
 * after. Therefore you need to free memory if an error occurred.
 *
 * Most operating systems store the last error code of the most system calls in
 * a thread-specific variable. Moreover, Windows stores the error code of the
 * last socket related call in a separate variable. Use
 * u_err_get_last_system() and u_err_set_last_system() to access the last
 * system error code, u_err_get_last_net() and u_err_set_last_net() to
 * access the last network error code.
 *
 * All error codes are splitted into the several domains. Every error should
 * belong to one of the domains described in #err_domain_t. Think of an error
 * domain as a logical subsystem.
 *
 * Every error domain has its own enumeration with the list of possible error
 * codes. System error codes are converted to specified domains using internal
 * routines.
 */
#ifndef U_ERROR_H__
# define U_ERROR_H__

#include "unic/macros.h"
#include "unic/types.h"

typedef struct err err_t;

/*!@brief Enum with error domains. */
enum err_domain {

  /*!@brief No domain was specified. */
  U_ERR_DOMAIN_NONE = 0,

  /*!@brief Input/output domain. */
  U_ERR_DOMAIN_IO = 500,

  /*!@brief Interprocess communication domain. */
  U_ERR_DOMAIN_IPC = 600
};

/*!@brief Enum with IO errors. */
enum err_io {

  /*!@brief No error. */
  U_ERR_IO_NONE = 500,

  /*!@brief Operating system hasn't enough resources. */
  U_ERR_IO_NO_RESOURCES = 501,

  /*!@brief Resource isn't available. */
  U_ERR_IO_NOT_AVAILABLE = 502,

  /*!@brief Access denied. */
  U_ERR_IO_ACCESS_DENIED = 503,

  /*!@brief Already connected. */
  U_ERR_IO_CONNECTED = 504,

  /*!@brief Operation in progress. */
  U_ERR_IO_IN_PROGRESS = 505,

  /*!@brief Operation aborted. */
  U_ERR_IO_ABORTED = 506,

  /*!@brief Invalid argument specified. */
  U_ERR_IO_INVALID_ARGUMENT = 507,

  /*!@brief Operation not supported. */
  U_ERR_IO_NOT_SUPPORTED = 508,

  /*!@brief Operation timed out. */
  U_ERR_IO_TIMED_OUT = 509,

  /*!@brief Operation cannot be completed immediatly. */
  U_ERR_IO_WOULD_BLOCK = 510,

  /*!@brief Address is already under usage. */
  U_ERR_IO_ADDRESS_IN_USE = 511,

  /*!@brief Connection refused. */
  U_ERR_IO_CONNECTION_REFUSED = 512,

  /*!@brief Connection required first. */
  U_ERR_IO_NOT_CONNECTED = 513,

  /*!@brief User quota exceeded. */
  U_ERR_IO_QUOTA = 514,

  /*!@brief Trying to open directory for writing. */
  U_ERR_IO_IS_DIRECTORY = 515,

  /*!@brief Component of the path prefix is not a directory. */
  U_ERR_IO_NOT_DIRECTORY = 516,

  /*!@brief Specified name is too long. */
  U_ERR_IO_NAMETOOLONG = 517,

  /*!@brief Specified entry already exists. */
  U_ERR_IO_EXISTS = 518,

  /*!@brief Specified entry doesn't exist. */
  U_ERR_IO_NOT_EXISTS = 519,

  /*!@brief No more data left. */
  U_ERR_IO_NO_MORE = 520,

  /*!@brief Operation is not implemented. */
  U_ERR_IO_NOT_IMPLEMENTED = 521,

  /*!@brief General error. */
  U_ERR_IO_FAILED = 522
};

/*!@brief Enum with IPC errors */
enum err_ipc {

  /*!@brief No error. */
  U_ERR_IPC_NONE = 600,

  /*!@brief Not enough rights to access object or its key. */
  U_ERR_IPC_ACCESS = 601,

  /*!@brief Object already exists and no proper open flags were specified. */
  U_ERR_IPC_EXISTS = 602,

  /**
   * Object doesn't exist or was removed before, and no proper create flags
   * were specified.
   */
  U_ERR_IPC_NOT_EXISTS = 603,

  /*!@brief Not enough system resources or memory to perform operation. */
  U_ERR_IPC_NO_RESOURCES = 604,

  /*!@brief Semaphore value overflow. */
  U_ERR_IPC_OVERFLOW = 605,

  /*!@brief Object name is too long. */
  U_ERR_IPC_NAMETOOLONG = 606,

  /*!@brief Invalid argument (parameter) specified. */
  U_ERR_IPC_INVALID_ARGUMENT = 607,

  /*!@brief Operation is not implemented (for example when using some filesystems). */
  U_ERR_IPC_NOT_IMPLEMENTED = 608,

  /*!@brief Deadlock detected. */
  U_ERR_IPC_DEADLOCK = 609,

  /*!@brief General error. */
  U_ERR_IPC_FAILED = 610
};

typedef enum err_domain err_domain_t;
typedef enum err_io err_io_t;
typedef enum err_ipc err_ipc_t;

/*!@brief Initializes a new empty #err_t.
 * @return Newly initialized #err_t object in case of success, NULL otherwise.
 * @since 0.0.1
 */
U_API err_t *
u_err_new(void);

/*!@brief Initializes a new #err_t with data.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @return Newly initialized #err_t object in case of success, NULL otherwise.
 * @since 0.0.1
 */
U_API err_t *
u_err_new_literal(int code, int native_code, const byte_t *message);

/*!@brief Gets an error message.
 * @param err #err_t object to get the message from.
 * @return Error message in case of success, NULL otherwise.
 * @since 0.0.1
 */
U_API const byte_t *
u_err_get_message(err_t *err);

/*!@brief Gets an error code.
 * @param err #err_t object to get the code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 */
U_API int
u_err_get_code(err_t *err);

/*!@brief Gets a platform native error code, if any.
 * @param err #err_t object to get the native code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 * @note In some situations there can be no native code error, i.e. when an
 * internal library call failed. Do not rely on this code.
 */
U_API int
u_err_get_native_code(err_t *err);

/*!@brief Gets an error domain.
 * @param err #err_t object to get the domain from.
 * @return Error domain in case of success, #U_ERR_DOMAIN_NONE otherwise.
 * @since 0.0.1
 */
U_API err_domain_t
u_err_get_domain(err_t *err);

/*!@brief Creates a copy of a #err_t object.
 * @param err #err_t object to copy.
 * @return Newly created #err_t object in case of success, NULL otherwise.
 * @since 0.0.1
 * @note The caller is responsible to free memory of the created object after
 * usage.
 */
U_API err_t *
u_err_copy(err_t *err);

/*!@brief Sets error data.
 * @param err #err_t object to set the data for.
 * @param code Error code.
 * @param ncode Native error code, leave 0 to ignore.
 * @param msg Error message.
 * @since 0.0.1
 */
U_API void
u_err_set_error(err_t *err, int code, int ncode, const byte_t *msg);

/*!@brief Sets error data through a double pointer.
 * @param err #err_t object to set the data for.
 * @param code Error code.
 * @param ncode Native error code, leave 0 to ignore.
 * @param msg Error message.
 * @since 0.0.1
 *
 * If @a error is NULL it does nothing. If @a error is not NULL then @a *error
 * should be NULL, otherwise it does nothing. It creates a #err_t object, sets
 * error data and assigns it to @a *error. The caller is responsible to free
 * memory of the created object after usage.
 */
U_API void
u_err_set_err_p(err_t **err, int code, int ncode, const byte_t *msg);

/*!@brief Sets an error code.
 * @param err #err_t object to set the code for.
 * @param code Error code.
 * @since 0.0.1
 */
U_API void
u_err_set_code(err_t *err, int code);

/*!@brief Sets a platform native error code.
 * @param err #err_t object to set the native error code for.
 * @param ncode Platform native error code.
 * @since 0.0.1
 */
U_API void
u_err_set_native_code(err_t *err, int ncode);

/*!@brief Sets an error message.
 * @param err #err_t object to set the message for.
 * @param msg Error message.
 * @since 0.0.1
 */
U_API void
u_err_set_message(err_t *err, const byte_t *msg);

/*!@brief Clears error data.
 * @param err #err_t object to clear the data for.
 * @since 0.0.1
 * @note Error code is reseted to 0.
 */
U_API void
u_err_clear(err_t *err);

/*!@brief Frees a previously initialized error object.
 * @param err #err_t object to free.
 * @since 0.0.1
 */
U_API void
u_err_free(err_t *err);

/*!@brief Gets the last system native error code.
 * @return Last system native error code.
 * @since 0.0.2
 * @sa u_err_get_last_net(), u_err_set_last_system(),
 * u_err_set_last_net()
 * @note If you want get an error code for socket-related calls, use
 * u_err_get_last_net() instead.
 */

U_API int
u_err_get_last_system(void);

/*!@brief Gets the last network native error code.
 * @return Last network native error code.
 * @since 0.0.2
 * @sa u_err_get_last_system(), u_err_set_last_net(),
 * u_err_set_last_system()
 */
U_API int
u_err_get_last_net(void);

/*!@brief Sets the last system native error code.
 * @param code Error code to set.
 * @since 0.0.2
 * @sa u_err_set_last_net(), u_err_get_last_system(),
 * u_err_get_last_net()
 * @note If you want set an error code for socket-related calls, use
 * u_err_set_last_net() instead.
 */
U_API void
u_err_set_last_system(int code);

/*!@brief Sets the last network native error code.
 * @param code Error code to set.
 * @since 0.0.2
 * @sa u_err_set_last_system(), u_err_get_last_net(),
 * u_err_get_last_system()
 */
U_API void
u_err_set_last_net(int code);

#endif /* !U_ERROR_H__ */
