/*
 * Copyright (C) 2008 Christian Kellner, Samuel Cormier-Iijima
 * Copyright (C) 2009 Codethink Limited
 * Copyright (C) 2009 Red Hat, Inc
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
#include "p/socket.h"

#ifdef P_OS_SCO
# include "p/ptimeprofiler.h"
#endif

#include "err-private.h"
#include "sysclose-private.h"

#ifndef P_OS_WIN
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# ifdef P_OS_VMS
#   include <stropts.h>
# endif
#endif

#ifndef P_OS_WIN
# if defined (P_OS_BEOS) || defined (P_OS_MAC) || defined (P_OS_MAC9) || \
      defined (P_OS_OS2)
#   define P_SOCKET_USE_SELECT
#   include <sys/select.h>
#   include <sys/time.h>
# else
#   define P_SOCKET_USE_POLL
#   include <sys/poll.h>
# endif
#endif

/* On old Solaris systems SOMAXCONN is set to 5 */
#define P_SOCKET_DEFAULT_BACKLOG  5
struct socket {
  socket_family_t family;
  socket_protocol_t protocol;
  socket_kind_t type;
  int fd;
  int listen_backlog;
  int timeout;
  uint_t blocking  : 1;
  uint_t keepalive  : 1;
  uint_t closed    : 1;
  uint_t connected  : 1;
  uint_t listening  : 1;
#ifdef P_OS_WIN
  WSAEVENT events;
#endif
#ifdef P_OS_SCO
  p_profiler_t *timer;
#endif
};

#ifndef SHUT_RD
# define SHUT_RD      0
#endif

#ifndef SHUT_WR
# define SHUT_WR      1
#endif

#ifndef SHUT_RDWR
# define SHUT_RDWR      2
#endif

#ifdef MSG_NOSIGNAL
# define P_SOCKET_DEFAULT_SEND_FLAGS  MSG_NOSIGNAL
#else
# define P_SOCKET_DEFAULT_SEND_FLAGS  0
#endif

static bool
pp_socket_set_fd_blocking(int fd, bool blocking, err_t **error);

static bool
pp_socket_check(const socket_t *socket, err_t **error);

static bool
pp_socket_set_details_from_fd(socket_t *socket, err_t **error);

static bool
pp_socket_set_fd_blocking(int fd, bool blocking, err_t **error) {
#ifndef P_OS_WIN
  int32_t arg;
#else
  ulong_t arg;
#endif

#ifndef P_OS_WIN
# ifdef P_OS_VMS
  arg = !blocking;
#   if (PLIBSYS_SIZEOF_VOID_P == 8)
#   pragma __pointer_size 32
#   endif
  /* Explicit (void *) cast is necessary */
  if (P_UNLIKELY (ioctl (fd, FIONBIO, (void *) &arg) < 0)) {
#   if (PLIBSYS_SIZEOF_VOID_P == 8)
#   pragma __pointer_size 64
#   endif
# else
  if (P_UNLIKELY ((arg = fcntl(fd, F_GETFL, NULL)) < 0)) {
    P_WARNING ("socket_t::pp_socket_set_fd_blocking: fcntl() failed");
    arg = 0;
  }
  arg = (!blocking) ? (arg | O_NONBLOCK) : (arg & ~O_NONBLOCK);
  if (P_UNLIKELY (fcntl(fd, F_SETFL, arg) < 0)) {
# endif
#else
  arg = !blocking;
  if (P_UNLIKELY (ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)) {
#endif
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to set socket blocking flags"
    );
    return false;
  }
  return true;
}

static bool
pp_socket_check(const socket_t *socket, err_t **error) {
  if (P_UNLIKELY (socket->closed)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NOT_AVAILABLE,
      0,
      "Socket is already closed"
    );
    return false;
  }
  return true;
}

static bool
pp_socket_set_details_from_fd(socket_t *socket, err_t **error) {
#ifdef SO_DOMAIN
  socket_family_t family;
#endif
  struct sockaddr_storage address;
  int fd, value;
  socklen_t addrlen, optlen;
#ifdef P_OS_WIN
  /* See comment below */
  BOOL bool_val = false;
#else
  int bool_val;
#endif

  fd = socket->fd;
  optlen = sizeof(value);
  if (P_UNLIKELY (
    getsockopt(fd, SOL_SOCKET, SO_TYPE, (ptr_t) &value, &optlen) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call getsockopt() to get socket info for fd"
    );
    return false;
  }
  if (P_UNLIKELY (optlen != sizeof(value))) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Failed to get socket info for fd, bad option length"
    );
    return false;
  }
  switch (value) {
    case SOCK_STREAM:
      socket->type = P_SOCKET_STREAM;
      break;
    case SOCK_DGRAM:
      socket->type = P_SOCKET_DATAGRAM;
      break;
#ifdef SOCK_SEQPACKET
    case SOCK_SEQPACKET:
      socket->type = P_SOCKET_SEQPACKET;
      break;
#endif
    default:
      socket->type = P_SOCKET_UNKNOWN;
      break;
  }
  addrlen = sizeof(address);
  if (P_UNLIKELY (
    getsockname(fd, (struct sockaddr *) &address, &addrlen) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call getsockname() to get socket address info"
    );
    return false;
  }
