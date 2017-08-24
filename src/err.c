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

#include "unic/err.h"
#include "unic/mem.h"
#include "unic/string.h"
#include "err-private.h"

#ifndef U_OS_WIN
# if defined (U_OS_OS2)
#   define INCL_DOSERRORS
#   include <os2.h>
#   include <sys/socket.h>
# endif
# include <errno.h>
#endif

struct err {
  int code;
  int native_code;
  byte_t *message;
};

err_io_t
u_err_get_io_from_system(int err_code) {
  switch (err_code) {
    case 0:
      return U_ERR_IO_NONE;
#if defined (U_OS_WIN)
# ifdef WSAEADDRINUSE
    case WSAEADDRINUSE:
      return U_ERR_IO_ADDRESS_IN_USE;
# endif
# ifdef WSAEWOULDBLOCK
    case WSAEWOULDBLOCK:
      return U_ERR_IO_WOULD_BLOCK;
# endif
# ifdef WSAEACCES
    case WSAEACCES:
      return U_ERR_IO_ACCESS_DENIED;
# endif
# ifdef WSA_INVALID_HANDLE
    case WSA_INVALID_HANDLE:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSA_INVALID_PARAMETER
    case WSA_INVALID_PARAMETER:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAEBADF
    case WSAEBADF:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAENOTSOCK
    case WSAENOTSOCK:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAEINVAL
    case WSAEINVAL:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAESOCKTNOSUPPORT
    case WSAESOCKTNOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEOPNOTSUPP
    case WSAEOPNOTSUPP:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEPFNOSUPPORT
    case WSAEPFNOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEAFNOSUPPORT
    case WSAEAFNOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEPROTONOSUPPORT
    case WSAEPROTONOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAECANCELLED
    case WSAECANCELLED:
      return U_ERR_IO_ABORTED;
# endif
# ifdef ERROR_ALREADY_EXISTS
    case ERROR_ALREADY_EXISTS:
      return U_ERR_IO_EXISTS;
# endif
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return U_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_NO_MORE_FILES
    case ERROR_NO_MORE_FILES:
      return U_ERR_IO_NO_MORE;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return U_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ERROR_OUTOFMEMORY
    case ERROR_OUTOFMEMORY:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_HANDLE
#   if !defined(WSA_INVALID_HANDLE) || (ERROR_INVALID_HANDLE != WSA_INVALID_HANDLE)
    case ERROR_INVALID_HANDLE:
      return U_ERR_IO_INVALID_ARGUMENT;
#   endif
# endif
# ifdef ERROR_INVALID_PARAMETER
# if !defined(WSA_INVALID_PARAMETER) || (ERROR_INVALID_PARAMETER != WSA_INVALID_PARAMETER)
    case ERROR_INVALID_PARAMETER:
      return U_ERR_IO_INVALID_ARGUMENT;
#   endif
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
#elif defined (U_OS_OS2)
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return U_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_PATH_NOT_FOUND
    case ERROR_PATH_NOT_FOUND:
      return U_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_TOO_MANY_OPEN_FILES
    case ERROR_TOO_MANY_OPEN_FILES:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return U_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_ADDRESS
    case ERROR_INVALID_ADDRESS:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_NO_MORE_FILES
    case ERROR_NO_MORE_FILES:
      return U_ERR_IO_NO_MORE;
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef ERROR_FILE_EXISTS
    case ERROR_FILE_EXISTS:
      return U_ERR_IO_EXISTS;
# endif
#else /* !U_OS_WIN && !U_OS_OS2 */
# ifdef EACCES
    case EACCES:
      return U_ERR_IO_ACCESS_DENIED;
# endif
# ifdef EPERM
    case EPERM:
      return U_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ENOMEM
    case ENOMEM:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOSR
    case ENOSR:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOBUFS
    case ENOBUFS:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENFILE
    case ENFILE:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOSPC
    case ENOSPC:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef EMFILE
    case EMFILE:
      return U_ERR_IO_NO_RESOURCES;
# endif
# ifdef EINVAL
    case EINVAL:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EBADF
    case EBADF:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ENOTSOCK
    case ENOTSOCK:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EFAULT
    case EFAULT:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EPROTOTYPE
    case EPROTOTYPE:
      return U_ERR_IO_INVALID_ARGUMENT;
# endif
      /* On Linux these errors can have same codes */
# if defined(ENOTSUP) && (!defined(EOPNOTSUPP) || ENOTSUP != EOPNOTSUPP)
    case ENOTSUP:
        return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef ENOPROTOOPT
    case ENOPROTOOPT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EOPNOTSUPP
    case EOPNOTSUPP:
      return U_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENETUNREACH
    case ENETUNREACH:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENETDOWN
    case ENETDOWN:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EHOSTDOWN
    case EHOSTDOWN:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENONET
    case ENONET:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EHOSTUNREACH
    case EHOSTUNREACH:
      return U_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EINPROGRESS
    case EINPROGRESS:
      return U_ERR_IO_IN_PROGRESS;
# endif
# ifdef EALREADY
    case EALREADY:
      return U_ERR_IO_IN_PROGRESS;
# endif
# ifdef EISCONN
    case EISCONN:
      return U_ERR_IO_CONNECTED;
# endif
# ifdef ECONNREFUSED
    case ECONNREFUSED:
      return U_ERR_IO_CONNECTION_REFUSED;
# endif
# ifdef ENOTCONN
    case ENOTCONN:
      return U_ERR_IO_NOT_CONNECTED;
# endif
# ifdef ECONNABORTED
    case ECONNABORTED:
      return U_ERR_IO_ABORTED;
# endif
# ifdef EADDRINUSE
    case EADDRINUSE:
      return U_ERR_IO_ADDRESS_IN_USE;
# endif
# ifdef ETIMEDOUT
    case ETIMEDOUT:
      return U_ERR_IO_TIMED_OUT;
# endif
# ifdef EDQUOT
    case EDQUOT:
      return U_ERR_IO_QUOTA;
# endif
# ifdef EISDIR
    case EISDIR:
      return U_ERR_IO_IS_DIRECTORY;
# endif
# ifdef ENOTDIR
    case ENOTDIR:
      return U_ERR_IO_NOT_DIRECTORY;
# endif
# ifdef EEXIST
    case EEXIST:
      return U_ERR_IO_EXISTS;
# endif
# ifdef ENOENT
    case ENOENT:
      return U_ERR_IO_NOT_EXISTS;
# endif
# ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return U_ERR_IO_NAMETOOLONG;
# endif
# ifdef ENOSYS
    case ENOSYS:
      return U_ERR_IO_NOT_IMPLEMENTED;
# endif
      /* Some magic to deal with EWOULDBLOCK and EAGAIN.
       * Apparently on HP-UX these are actually defined to different values,
       * but on Linux, for example, they are the same. */
# if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
      /* We have both and they are the same: only emit one case. */
    case EAGAIN:
      return U_ERR_IO_WOULD_BLOCK;
# else
    /* Else: consider each of them separately. This handles both the
     * case of having only one and the case where they are different values. */
#   ifdef EAGAIN
    case EAGAIN:
      return U_ERR_IO_WOULD_BLOCK;
#   endif
#   ifdef EWOULDBLOCK
    case EWOULDBLOCK:
      return U_ERR_IO_WOULD_BLOCK;
#   endif
# endif
#endif /* !U_OS_WIN */
    default:
      return U_ERR_IO_FAILED;
  }
}

