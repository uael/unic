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

#include "p/err.h"
#include "p/mem.h"
#include "p/string.h"
#include "perror-private.h"

#ifndef P_OS_WIN
# if defined (P_OS_OS2)
#   define INCL_DOSERRORS
#   include <os2.h>
#   include <sys/socket.h>
# endif
# include <errno.h>
#endif

struct err {
  int_t code;
  int_t native_code;
  byte_t *message;
};

err_io_t
p_error_get_io_from_system(int_t err_code) {
  switch (err_code) {
    case 0:
      return P_ERR_IO_NONE;
#if defined (P_OS_WIN)
# ifdef WSAEADDRINUSE
    case WSAEADDRINUSE:
      return P_ERR_IO_ADDRESS_IN_USE;
# endif
# ifdef WSAEWOULDBLOCK
    case WSAEWOULDBLOCK:
      return P_ERR_IO_WOULD_BLOCK;
# endif
# ifdef WSAEACCES
    case WSAEACCES:
      return P_ERR_IO_ACCESS_DENIED;
# endif
# ifdef WSA_INVALID_HANDLE
    case WSA_INVALID_HANDLE:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSA_INVALID_PARAMETER
    case WSA_INVALID_PARAMETER:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAEBADF
    case WSAEBADF:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAENOTSOCK
    case WSAENOTSOCK:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAEINVAL
    case WSAEINVAL:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef WSAESOCKTNOSUPPORT
    case WSAESOCKTNOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEOPNOTSUPP
    case WSAEOPNOTSUPP:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEPFNOSUPPORT
    case WSAEPFNOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEAFNOSUPPORT
    case WSAEAFNOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAEPROTONOSUPPORT
    case WSAEPROTONOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef WSAECANCELLED
    case WSAECANCELLED:
      return P_ERR_IO_ABORTED;
# endif
# ifdef ERROR_ALREADY_EXISTS
    case ERROR_ALREADY_EXISTS:
      return P_ERR_IO_EXISTS;
# endif
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return P_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_NO_MORE_FILES
    case ERROR_NO_MORE_FILES:
      return P_ERR_IO_NO_MORE;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return P_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ERROR_OUTOFMEMORY
    case ERROR_OUTOFMEMORY:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_HANDLE
#   if !defined(WSA_INVALID_HANDLE) || (ERROR_INVALID_HANDLE != WSA_INVALID_HANDLE)
    case ERROR_INVALID_HANDLE:
      return P_ERR_IO_INVALID_ARGUMENT;
#   endif
# endif
# ifdef ERROR_INVALID_PARAMETER
# if !defined(WSA_INVALID_PARAMETER) || (ERROR_INVALID_PARAMETER != WSA_INVALID_PARAMETER)
    case ERROR_INVALID_PARAMETER:
      return P_ERR_IO_INVALID_ARGUMENT;
#   endif
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
#elif defined (P_OS_OS2)
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return P_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_PATH_NOT_FOUND
    case ERROR_PATH_NOT_FOUND:
      return P_ERR_IO_NOT_EXISTS;
# endif
# ifdef ERROR_TOO_MANY_OPEN_FILES
    case ERROR_TOO_MANY_OPEN_FILES:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return P_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_ADDRESS
    case ERROR_INVALID_ADDRESS:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ERROR_NO_MORE_FILES
    case ERROR_NO_MORE_FILES:
      return P_ERR_IO_NO_MORE;
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef ERROR_FILE_EXISTS
    case ERROR_FILE_EXISTS:
      return P_ERR_IO_EXISTS;
# endif
#else /* !P_OS_WIN && !P_OS_OS2 */
# ifdef EACCES
    case EACCES:
      return P_ERR_IO_ACCESS_DENIED;
# endif
# ifdef EPERM
    case EPERM:
      return P_ERR_IO_ACCESS_DENIED;
# endif
# ifdef ENOMEM
    case ENOMEM:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOSR
    case ENOSR:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOBUFS
    case ENOBUFS:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENFILE
    case ENFILE:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef ENOSPC
    case ENOSPC:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef EMFILE
    case EMFILE:
      return P_ERR_IO_NO_RESOURCES;
# endif
# ifdef EINVAL
    case EINVAL:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EBADF
    case EBADF:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef ENOTSOCK
    case ENOTSOCK:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EFAULT
    case EFAULT:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
# ifdef EPROTOTYPE
    case EPROTOTYPE:
      return P_ERR_IO_INVALID_ARGUMENT;
# endif
      /* On Linux these errors can have same codes */
# if defined(ENOTSUP) && (!defined(EOPNOTSUPP) || ENOTSUP != EOPNOTSUPP)
    case ENOTSUP:
        return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef ENOPROTOOPT
    case ENOPROTOOPT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EOPNOTSUPP
    case EOPNOTSUPP:
      return P_ERR_IO_NOT_SUPPORTED;
# endif
# ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENETUNREACH
    case ENETUNREACH:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENETDOWN
    case ENETDOWN:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EHOSTDOWN
    case EHOSTDOWN:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef ENONET
    case ENONET:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EHOSTUNREACH
    case EHOSTUNREACH:
      return P_ERR_IO_NOT_AVAILABLE;
# endif
# ifdef EINPROGRESS
    case EINPROGRESS:
      return P_ERR_IO_IN_PROGRESS;
# endif
# ifdef EALREADY
    case EALREADY:
      return P_ERR_IO_IN_PROGRESS;
# endif
# ifdef EISCONN
    case EISCONN:
      return P_ERR_IO_CONNECTED;
# endif
# ifdef ECONNREFUSED
    case ECONNREFUSED:
      return P_ERR_IO_CONNECTION_REFUSED;
# endif
# ifdef ENOTCONN
    case ENOTCONN:
      return P_ERR_IO_NOT_CONNECTED;
# endif
# ifdef ECONNABORTED
    case ECONNABORTED:
      return P_ERR_IO_ABORTED;
# endif
# ifdef EADDRINUSE
    case EADDRINUSE:
      return P_ERR_IO_ADDRESS_IN_USE;
# endif
# ifdef ETIMEDOUT
    case ETIMEDOUT:
      return P_ERR_IO_TIMED_OUT;
# endif
# ifdef EDQUOT
    case EDQUOT:
      return P_ERR_IO_QUOTA;
# endif
# ifdef EISDIR
    case EISDIR:
      return P_ERR_IO_IS_DIRECTORY;
# endif
# ifdef ENOTDIR
    case ENOTDIR:
      return P_ERR_IO_NOT_DIRECTORY;
# endif
# ifdef EEXIST
    case EEXIST:
      return P_ERR_IO_EXISTS;
# endif
# ifdef ENOENT
    case ENOENT:
      return P_ERR_IO_NOT_EXISTS;
# endif
# ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return P_ERR_IO_NAMETOOLONG;
# endif
# ifdef ENOSYS
    case ENOSYS:
      return P_ERR_IO_NOT_IMPLEMENTED;
# endif
      /* Some magic to deal with EWOULDBLOCK and EAGAIN.
       * Apparently on HP-UX these are actually defined to different values,
       * but on Linux, for example, they are the same. */
# if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
      /* We have both and they are the same: only emit one case. */
    case EAGAIN:
      return P_ERR_IO_WOULD_BLOCK;
# else
    /* Else: consider each of them separately. This handles both the
     * case of having only one and the case where they are different values. */
#   ifdef EAGAIN
    case EAGAIN:
      return P_ERR_IO_WOULD_BLOCK;
#   endif
#   ifdef EWOULDBLOCK
    case EWOULDBLOCK:
      return P_ERR_IO_WOULD_BLOCK;
#   endif
# endif
#endif /* !P_OS_WIN */
    default:
      return P_ERR_IO_FAILED;
  }
}