#ifdef SO_DOMAIN
  if (addrlen <= 0) {
    optlen = sizeof(family);

    if (P_UNLIKELY (getsockopt(socket->fd,
      SOL_SOCKET,
      SO_DOMAIN,
      (ptr_t) &family,
      &optlen) != 0)) {
      p_err_set_err_p(error,
        (int) p_err_get_io_from_system(p_err_get_last_net()),
        (int) p_err_get_last_net(),
        "Failed to call getsockopt() to get socket SO_DOMAIN option");
      return false;
    }
  }
#endif
  switch (address.ss_family) {
    case P_SOCKET_FAMILY_INET:
      socket->family = P_SOCKET_FAMILY_INET;
      break;
#ifdef AF_INET6
    case P_SOCKET_FAMILY_INET6:
      socket->family = P_SOCKET_FAMILY_INET6;
      break;
#endif
    default:
      socket->family = P_SOCKET_FAMILY_UNKNOWN;
      break;
  }
#ifdef AF_INET6
  if (socket->family == P_SOCKET_FAMILY_INET6
    || socket->family == P_SOCKET_FAMILY_INET) {
#else
    if (socket->family == P_SOCKET_FAMILY_INET) {
#endif
    switch (socket->type) {
      case P_SOCKET_STREAM:
        socket->protocol = P_SOCKET_PROTOCOL_TCP;
        break;
      case P_SOCKET_DATAGRAM:
        socket->protocol = P_SOCKET_PROTOCOL_UDP;
        break;
      case P_SOCKET_SEQPACKET:
        socket->protocol = P_SOCKET_PROTOCOL_SCTP;
        break;
      case P_SOCKET_UNKNOWN:
        break;
    }
  }
  if (P_LIKELY (socket->family != P_SOCKET_FAMILY_UNKNOWN)) {
    addrlen = sizeof(address);
    if (getpeername(fd, (struct sockaddr *) &address, &addrlen) >= 0) {
      socket->connected = true;
    }
  }
  optlen = sizeof(bool_val);
  if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (ptr_t) &bool_val, &optlen)
    == 0) {
#ifndef P_OS_WIN
    /* Experimentation indicates that the SO_KEEPALIVE value is
     * actually a char on Windows, even if documentation claims it
     * to be a BOOL which is a typedef for int. */
    if (optlen != sizeof(bool_val))
      P_WARNING (
        "socket_t::pp_socket_set_details_from_fd: getsockopt() with SO_KEEPALIVE failed");
#endif
    socket->keepalive = (uint_t) (bool_val != 0);
  } else {
    /* Can't read, maybe not supported, assume false */
    socket->keepalive = false;
  }
  return true;
}

bool
p_socket_init_once(void) {
#ifdef P_OS_WIN
  WORD ver_req;
  WSADATA wsa_data;
  ver_req = MAKEWORD (2, 2);
  if (P_UNLIKELY (WSAStartup(ver_req, &wsa_data) != 0)) {
    return false;
  }
  if (P_UNLIKELY (
    LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)) {
    WSACleanup();
    return false;
  }
#else
# ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
# endif
#endif
  return true;
}

void
p_socket_close_once(void) {
#ifdef P_OS_WIN
  WSACleanup();
#endif
}

socket_t *
p_socket_new_from_fd(int fd, err_t **error) {
  socket_t *ret;
#if !defined (P_OS_WIN) && defined (SO_NOSIGPIPE)
  int flags;
#endif

  if (P_UNLIKELY (fd < 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Unable to create socket from bad fd"
    );
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(socket_t))) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for socket"
    );
    return NULL;
  }
  ret->fd = fd;
  if (P_UNLIKELY (pp_socket_set_details_from_fd(ret, error) == false)) {
    p_free(ret);
    return NULL;
  }
  if (P_UNLIKELY (pp_socket_set_fd_blocking(ret->fd, false, error) == false)) {
    p_free(ret);
    return NULL;
  }