err_io_t
u_err_get_last_io(void) {
  return u_err_get_io_from_system(u_err_get_last_system());
}

err_ipc_t
u_err_get_ipc_from_system(int err_code) {
  switch (err_code) {
    case 0:
      return U_ERR_IPC_NONE;
#ifdef U_OS_WIN
# ifdef ERROR_ALREADY_EXISTS
    case ERROR_ALREADY_EXISTS:
      return U_ERR_IPC_EXISTS;
# endif
# ifdef ERROR_SEM_OWNER_DIED
    case ERROR_SEM_OWNER_DIED:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_SEM_NOT_FOUND
    case ERROR_SEM_NOT_FOUND:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_SEM_USER_LIMIT
    case ERROR_SEM_USER_LIMIT:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_SEMAPHORES
    case ERROR_TOO_MANY_SEMAPHORES:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_EXCL_SEM_ALREADY_OWNED
    case ERROR_EXCL_SEM_ALREADY_OWNED:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_TOO_MANY_SEM_REQUESTS
    case ERROR_TOO_MANY_SEM_REQUESTS:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_POSTS
    case ERROR_TOO_MANY_POSTS:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_OUTOFMEMORY
    case ERROR_OUTOFMEMORY:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return U_ERR_IPC_NOT_IMPLEMENTED;
# endif
#elif defined (U_OS_OS2)
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_NAME
    case ERROR_INVALID_NAME:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_INVALID_ADDRESS
    case ERROR_INVALID_ADDRESS:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_GEN_FAILURE
    case ERROR_GEN_FAILURE:
      return U_ERR_IPC_FAILED;
# endif
# ifdef ERROR_LOCKED
    case ERROR_LOCKED:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_DUPLICATE_NAME
    case ERROR_DUPLICATE_NAME:
      return U_ERR_IPC_EXISTS;
# endif
# ifdef ERROR_TOO_MANY_HANDLES
    case ERROR_TOO_MANY_HANDLES:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_OPENS
    case ERROR_TOO_MANY_OPENS:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_SEM_REQUESTS
    case ERROR_TOO_MANY_SEM_REQUESTS:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_SEM_OWNER_DIED
    case ERROR_SEM_OWNER_DIED:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_NOT_OWNER
    case ERROR_NOT_OWNER:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_SEM_NOT_FOUND
    case ERROR_SEM_NOT_FOUND:
      return U_ERR_IPC_NOT_EXISTS;
# endif
#else /* !U_OS_WINDOWS && !U_OS_OS2 */
# ifdef EACCES
    case EACCES:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef EPERM
    case EPERM:
      return U_ERR_IPC_ACCESS;
# endif
# ifdef EEXIST
    case EEXIST:
      return U_ERR_IPC_EXISTS;
# endif
# ifdef E2BIG
    case E2BIG:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EFAULT
    case EFAULT:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EFBIG
    case EFBIG:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EINVAL
    case EINVAL:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ELOOP
    case ELOOP:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERANGE
    case ERANGE:
      return U_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ENOMEM
    case ENOMEM:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef EMFILE
    case EMFILE:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ENFILE
    case ENFILE:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ENOSPC
    case ENOSPC:
      return U_ERR_IPC_NO_RESOURCES;
# endif
# ifdef EIDRM
    case EIDRM:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ENOENT
    case ENOENT:
      return U_ERR_IPC_NOT_EXISTS;
# endif
# ifdef EOVERFLOW
    case EOVERFLOW:
      return U_ERR_IPC_OVERFLOW;
# endif
# ifdef ENOSYS
    case ENOSYS:
      return U_ERR_IPC_NOT_IMPLEMENTED;
# endif
# ifdef EDEADLK
    case EDEADLK:
      return U_ERR_IPC_DEADLOCK;
# endif
# ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return U_ERR_IPC_NAMETOOLONG;
# endif
#endif /* !U_OS_WIN */
    default:
      return U_ERR_IPC_FAILED;
  }
}