err_io_t
p_error_get_last_io(void) {
  return p_error_get_io_from_system(p_error_get_last_system());
}

err_ipc_t
p_error_get_ipc_from_system(int_t err_code) {
  switch (err_code) {
    case 0:
      return P_ERR_IPC_NONE;
#ifdef P_OS_WIN
# ifdef ERROR_ALREADY_EXISTS
    case ERROR_ALREADY_EXISTS:
      return P_ERR_IPC_EXISTS;
# endif
# ifdef ERROR_SEM_OWNER_DIED
    case ERROR_SEM_OWNER_DIED:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_SEM_NOT_FOUND
    case ERROR_SEM_NOT_FOUND:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_SEM_USER_LIMIT
    case ERROR_SEM_USER_LIMIT:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_SEMAPHORES
    case ERROR_TOO_MANY_SEMAPHORES:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_ACCESS_DENIED
    case ERROR_ACCESS_DENIED:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_EXCL_SEM_ALREADY_OWNED
    case ERROR_EXCL_SEM_ALREADY_OWNED:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_TOO_MANY_SEM_REQUESTS
    case ERROR_TOO_MANY_SEM_REQUESTS:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_POSTS
    case ERROR_TOO_MANY_POSTS:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_OUTOFMEMORY
    case ERROR_OUTOFMEMORY:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_NOT_SUPPORTED
    case ERROR_NOT_SUPPORTED:
      return P_ERR_IPC_NOT_IMPLEMENTED;
# endif
#elif defined (P_OS_OS2)
# ifdef ERROR_NOT_ENOUGH_MEMORY
    case ERROR_NOT_ENOUGH_MEMORY:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_INVALID_PARAMETER
    case ERROR_INVALID_PARAMETER:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_NAME
    case ERROR_INVALID_NAME:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_INVALID_HANDLE
    case ERROR_INVALID_HANDLE:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_FILE_NOT_FOUND
    case ERROR_FILE_NOT_FOUND:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_INVALID_ADDRESS
    case ERROR_INVALID_ADDRESS:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERROR_GEN_FAILURE
    case ERROR_GEN_FAILURE:
      return P_ERR_IPC_FAILED;
# endif
# ifdef ERROR_LOCKED
    case ERROR_LOCKED:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_DUPLICATE_NAME
    case ERROR_DUPLICATE_NAME:
      return P_ERR_IPC_EXISTS;
# endif
# ifdef ERROR_TOO_MANY_HANDLES
    case ERROR_TOO_MANY_HANDLES:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_OPENS
    case ERROR_TOO_MANY_OPENS:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_TOO_MANY_SEM_REQUESTS
    case ERROR_TOO_MANY_SEM_REQUESTS:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ERROR_SEM_OWNER_DIED
    case ERROR_SEM_OWNER_DIED:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ERROR_NOT_OWNER
    case ERROR_NOT_OWNER:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef ERROR_SEM_NOT_FOUND
    case ERROR_SEM_NOT_FOUND:
      return P_ERR_IPC_NOT_EXISTS;
# endif
#else /* !P_OS_WINDOWS && !P_OS_OS2 */
# ifdef EACCES
    case EACCES:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef EPERM
    case EPERM:
      return P_ERR_IPC_ACCESS;
# endif
# ifdef EEXIST
    case EEXIST:
      return P_ERR_IPC_EXISTS;
# endif
# ifdef E2BIG
    case E2BIG:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EFAULT
    case EFAULT:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EFBIG
    case EFBIG:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef EINVAL
    case EINVAL:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ELOOP
    case ELOOP:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ERANGE
    case ERANGE:
      return P_ERR_IPC_INVALID_ARGUMENT;
# endif
# ifdef ENOMEM
    case ENOMEM:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef EMFILE
    case EMFILE:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ENFILE
    case ENFILE:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef ENOSPC
    case ENOSPC:
      return P_ERR_IPC_NO_RESOURCES;
# endif
# ifdef EIDRM
    case EIDRM:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef ENOENT
    case ENOENT:
      return P_ERR_IPC_NOT_EXISTS;
# endif
# ifdef EOVERFLOW
    case EOVERFLOW:
      return P_ERR_IPC_OVERFLOW;
# endif
# ifdef ENOSYS
    case ENOSYS:
      return P_ERR_IPC_NOT_IMPLEMENTED;
# endif
# ifdef EDEADLK
    case EDEADLK:
      return P_ERR_IPC_DEADLOCK;
# endif
# ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return P_ERR_IPC_NAMETOOLONG;
# endif
#endif /* !P_OS_WIN */
    default:
      return P_ERR_IPC_FAILED;
  }
}