#if !defined (P_OS_WIN) && defined (SO_NOSIGPIPE)
  flags = 1;

  if (setsockopt (ret->fd, SOL_SOCKET, SO_NOSIGPIPE, &flags, sizeof (flags)) < 0)
    P_WARNING ("socket_t::p_socket_new_from_fd: setsockopt() with SO_NOSIGPIPE failed");
#endif
  p_socket_set_listen_backlog(ret, P_SOCKET_DEFAULT_BACKLOG);
  ret->timeout = 0;
  ret->blocking = true;
#ifdef P_OS_SCO
  if (P_UNLIKELY ((ret->timer = p_profiler_new ()) == NULL)) {
    p_err_set_err_p (error,
             (int) P_ERR_IO_NO_RESOURCES,
             0,
             "Failed to allocate memory for internal timer");
    p_free (ret);
    return NULL;
  }
#endif
#ifdef P_OS_WIN
  if (P_UNLIKELY ((ret->events = WSACreateEvent()) == WSA_INVALID_EVENT)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      (int) p_err_get_last_net(),
      "Failed to call WSACreateEvent() on socket"
    );
    p_free(ret);
    return NULL;
  }
#endif
  return ret;
}

socket_t *
p_socket_new(socket_family_t family, socket_kind_t type,
  socket_protocol_t protocol, err_t **error) {
  socket_t *ret;
  int native_type, fd;
#ifndef P_OS_WIN
  int flags;
#endif

  if (P_UNLIKELY (family == P_SOCKET_FAMILY_UNKNOWN ||
    type == P_SOCKET_UNKNOWN ||
    protocol == P_SOCKET_PROTOCOL_UNKNOWN)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input socket family, type or protocol"
    );
    return NULL;
  }
  switch (type) {
    case P_SOCKET_STREAM:
      native_type = SOCK_STREAM;
      break;
    case P_SOCKET_DATAGRAM:
      native_type = SOCK_DGRAM;
      break;
#ifdef SOCK_SEQPACKET
    case P_SOCKET_SEQPACKET:
      native_type = SOCK_SEQPACKET;
      break;
#endif
    default:
      p_err_set_err_p(
        error,
        (int) P_ERR_IO_INVALID_ARGUMENT,
        0,
        "Unable to create socket with unknown family"
      );
      return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(socket_t))) == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_NO_RESOURCES,
      0,
      "Failed to allocate memory for socket"
    );
    return NULL;
  }
#ifdef P_OS_SCO
  if (P_UNLIKELY ((ret->timer = p_profiler_new ()) == NULL)) {
    p_err_set_err_p (error,
             (int) P_ERR_IO_NO_RESOURCES,
             0,
             "Failed to allocate memory for internal timer");
    p_free (ret);
    return NULL;
  }
#endif
#ifdef SOCK_CLOEXEC
  native_type |= SOCK_CLOEXEC;
#endif
  if (P_UNLIKELY ((fd = (int) socket(family, native_type, protocol)) < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call socket() to create socket"
    );
#ifdef P_OS_SCO
    p_profiler_free (ret->timer);
#endif
    p_free(ret);
    return NULL;
  }
#ifndef P_OS_WIN
  flags = fcntl(fd, F_GETFD, 0);

  if (P_LIKELY (flags != -1 && (flags & FD_CLOEXEC) == 0)) {
    flags |= FD_CLOEXEC;

    if (P_UNLIKELY (fcntl(fd, F_SETFD, flags) < 0))
      P_WARNING ("socket_t::p_socket_new: fcntl() with FD_CLOEXEC failed");
  }
#endif
  ret->fd = fd;
#ifdef P_OS_WIN
  ret->events = WSA_INVALID_EVENT;
#endif
  if (P_UNLIKELY (pp_socket_set_fd_blocking(ret->fd, false, error) == false)) {
    p_socket_free(ret);
    return NULL;
  }
#if !defined (P_OS_WIN) && defined (SO_NOSIGPIPE)
  flags = 1;

  if (setsockopt (ret->fd, SOL_SOCKET, SO_NOSIGPIPE, &flags, sizeof (flags)) < 0)
    P_WARNING ("socket_t::p_socket_new: setsockopt() with SO_NOSIGPIPE failed");
#endif
  ret->timeout = 0;
  ret->blocking = true;
  ret->family = family;
  ret->protocol = protocol;
  ret->type = type;
  p_socket_set_listen_backlog(ret, P_SOCKET_DEFAULT_BACKLOG);