err_ipc_t
u_err_get_last_ipc(void) {
  return u_err_get_ipc_from_system(u_err_get_last_system());
}

err_t *
u_err_new(void) {
  err_t *ret;

  if (U_UNLIKELY ((ret = u_malloc0(sizeof(err_t))) == NULL)) {
    return NULL;
  }
  return ret;
}

err_t *
u_err_new_literal(int code, int native_code, const byte_t *message) {
  err_t *ret;

  if (U_UNLIKELY ((ret = u_err_new()) == NULL)) {
    return NULL;
  }
  ret->code = code;
  ret->native_code = native_code;
  ret->message = u_strdup(message);
  return ret;
}

const byte_t *
u_err_get_message(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return NULL;
  }
  return err->message;
}

int
u_err_get_code(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return 0;
  }
  return err->code;
}

int
u_err_get_native_code(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return 0;
  }
  return err->native_code;
}

err_domain_t
u_err_get_domain(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return U_ERR_DOMAIN_NONE;
  }
  if (err->code >= (int) U_ERR_DOMAIN_IPC) {
    return U_ERR_DOMAIN_IPC;
  } else if (err->code >= (int) U_ERR_DOMAIN_IO) {
    return U_ERR_DOMAIN_IO;
  } else {
    return U_ERR_DOMAIN_NONE;
  }
}

err_t *
u_err_copy(err_t *err) {
  err_t *ret;

  if (U_UNLIKELY (err == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((
    ret = u_err_new_literal(
      err->code,
      err->native_code,
      err->message
    )) == NULL)) {
    return NULL;
  }
  return ret;
}

void
u_err_set_error(err_t *err, int code, int ncode, const byte_t *msg) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    u_free(err->message);
  }
  err->code = code;
  err->native_code = ncode;
  err->message = u_strdup(msg);
}

void
u_err_set_err_p(err_t **err, int code, int ncode, const byte_t *msg) {
  if (err == NULL || *err != NULL) {
    return;
  }
  *err = u_err_new_literal(code, ncode, msg);
}

void
u_err_set_code(err_t *err, int code) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  err->code = code;
}

void
u_err_set_native_code(err_t *err, int ncode) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  err->native_code = ncode;
}

void
u_err_set_message(err_t *err, const byte_t *msg) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    u_free(err->message);
  }
  err->message = u_strdup(msg);
}

void
u_err_clear(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    u_free(err->message);
  }
  err->message = NULL;
  err->code = 0;
  err->native_code = 0;
}

void
u_err_free(err_t *err) {
  if (U_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    u_free(err->message);
  }
  u_free(err);
}

int
u_err_get_last_system(void) {
#ifdef U_OS_WIN
  return (int) GetLastError();
#else
# ifdef U_OS_VMS
  int error_code = errno;

  if (error_code == EVMSERR)
    return vaxc$errno;
  else
    return error_code;
# else
  return errno;
# endif
#endif
}

int
u_err_get_last_net(void) {
#if defined (U_OS_WIN)
  return WSAGetLastError();
#elif defined (U_OS_OS2)
  return sock_errno();
#else
  return u_err_get_last_system();
#endif
}

void
u_err_set_last_system(int code) {
#ifdef U_OS_WIN
  SetLastError((DWORD) code);
#else
  errno = code;
#endif
}

void
u_err_set_last_net(int code) {
#if defined (U_OS_WIN)
  WSASetLastError(code);
#elif defined (U_OS_OS2)
  U_UNUSED(code);
#else
  errno = code;
#endif
}