err_ipc_t
p_error_get_last_ipc(void) {
  return p_error_get_ipc_from_system(p_error_get_last_system());
}

err_t *
p_error_new(void) {
  err_t *ret;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(err_t))) == NULL)) {
    return NULL;
  }
  return ret;
}

err_t *
p_error_new_literal(int_t code, int_t native_code, const byte_t *message) {
  err_t *ret;

  if (P_UNLIKELY ((ret = p_error_new()) == NULL)) {
    return NULL;
  }
  ret->code = code;
  ret->native_code = native_code;
  ret->message = p_strdup(message);
  return ret;
}

const byte_t *
p_error_get_message(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return NULL;
  }
  return err->message;
}

int_t
p_error_get_code(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return 0;
  }
  return err->code;
}

int_t
p_error_get_native_code(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return 0;
  }
  return err->native_code;
}

err_domain_t
p_error_get_domain(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return P_ERR_DOMAIN_NONE;
  }
  if (err->code >= (int_t) P_ERR_DOMAIN_IPC) {
    return P_ERR_DOMAIN_IPC;
  } else if (err->code >= (int_t) P_ERR_DOMAIN_IO) {
    return P_ERR_DOMAIN_IO;
  } else {
    return P_ERR_DOMAIN_NONE;
  }
}