#ifdef P_OS_WIN
  if (P_UNLIKELY ((ret->events = WSACreateEvent()) == WSA_INVALID_EVENT)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      (int) p_err_get_last_net(),
      "Failed to call WSACreateEvent() on socket"
    );
    p_socket_free(ret);
    return NULL;
  }
#endif
  return ret;
}

int
p_socket_get_fd(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return -1;
  }
  return socket->fd;
}

socket_family_t
p_socket_get_family(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return P_SOCKET_FAMILY_UNKNOWN;
  }
  return socket->family;
}

socket_kind_t
p_socket_get_type(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return P_SOCKET_UNKNOWN;
  }
  return socket->type;
}

socket_protocol_t
p_socket_get_protocol(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return P_SOCKET_PROTOCOL_UNKNOWN;
  }
  return socket->protocol;
}

bool
p_socket_get_keepalive(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return false;
  }
  return (bool) socket->keepalive;
}

bool
p_socket_get_blocking(socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return false;
  }
  return (bool) socket->blocking;
}

int
p_socket_get_listen_backlog(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return -1;
  }
  return socket->listen_backlog;
}

int
p_socket_get_timeout(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return -1;
  }
  return socket->timeout;
}

socketaddr_t *
p_socket_get_local_address(const socket_t *socket, err_t **error) {
  struct sockaddr_storage buffer;
  socklen_t len;
  socketaddr_t *ret;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  len = sizeof(buffer);
  if (P_UNLIKELY (
    getsockname(socket->fd, (struct sockaddr *) &buffer, &len) < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call getsockname() to get local socket address"
    );
    return NULL;
  }
  ret = p_socketaddr_new_from_native(&buffer, (size_t) len);
  if (P_UNLIKELY (ret == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      0,
      "Failed to create socket address from native structure"
    );
  }
  return ret;
}

socketaddr_t *
p_socket_get_remote_address(const socket_t *socket, err_t **error) {
  struct sockaddr_storage buffer;
  socklen_t len;
  socketaddr_t *ret;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  len = sizeof(buffer);
  if (P_UNLIKELY (
    getpeername(socket->fd, (struct sockaddr *) &buffer, &len) < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call getpeername() to get remote socket address"
    );
    return NULL;
  }
#ifdef P_OS_SYLLABLE
  /* Syllable has a bug with a wrong byte order for a TCP port,
   * as it only supports IPv4 we can easily fix it here. */
  ((struct sockaddr_in *) &buffer)->sin_port =
      p_htons (((struct sockaddr_in *) &buffer)->sin_port);
#endif
  ret = p_socketaddr_new_from_native(&buffer, (size_t) len);
  if (P_UNLIKELY (ret == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      0,
      "Failed to create socket address from native structure"
    );
  }
  return ret;
}

bool
p_socket_is_connected(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return false;
  }
  return (bool) socket->connected;
}

bool
p_socket_is_closed(const socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return true;
  }
  return (bool) socket->closed;
}

bool
p_socket_check_connect_result(socket_t *socket, err_t **error) {
  socklen_t optlen;
  int val;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  optlen = sizeof(val);
  if (P_UNLIKELY (
    getsockopt(socket->fd, SOL_SOCKET, SO_ERROR, (ptr_t) &val, &optlen)
      < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call getsockopt() to get connection status"
    );
    return false;
  }
  if (P_UNLIKELY (val != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(val),
      val,
      "Error in socket layer"
    );
  }
  socket->connected = (uint_t) (val == 0);
  return (val == 0);
}

void
p_socket_set_keepalive(socket_t *socket,
  bool keepalive) {
#ifdef P_OS_WIN
  byte_t value;
#else
  int value;
#endif
  if (P_UNLIKELY (socket == NULL)) {
    return;
  }
  if (socket->keepalive == (uint_t) !!keepalive) {
    return;
  }
#ifdef P_OS_WIN
  value = !!(byte_t) keepalive;
#else
  value = !!(int) keepalive;
#endif
  if (setsockopt(socket->fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value))
    < 0) {
    P_WARNING (
      "socket_t::p_socket_set_keepalive: setsockopt() with SO_KEEPALIVE failed");
    return;
  }
  socket->keepalive = !!(int) keepalive;
}

void
p_socket_set_blocking(socket_t *socket, bool blocking) {
  if (P_UNLIKELY (socket == NULL)) {
    return;
  }
  socket->blocking = !!blocking;
}

void
p_socket_set_listen_backlog(socket_t *socket, int backlog) {
  if (P_UNLIKELY (socket == NULL || socket->listening)) {
    return;
  }
  socket->listen_backlog = backlog;
}

void
p_socket_set_timeout(socket_t *socket, int timeout) {
  if (P_UNLIKELY (socket == NULL)) {
    return;
  }
  if (timeout < 0) {
    timeout = 0;
  }
  socket->timeout = timeout;
}

bool
p_socket_bind(const socket_t *socket, socketaddr_t *address, bool allow_reuse,
  err_t **error) {
  struct sockaddr_storage addr;
#ifdef SO_REUSEPORT
  bool reuse_port;
#endif
#ifdef P_OS_WIN
  byte_t value;
#else
  int value;
#endif

  if (P_UNLIKELY (socket == NULL || address == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }

  /* Windows allows to reuse the same address even for an active TCP
   * connection, that's why on Windows we should use SO_REUSEADDR only
   * for UDP sockets, UNIX doesn't have such behavior
   *
   * Ignore errors here, the only likely error is "not supported", and
   * this is a "best effort" thing mainly */

#ifdef P_OS_WIN
  value = !!(byte_t) (allow_reuse && (socket->type == P_SOCKET_DATAGRAM));
#else
  value = !!(int) allow_reuse;
#endif
  if (setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))
    < 0)
    P_WARNING ("socket_t::p_socket_bind: setsockopt() with SO_REUSEADDR failed");
#ifdef SO_REUSEPORT
  reuse_port = allow_reuse && (socket->type == P_SOCKET_DATAGRAM);

# ifdef P_OS_WIN
  value = !! (byte_t) reuse_port;
# else
  value = !!(int) reuse_port;
# endif

  if (setsockopt(socket->fd, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value))
    < 0)
    P_WARNING ("socket_t::p_socket_bind: setsockopt() with SO_REUSEPORT failed");
#endif
  if (P_UNLIKELY (
    p_socketaddr_to_native(address, &addr, sizeof(addr)) == false)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      0,
      "Failed to convert socket address to native structure"
    );
    return false;
  }
  if (P_UNLIKELY (bind(
    socket->fd,
    (struct sockaddr *) &addr,
    (socklen_t) p_socketaddr_get_native_size(address)) < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call bind() on socket"
    );
    return false;
  }
  return true;
}

bool
p_socket_connect(socket_t *socket, socketaddr_t *address, err_t **error) {
  struct sockaddr_storage buffer;
  int err_code;
  int conn_result;
  err_io_t sock_err;

  if (P_UNLIKELY (socket == NULL || address == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }
  if (P_UNLIKELY (
    p_socketaddr_to_native(address, &buffer, sizeof(buffer)) == false)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      0,
      "Failed to convert socket address to native structure"
    );
    return false;
  }
#if !defined (P_OS_WIN) && defined (EINTR)
  for (;;) {
    conn_result = connect(socket->fd, (struct sockaddr *) &buffer,
      (socklen_t) p_socketaddr_get_native_size(address));

    if (P_LIKELY (conn_result == 0))
      break;

    err_code = p_err_get_last_net();

    if (err_code == EINTR)
      continue;
    else
      break;
  }
#else
  conn_result = connect(
    socket->fd, (struct sockaddr *) &buffer,
    (int) p_socketaddr_get_native_size(address));
  if (conn_result != 0) {
    err_code = p_err_get_last_net();
  }
#endif
  if (conn_result == 0) {
    socket->connected = true;
    return true;
  }
  sock_err = p_err_get_io_from_system(err_code);
  if (P_LIKELY (
    sock_err == P_ERR_IO_WOULD_BLOCK || sock_err == P_ERR_IO_IN_PROGRESS)) {
    if (socket->blocking) {
      if (p_socket_io_condition_wait(
        socket,
        P_SOCKET_IO_CONDITION_POLLOUT,
        error
      ) == true &&
        p_socket_check_connect_result(socket, error) == true) {
        socket->connected = true;
        return true;
      }
    } else {
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Couldn't block non-blocking socket"
      );
    }
  } else {
    p_err_set_err_p(
      error,
      (int) sock_err,
      err_code,
      "Failed to call connect() on socket"
    );
  }
  return false;
}

bool
p_socket_listen(socket_t *socket, err_t **error) {
  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }
  if (P_UNLIKELY (listen(socket->fd, socket->listen_backlog) < 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call listen() on socket"
    );
    return false;
  }
  socket->listening = true;
  return true;
}