err_t *
p_error_copy(err_t *err) {
  err_t *ret;

  if (P_UNLIKELY (err == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((
    ret = p_error_new_literal(
      err->code,
      err->native_code,
      err->message
    )) == NULL)) {
    return NULL;
  }
  return ret;
}

void
p_error_set_error(err_t *err, int_t code, int_t ncode, const byte_t *msg) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    p_free(err->message);
  }
  err->code = code;
  err->native_code = ncode;
  err->message = p_strdup(msg);
}

void
p_error_set_error_p(err_t **err, int_t code, int_t ncode, const byte_t *msg) {
  if (err == NULL || *err != NULL) {
    return;
  }
  *err = p_error_new_literal(code, ncode, msg);
}

void
p_error_set_code(err_t *err, int_t code) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  err->code = code;
}

void
p_error_set_native_code(err_t *err, int_t ncode) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  err->native_code = ncode;
}

void
p_error_set_message(err_t *err, const byte_t *msg) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    p_free(err->message);
  }
  err->message = p_strdup(msg);
}

void
p_error_clear(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    p_free(err->message);
  }
  err->message = NULL;
  err->code = 0;
  err->native_code = 0;
}

void
p_error_free(err_t *err) {
  if (P_UNLIKELY (err == NULL)) {
    return;
  }
  if (err->message != NULL) {
    p_free(err->message);
  }
  p_free(err);
}

int_t
p_error_get_last_system(void) {
#ifdef P_OS_WIN
  return (int_t) GetLastError();
#else
# ifdef P_OS_VMS
  int_t error_code = errno;

  if (error_code == EVMSERR)
    return vaxc$errno;
  else
    return error_code;
# else
  return errno;
# endif
#endif
}

int_t
p_error_get_last_net(void) {
#if defined (P_OS_WIN)
  return WSAGetLastError();
#elif defined (P_OS_OS2)
  return sock_errno();
#else
  return p_error_get_last_system();
#endif
}

void
p_error_set_last_system(int_t code) {
#ifdef P_OS_WIN
  SetLastError((DWORD) code);
#else
  errno = code;
#endif
}

void
p_error_set_last_net(int_t code) {
#if defined (P_OS_WIN)
  WSASetLastError(code);
#elif defined (P_OS_OS2)
  P_UNUSED(code);
#else
  errno = code;
#endif
}