socket_t *
p_socket_accept(const socket_t *socket, err_t **error) {
  socket_t *ret;
  err_io_t sock_err;
  int res;
  int err_code;
#ifndef P_OS_WIN
  int flags;
#endif

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return NULL;
  }
  for (;;) {
    if (socket->blocking &&
      p_socket_io_condition_wait(
        socket,
        P_SOCKET_IO_CONDITION_POLLIN,
        error
      ) == false) {
        return NULL;
    }
    if ((res = (int) accept(socket->fd, NULL, 0)) < 0) {
      err_code = p_err_get_last_net();
#if !defined (P_OS_WIN) && defined (EINTR)
      if (p_err_get_last_net() == EINTR)
        continue;
#endif
      sock_err = p_err_get_io_from_system(err_code);
      if (socket->blocking && sock_err == P_ERR_IO_WOULD_BLOCK) {
        continue;
      }
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Failed to call accept() on socket"
      );
      return NULL;
    }
    break;
  }
#ifdef P_OS_WIN
  /* The socket inherits the accepting sockets event mask and even object,
   * we need to remove that */
  WSAEventSelect(res, NULL, 0);
#else
  flags = fcntl(res, F_GETFD, 0);

  if (P_LIKELY (flags != -1 && (flags & FD_CLOEXEC) == 0)) {
    flags |= FD_CLOEXEC;

    if (P_UNLIKELY (fcntl(res, F_SETFD, flags) < 0))
      P_WARNING ("socket_t::p_socket_accept: fcntl() with FD_CLOEXEC failed");
  }
#endif
  if (P_UNLIKELY ((ret = p_socket_new_from_fd(res, error)) == NULL)) {
    if (P_UNLIKELY (p_sys_close(res) != 0))
      P_WARNING ("socket_t::p_socket_accept: p_sys_close() failed");
  } else {
    ret->protocol = socket->protocol;
  }
  return ret;
}

ssize_t
p_socket_receive(const socket_t *socket, byte_t *buffer, size_t buflen,
  err_t **error) {
  err_io_t sock_err;
  ssize_t ret;
  int err_code;

  if (P_UNLIKELY (socket == NULL || buffer == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return -1;
  }
  for (;;) {
    if (socket->blocking &&
      p_socket_io_condition_wait(
        socket,
        P_SOCKET_IO_CONDITION_POLLIN,
        error
      ) == false) {
        return -1;
    }
    if ((ret = recv(socket->fd, buffer, (socklen_t) buflen, 0)) < 0) {
      err_code = p_err_get_last_net();
#if !defined (P_OS_WIN) && defined (EINTR)
      if (err_code == EINTR)
        continue;
#endif
      sock_err = p_err_get_io_from_system(err_code);
      if (socket->blocking && sock_err == P_ERR_IO_WOULD_BLOCK) {
        continue;
      }
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Failed to call recv() on socket"
      );
      return -1;
    }
    break;
  }
  return ret;
}

ssize_t
p_socket_receive_from(const socket_t *socket, socketaddr_t **address,
  byte_t *buffer, size_t buflen, err_t **error) {
  err_io_t sock_err;
  struct sockaddr_storage sa;
  socklen_t optlen;
  ssize_t ret;
  int err_code;

  if (P_UNLIKELY (socket == NULL || buffer == NULL || buflen == 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return -1;
  }
  optlen = sizeof(sa);
  for (;;) {
    if (socket->blocking &&
      p_socket_io_condition_wait(
        socket,
        P_SOCKET_IO_CONDITION_POLLIN,
        error
      ) == false) {
        return -1;
    }
    if ((
      ret = recvfrom(
        socket->fd,
        buffer,
        (socklen_t) buflen,
        0,
        (struct sockaddr *) &sa,
        &optlen
      )) < 0) {
      err_code = p_err_get_last_net();
#if !defined (P_OS_WIN) && defined (EINTR)
      if (err_code == EINTR)
        continue;
#endif
      sock_err = p_err_get_io_from_system(err_code);
      if (socket->blocking && sock_err == P_ERR_IO_WOULD_BLOCK) {
        continue;
      }
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Failed to call recvfrom() on socket"
      );
      return -1;
    }
    break;
  }
  if (address != NULL) {
    *address = p_socketaddr_new_from_native(&sa, optlen);
  }
  return ret;
}

ssize_t
p_socket_send(const socket_t *socket, const byte_t *buffer, size_t buflen,
  err_t **error) {
  err_io_t sock_err;
  ssize_t ret;
  int err_code;

  if (P_UNLIKELY (socket == NULL || buffer == NULL || buflen == 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return -1;
  }
  for (;;) {
    if (socket->blocking &&
      p_socket_io_condition_wait(
        socket,
        P_SOCKET_IO_CONDITION_POLLOUT,
        error
      ) == false) {
        return -1;
    }
    if ((
      ret = send(
        socket->fd,
        buffer,
        (socklen_t) buflen,
        P_SOCKET_DEFAULT_SEND_FLAGS
      )) < 0) {
      err_code = p_err_get_last_net();
#if !defined (P_OS_WIN) && defined (EINTR)
      if (err_code == EINTR)
        continue;
#endif
      sock_err = p_err_get_io_from_system(err_code);
      if (socket->blocking && sock_err == P_ERR_IO_WOULD_BLOCK) {
        continue;
      }
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Failed to call send() on socket"
      );
      return -1;
    }
    break;
  }
  return ret;
}

ssize_t
p_socket_send_to(const socket_t *socket, socketaddr_t *address,
  const byte_t *buffer, size_t buflen, err_t **error) {
  err_io_t sock_err;
  struct sockaddr_storage sa;
  socklen_t optlen;
  ssize_t ret;
  int err_code;

  if (!socket || !address || !buffer) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return -1;
  }
  if (!pp_socket_check(socket, error)) {
    return -1;
  }
  if (!p_socketaddr_to_native(address, &sa, sizeof(sa))) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_FAILED,
      0,
      "Failed to convert socket address to native structure"
    );
    return -1;
  }
  optlen = (socklen_t) p_socketaddr_get_native_size(address);
  for (;;) {
    if (socket->blocking &&
      p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLOUT, error)
        == false) {
          return -1;
    }
    if ((
      ret = sendto(
        socket->fd,
        buffer,
        (socklen_t) buflen,
        0,
        (struct sockaddr *) &sa,
        optlen
      )) < 0) {
      err_code = p_err_get_last_net();
#if !defined (P_OS_WIN) && defined (EINTR)
      if (err_code == EINTR)
        continue;
#endif
      sock_err = p_err_get_io_from_system(err_code);
      if (socket->blocking && sock_err == P_ERR_IO_WOULD_BLOCK) {
        continue;
      }
      p_err_set_err_p(
        error,
        (int) sock_err,
        err_code,
        "Failed to call sendto() on socket"
      );
      return -1;
    }
    break;
  }
  return ret;
}

bool
p_socket_close(socket_t *socket, err_t **error) {
  int err_code;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (socket->closed) {
    return true;
  }
  if (P_LIKELY (p_sys_close(socket->fd) == 0)) {
    socket->connected = false;
    socket->closed = true;
    socket->listening = false;
    socket->fd = -1;
    return true;
  } else {
    err_code = p_err_get_last_net();
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(err_code),
      err_code,
      "Failed to close socket"
    );
    return false;
  }
}

bool
p_socket_shutdown(socket_t *socket, bool shutdown_read, bool shutdown_write,
  err_t **error) {
  int how;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }
  if (P_UNLIKELY (shutdown_read == false && shutdown_write == false)) {
    return true;
  }
#ifndef P_OS_WIN
  if (shutdown_read == true && shutdown_write == true)
    how = SHUT_RDWR;
  else if (shutdown_read == true)
    how = SHUT_RD;
  else
    how = SHUT_WR;
#else
  if (shutdown_read == true && shutdown_write == true) {
    how = SD_BOTH;
  } else if (shutdown_read == true) {
    how = SD_RECEIVE;
  } else {
    how = SD_SEND;
  }
#endif
  if (P_UNLIKELY (shutdown(socket->fd, how) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call shutdown() on socket"
    );
    return false;
  }
  if (shutdown_read == true && shutdown_write == true) {
    socket->connected = false;
  }
  return true;
}

void
p_socket_free(socket_t *socket) {
  if (P_UNLIKELY (socket == NULL)) {
    return;
  }
#ifdef P_OS_WIN
  if (P_LIKELY (socket->events != WSA_INVALID_EVENT)) {
    WSACloseEvent(socket->events);
  }
#endif
  p_socket_close(socket, NULL);
#ifdef P_OS_SCO
  if (P_LIKELY (socket->timer != NULL))
    p_profiler_free (socket->timer);
#endif
  p_free(socket);
}

bool
p_socket_set_buffer_size(const socket_t *socket, socket_dir_t dir, size_t size,
  err_t **error) {
  int optname;
  int optval;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }
  optname = (dir == P_SOCKET_DIRECTION_RCV) ? SO_RCVBUF : SO_SNDBUF;
  optval = (int) size;
  if (P_UNLIKELY (setsockopt(
    socket->fd,
    SOL_SOCKET,
    optname,
    (const_ptr_t) &optval,
    sizeof(optval)) != 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call setsockopt() on socket to set buffer size"
    );
    return false;
  }
  return true;
}

bool
p_socket_io_condition_wait(const socket_t *socket, socket_io_cond_t condition,
  err_t **error) {
#if defined (P_OS_WIN)
  long network_events;
  int evret;
  int timeout;
  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (P_UNLIKELY (pp_socket_check(socket, error) == false)) {
    return false;
  }
  timeout = socket->timeout > 0 ? socket->timeout : WSA_INFINITE;
  if (condition == P_SOCKET_IO_CONDITION_POLLIN) {
    network_events = FD_READ | FD_ACCEPT;
  } else {
    network_events = FD_WRITE | FD_CONNECT;
  }
  WSAResetEvent(socket->events);
  WSAEventSelect(socket->fd, socket->events, network_events);
  evret = WSAWaitForMultipleEvents(
    1, (const HANDLE *) &socket->events, true, timeout, false
  );
  if (evret == WSA_WAIT_EVENT_0) {
    return true;
  } else if (evret == WSA_WAIT_TIMEOUT) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_TIMED_OUT,
      (int) p_err_get_last_net(),
      "Timed out while waiting socket condition"
    );
    return false;
  } else {
    p_err_set_err_p(
      error,
      (int) p_err_get_io_from_system(p_err_get_last_net()),
      (int) p_err_get_last_net(),
      "Failed to call WSAWaitForMultipleEvents() on socket"
    );
    return false;
  }
#elif defined (P_SOCKET_USE_POLL)
  struct pollfd pfd;
  int evret;
  int timeout;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p(error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument");
    return false;
  }

  if (P_UNLIKELY (pp_socket_check(socket, error) == false))
    return false;

  timeout = socket->timeout > 0 ? socket->timeout : -1;

  pfd.fd = socket->fd;
  pfd.revents = 0;

  if (condition == P_SOCKET_IO_CONDITION_POLLIN)
    pfd.events = POLLIN;
  else
    pfd.events = POLLOUT;

# ifdef P_OS_SCO
  p_profiler_reset (socket->timer);
# endif

  while (true) {
    evret = poll(&pfd, 1, timeout);

# ifdef EINTR
    if (evret == -1 && p_err_get_last_net() == EINTR) {
#   ifdef P_OS_SCO
      if (timeout < 0 ||
          (p_profiler_elapsed_usecs (socket->timer) / 1000) < (uint64_t) timeout)
        continue;
      else
        evret = 0;
#   else
      continue;
#   endif
    }
# endif

    if (evret == 1)
      return true;
    else if (evret == 0) {
      p_err_set_err_p(error,
        (int) P_ERR_IO_TIMED_OUT,
        (int) p_err_get_last_net(),
        "Timed out while waiting socket condition");
      return false;
    } else {
      p_err_set_err_p(error,
        (int) p_err_get_io_from_system(p_err_get_last_net()),
        (int) p_err_get_last_net(),
        "Failed to call poll() on socket");
      return false;
    }
  }
#else
  fd_set   fds;
  struct timeval  tv;
  struct timeval * ptv;
  int   evret;

  if (P_UNLIKELY (socket == NULL)) {
    p_err_set_err_p (error,
             (int) P_ERR_IO_INVALID_ARGUMENT,
             0,
             "Invalid input argument");
    return false;
  }

  if (P_UNLIKELY (pp_socket_check (socket, error) == false))
    return false;

  if (socket->timeout > 0)
    ptv = &tv;
  else
    ptv = NULL;

  while (true) {
    FD_ZERO (&fds);
    FD_SET (socket->fd, &fds);

    if (socket->timeout > 0) {
      tv.tv_sec  = socket->timeout / 1000;
      tv.tv_usec = (socket->timeout % 1000) * 1000;
    }

    if (condition == P_SOCKET_IO_CONDITION_POLLIN)
      evret = select (socket->fd + 1, &fds, NULL, NULL, ptv);
    else
      evret = select (socket->fd + 1, NULL, &fds, NULL, ptv);

#ifdef EINTR
    if (evret == -1 && p_err_get_last_net () == EINTR)
      continue;
#endif

    if (evret == 1)
      return true;
    else if (evret == 0) {
      p_err_set_err_p (error,
               (int) P_ERR_IO_TIMED_OUT,
               (int) p_err_get_last_net (),
               "Timed out while waiting socket condition");
      return false;
    } else {
      p_err_set_err_p (error,
               (int) p_err_get_io_from_system (p_err_get_last_net ()),
               (int) p_err_get_last_net (),
               "Failed to call select() on socket");
      return false;
    }
  }
#endif
}
