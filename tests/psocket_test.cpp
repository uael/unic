/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE psocket_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static byte_t socket_data[] = "This is a socket test data!";
volatile static bool is_sender_working = false;
volatile static bool is_receiver_working = false;

typedef struct _SocketTestData {
  uint16_t sender_port;
  uint16_t receiver_port;
  bool shutdown_channel;
} SocketTestData;

extern "C" ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

extern "C" ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED (block);
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

extern "C" void
pmem_free(ptr_t block) {
  P_UNUSED (block);
}

static void
clean_error(err_t **error) {
  if (error == NULL || *error == NULL)
    return;

  p_error_free(*error);
  *error = NULL;
}

static bool
test_socketaddr_directly(const socketaddr_t *addr, uint16_t port) {
  if (addr == NULL)
    return false;

  byte_t *addr_str = p_socketaddr_get_address(addr);
  socket_family_t remote_family = p_socketaddr_get_family(addr);
  uint16_t remote_port = p_socketaddr_get_port(addr);
  size_t remote_size = p_socketaddr_get_native_size(addr);

  bool ret =
    (strcmp(addr_str, "127.0.0.1") == 0 && remote_family == P_SOCKET_FAMILY_INET
      &&
        remote_port == port && remote_size > 0) ? true : false;

  p_free(addr_str);

  return ret;
}

static bool
test_socketaddr(socket_t *socket, uint16_t port) {
  /* Test remote address */
  socketaddr_t *remote_addr = p_socket_get_remote_address(socket, NULL);

  if (remote_addr == NULL)
    return false;

  bool ret = test_socketaddr_directly(remote_addr, port);

  p_socketaddr_free(remote_addr);

  return ret;
}

static bool
compare_socketaddres(const socketaddr_t *addr1, const socketaddr_t *addr2) {
  if (addr1 == NULL || addr2 == NULL)
    return false;

  byte_t *addr_str1 = p_socketaddr_get_address(addr1);
  byte_t *addr_str2 = p_socketaddr_get_address(addr2);

  if (addr_str1 == NULL || addr_str2 == NULL) {
    p_free(addr_str1);
    p_free(addr_str2);

    return false;
  }

  bool addr_cmp = (strcmp(addr_str1, addr_str2) == 0 ? true : false);

  p_free(addr_str1);
  p_free(addr_str2);

  if (addr_cmp == false)
    return false;

  if (p_socketaddr_get_family(addr1) != p_socketaddr_get_family(addr2))
    return false;

  if (p_socketaddr_get_native_size(addr1)
    != p_socketaddr_get_native_size(addr2))
    return false;

  return true;
}

static void *
udp_socket_sender_thread(void *arg) {
  int_t send_counter = 0;

  if (arg == NULL)
    p_uthread_exit(-1);

  SocketTestData *data = (SocketTestData *) (arg);

  /* Create sender socket */
  socket_t *skt_sender = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_DATAGRAM,
    P_SOCKET_PROTOCOL_UDP,
    NULL);

  if (skt_sender == NULL)
    p_uthread_exit(-1);

  socketaddr_t *addr_sender = p_socketaddr_new("127.0.0.1", data->sender_port);

  if (addr_sender == NULL) {
    p_socket_free(skt_sender);
    p_uthread_exit(-1);
  }

  if (p_socket_bind(skt_sender, addr_sender, false, NULL) == false) {
    p_socket_free(skt_sender);
    p_socketaddr_free(addr_sender);
    p_uthread_exit(-1);
  } else {
    p_socketaddr_free(addr_sender);

    socketaddr_t *local_addr = p_socket_get_local_address(skt_sender, NULL);

    if (local_addr == NULL) {
      p_socket_free(skt_sender);
      p_uthread_exit(-1);
    }

    data->sender_port = p_socketaddr_get_port(local_addr);

    p_socketaddr_free(local_addr);
  }

  p_socket_set_timeout(skt_sender, 50);

  /* Test that remote address is NULL */
  socketaddr_t *remote_addr = p_socket_get_remote_address(skt_sender, NULL);

  if (remote_addr != NULL) {
    if (p_socketaddr_is_any(remote_addr) == false) {
      p_socketaddr_free(remote_addr);
      p_socket_free(skt_sender);
      p_uthread_exit(-1);
    } else {
      p_socketaddr_free(remote_addr);
      remote_addr = NULL;
    }
  }

  /* Test that we are not connected */
  if (p_socket_is_connected(skt_sender) == true) {
    p_socket_free(skt_sender);
    p_uthread_exit(-1);
  }

  while (is_sender_working == true && data->receiver_port == 0) {
    p_uthread_sleep(1);
    continue;
  }

  socketaddr_t *addr_receiver = NULL;

  if (data->receiver_port != 0)
    addr_receiver = p_socketaddr_new("127.0.0.1", data->receiver_port);

  while (is_sender_working == true) {
    if (data->receiver_port == 0)
      break;

    if (p_socket_send_to(skt_sender,
      addr_receiver,
      socket_data,
      sizeof(socket_data),
      NULL) == sizeof(socket_data))
      ++send_counter;

    p_uthread_sleep(1);
  }

  p_socketaddr_free(addr_receiver);
  p_socket_free(skt_sender);
  p_uthread_exit(send_counter);

  return NULL;
}

static void *
udp_socket_receiver_thread(void *arg) {
  byte_t recv_buffer[sizeof(socket_data) * 3];
  int_t recv_counter = 0;

  if (arg == NULL)
    p_uthread_exit(-1);

  SocketTestData *data = (SocketTestData *) (arg);

  /* Create receiving socket */
  socket_t *skt_receiver = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_DATAGRAM,
    P_SOCKET_PROTOCOL_UDP,
    NULL);

  if (skt_receiver == NULL)
    p_uthread_exit(-1);

  p_socket_set_blocking(skt_receiver, false);

  socketaddr_t
    *addr_receiver = p_socketaddr_new("127.0.0.1", data->receiver_port);

  if (addr_receiver == NULL) {
    p_socket_free(skt_receiver);
    p_uthread_exit(-1);
  }

  if (p_socket_bind(skt_receiver, addr_receiver, true, NULL) == false) {
    p_socket_free(skt_receiver);
    p_socketaddr_free(addr_receiver);
    p_uthread_exit(-1);
  } else {
    p_socketaddr_free(addr_receiver);

    socketaddr_t *local_addr = p_socket_get_local_address(skt_receiver, NULL);

    if (local_addr == NULL) {
      p_socket_free(skt_receiver);
      p_uthread_exit(-1);
    }

    data->receiver_port = p_socketaddr_get_port(local_addr);

    p_socketaddr_free(local_addr);
  }

  p_socket_set_timeout(skt_receiver, 50);

  /* Test that remote address is NULL */
  socketaddr_t *remote_addr = p_socket_get_remote_address(skt_receiver, NULL);

  if (remote_addr != NULL) {
    if (p_socketaddr_is_any(remote_addr) == false) {
      p_socketaddr_free(remote_addr);
      p_socket_free(skt_receiver);
      p_uthread_exit(-1);
    } else {
      p_socketaddr_free(remote_addr);
      remote_addr = NULL;
    }
  }

  /* Test that we are not connected */
  if (p_socket_is_connected(skt_receiver) == true) {
    p_socket_free(skt_receiver);
    p_uthread_exit(-1);
  }

  while (is_receiver_working == true) {
    socketaddr_t *remote_addr = NULL;

    ssize_t received = p_socket_receive_from(skt_receiver,
      &remote_addr,
      recv_buffer,
      sizeof(recv_buffer),
      NULL);

    if (remote_addr != NULL
      && test_socketaddr_directly(remote_addr, data->sender_port) == false) {
      p_socketaddr_free(remote_addr);
      break;
    }

    p_socketaddr_free(remote_addr);

    if (received == sizeof(socket_data))
      ++recv_counter;
    else if (received > 0) {
      p_socket_free(skt_receiver);
      p_uthread_exit(-1);
    }

    p_uthread_sleep(1);
  }

  p_socket_free(skt_receiver);
  p_uthread_exit(recv_counter);

  return NULL;
}

static void *
tcp_socket_sender_thread(void *arg) {
  int_t send_counter = 0;
  size_t send_total;
  ssize_t send_now;
  bool is_connected = false;

  if (arg == NULL)
    p_uthread_exit(-1);

  SocketTestData *data = (SocketTestData *) (arg);

  /* Create sender socket */
  socket_t *skt_sender = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_STREAM,
    P_SOCKET_PROTOCOL_DEFAULT,
    NULL);

  if (skt_sender == NULL)
    p_uthread_exit(-1);

  p_socket_set_timeout(skt_sender, 2000);

  if (p_socket_get_fd(skt_sender) < 0) {
    p_socket_free(skt_sender);
    p_uthread_exit(-1);
  }

  while (is_sender_working == true && data->receiver_port == 0) {
    p_uthread_sleep(1);
    continue;
  }

  socketaddr_t *addr_sender = p_socketaddr_new("127.0.0.1", data->sender_port);

  if (addr_sender == NULL) {
    p_socket_free(skt_sender);
    p_uthread_exit(-1);
  }

  if (p_socket_bind(skt_sender, addr_sender, false, NULL) == false) {
    p_socket_free(skt_sender);
    p_socketaddr_free(addr_sender);
    p_uthread_exit(-1);
  } else {
    p_socketaddr_free(addr_sender);

    socketaddr_t *local_addr = p_socket_get_local_address(skt_sender, NULL);

    if (local_addr == NULL) {
      p_socket_free(skt_sender);
      p_uthread_exit(-1);
    }

    data->sender_port = p_socketaddr_get_port(local_addr);

    p_socketaddr_free(local_addr);
  }

  send_total = 0;
  send_now = 0;

  while (is_sender_working == true && data->receiver_port == 0) {
    p_uthread_sleep(1);
    continue;
  }

  socketaddr_t *addr_receiver = NULL;

  /* Try to connect in non-blocking mode */
  p_socket_set_blocking(skt_sender, false);

  if (data->receiver_port != 0) {
    addr_receiver = p_socketaddr_new("127.0.0.1", data->receiver_port);
    is_connected = p_socket_connect(skt_sender, addr_receiver, NULL);

    if (is_connected == false) {
      if (p_socket_io_condition_wait(skt_sender, P_SOCKET_IO_CONDITION_POLLOUT,
        NULL) == true &&
        p_socket_check_connect_result(skt_sender, NULL) == false) {
        p_socketaddr_free(addr_receiver);
        p_socket_free(skt_sender);
        p_uthread_exit(-1);
      }
    }

    is_connected = p_socket_is_connected(skt_sender);

    if (is_connected == true && p_socket_shutdown(skt_sender,
      false,
      data->shutdown_channel,
      NULL) == false)
      is_connected = false;
  }

  if (data->shutdown_channel == true
    && p_socket_is_closed(skt_sender) == true) {
    p_socketaddr_free(addr_receiver);
    p_socket_free(skt_sender);
    p_uthread_exit(-1);
  }

  p_socket_set_blocking(skt_sender, true);

  while (is_sender_working == true) {
    if (data->receiver_port == 0 || is_connected == false)
      break;

    if (test_socketaddr(skt_sender, data->receiver_port) == false)
      break;

    if (data->shutdown_channel == false
      && p_socket_is_connected(skt_sender) == false) {
      p_socketaddr_free(addr_receiver);
      p_socket_free(skt_sender);
      p_uthread_exit(-1);
    }

    send_now = p_socket_send(skt_sender,
      socket_data + send_total,
      sizeof(socket_data) - send_total,
      NULL);

    if (send_now > 0)
      send_total += (size_t) send_now;

    if (send_total == sizeof(socket_data)) {
      send_total = 0;
      ++send_counter;
    }

    p_uthread_sleep(1);
  }

  if (p_socket_close(skt_sender, NULL) == false)
    send_counter = -1;

  p_socketaddr_free(addr_receiver);
  p_socket_free(skt_sender);
  p_uthread_exit(send_counter);

  return NULL;
}

static void *
tcp_socket_receiver_thread(void *arg) {
  byte_t recv_buffer[sizeof(socket_data)];
  int_t recv_counter = 0;
  size_t recv_total;
  ssize_t recv_now;

  if (arg == NULL)
    p_uthread_exit(-1);

  SocketTestData *data = (SocketTestData *) (arg);

  /* Create receiving socket */
  socket_t *skt_receiver = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_STREAM,
    P_SOCKET_PROTOCOL_TCP,
    NULL);

  if (skt_receiver == NULL)
    p_uthread_exit(-1);

  socketaddr_t
    *addr_receiver = p_socketaddr_new("127.0.0.1", data->receiver_port);

  if (addr_receiver == NULL) {
    p_socket_free(skt_receiver);
    p_uthread_exit(-1);
  }

  p_socket_set_timeout(skt_receiver, 2000);

  if (p_socket_bind(skt_receiver, addr_receiver, true, NULL) == false ||
    p_socket_listen(skt_receiver, NULL) == false) {
    p_socket_free(skt_receiver);
    p_socketaddr_free(addr_receiver);
    p_uthread_exit(-1);
  } else {
    p_socketaddr_free(addr_receiver);

    socketaddr_t *local_addr = p_socket_get_local_address(skt_receiver, NULL);

    if (local_addr == NULL) {
      p_socket_free(skt_receiver);
      p_uthread_exit(-1);
    }

    data->receiver_port = p_socketaddr_get_port(local_addr);

    p_socketaddr_free(local_addr);
  }

  socket_t *conn_socket = NULL;
  recv_total = 0;
  recv_now = 0;

  while (is_receiver_working == true) {
    if (conn_socket == NULL) {
      conn_socket = p_socket_accept(skt_receiver, NULL);

      if (conn_socket == NULL) {
        p_uthread_sleep(1);
        continue;
      } else {
        /* On Syllable there is a bug in TCP which changes a local port
         * of the client socket which connects to a server */
#ifndef P_OS_SYLLABLE
        if (test_socketaddr(conn_socket, data->sender_port) == false)
          break;
#endif

        if (p_socket_shutdown(conn_socket, data->shutdown_channel, false, NULL)
          == false)
          break;

        p_socket_set_timeout(conn_socket, 2000);
      }
    }

    if ((data->shutdown_channel == false
      && p_socket_is_connected(conn_socket) == false) ||
      (data->shutdown_channel == true
        && p_socket_is_closed(conn_socket) == true)) {
      p_socket_free(conn_socket);
      p_socket_free(skt_receiver);
      p_uthread_exit(-1);
    }

    recv_now = p_socket_receive(conn_socket,
      recv_buffer + recv_total,
      sizeof(recv_buffer) - recv_total,
      NULL);

    if (recv_now > 0)
      recv_total += (size_t) recv_now;

    if (recv_total == sizeof(recv_buffer)) {
      recv_total = 0;

      if (strncmp(recv_buffer, socket_data, sizeof(recv_buffer)) == 0)
        ++recv_counter;

      memset(recv_buffer, 0, sizeof(recv_buffer));
    }

    p_uthread_sleep(1);
  }

  if (p_socket_close(skt_receiver, NULL) == false)
    recv_counter = -1;

  p_socket_free(conn_socket);
  p_socket_free(skt_receiver);

  p_uthread_exit(recv_counter);

  return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psocket_nomem_test) {
  p_libsys_init();

  socket_t *socket = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_DATAGRAM,
    P_SOCKET_PROTOCOL_UDP,
    NULL);
  BOOST_CHECK (socket != NULL);

  socketaddr_t *sock_addr = p_socketaddr_new("127.0.0.1", 32211);

  BOOST_CHECK (sock_addr != NULL);
  BOOST_CHECK (p_socket_bind(socket, sock_addr, true, NULL) == true);

  p_socketaddr_free(sock_addr);

  p_socket_set_timeout(socket, 1000);
  sock_addr = p_socketaddr_new("127.0.0.1", 32215);
  BOOST_CHECK (sock_addr != NULL);
  BOOST_CHECK (p_socket_connect(socket, sock_addr, NULL) == true);

  p_socketaddr_free(sock_addr);

  mem_vtable_t vtable;

  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;

  BOOST_CHECK (p_mem_set_vtable(&vtable) == true);

  BOOST_CHECK (p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_DATAGRAM,
    P_SOCKET_PROTOCOL_UDP,
    NULL) == NULL);
  BOOST_CHECK (p_socket_new_from_fd(p_socket_get_fd(socket), NULL) == NULL);
  BOOST_CHECK (p_socket_get_local_address(socket, NULL) == NULL);
  BOOST_CHECK (p_socket_get_remote_address(socket, NULL) == NULL);

  p_mem_restore_vtable();

  p_socket_close(socket, NULL);
  p_socket_free(socket);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_bad_input_test) {
  p_libsys_init();

  err_t *error = NULL;

  BOOST_CHECK (p_socket_new_from_fd(-1, &error) == NULL);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_new(P_SOCKET_FAMILY_INET,
    (socket_kind_t) -1,
    P_SOCKET_PROTOCOL_TCP,
    NULL) == NULL);
  /* Syllable doesn't validate socket family */
#ifndef P_OS_SYLLABLE
  BOOST_CHECK (p_socket_new((socket_family_t) -1,
    P_SOCKET_SEQPACKET,
    P_SOCKET_PROTOCOL_TCP,
    NULL) == NULL);
#endif
  BOOST_CHECK (p_socket_new(P_SOCKET_FAMILY_UNKNOWN,
    P_SOCKET_UNKNOWN,
    P_SOCKET_PROTOCOL_UNKNOWN,
    &error) == NULL);
  BOOST_CHECK (p_socket_new_from_fd(1, NULL) == NULL);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_get_fd(NULL) == -1);
  BOOST_CHECK (p_socket_get_family(NULL) == P_SOCKET_FAMILY_UNKNOWN);
  BOOST_CHECK (p_socket_get_type(NULL) == P_SOCKET_UNKNOWN);
  BOOST_CHECK (p_socket_get_protocol(NULL) == P_SOCKET_PROTOCOL_UNKNOWN);
  BOOST_CHECK (p_socket_get_keepalive(NULL) == false);
  BOOST_CHECK (p_socket_get_blocking(NULL) == false);
  BOOST_CHECK (p_socket_get_timeout(NULL) == -1);
  BOOST_CHECK (p_socket_get_listen_backlog(NULL) == -1);
  BOOST_CHECK (
    p_socket_io_condition_wait(NULL, P_SOCKET_IO_CONDITION_POLLIN, NULL)
      == false);
  BOOST_CHECK (
    p_socket_io_condition_wait(NULL, P_SOCKET_IO_CONDITION_POLLOUT, NULL)
      == false);

  BOOST_CHECK (p_socket_get_local_address(NULL, &error) == NULL);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_get_remote_address(NULL, &error) == NULL);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_is_connected(NULL) == false);
  BOOST_CHECK (p_socket_is_closed(NULL) == true);

  BOOST_CHECK (p_socket_check_connect_result(NULL, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  p_socket_set_keepalive(NULL, false);
  p_socket_set_blocking(NULL, false);
  p_socket_set_timeout(NULL, 0);
  p_socket_set_listen_backlog(NULL, 0);

  BOOST_CHECK (p_socket_bind(NULL, NULL, false, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_connect(NULL, NULL, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_listen(NULL, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_accept(NULL, &error) == NULL);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_receive(NULL, NULL, 0, &error) == -1);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_receive_from(NULL, NULL, NULL, 0, &error) == -1);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_send(NULL, NULL, 0, &error) == -1);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_send_to(NULL, NULL, NULL, 0, &error) == -1);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_close(NULL, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (p_socket_shutdown(NULL, false, false, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (
    p_socket_set_buffer_size(NULL, P_SOCKET_DIRECTION_RCV, 0, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  BOOST_CHECK (
    p_socket_set_buffer_size(NULL, P_SOCKET_DIRECTION_SND, 0, &error) == false);
  BOOST_CHECK (error != NULL);
  clean_error(&error);

  p_socket_free(NULL);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_general_udp_test) {
  p_libsys_init();

  /* Test UDP socket */
  socket_t *socket = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_DATAGRAM,
    P_SOCKET_PROTOCOL_UDP,
    NULL);

  BOOST_CHECK (socket != NULL);
  BOOST_CHECK (p_socket_get_family(socket) == P_SOCKET_FAMILY_INET);
  BOOST_CHECK (p_socket_get_fd(socket) >= 0);
  BOOST_CHECK (p_socket_get_listen_backlog(socket) == 5);
  BOOST_CHECK (p_socket_get_timeout(socket) == 0);

  /* On some operating systems (i.e. OpenVMS) remote address is not NULL */
  socketaddr_t *remote_addr = p_socket_get_remote_address(socket, NULL);

  if (remote_addr != NULL) {
    BOOST_CHECK (p_socketaddr_is_any(remote_addr) == true);
    p_socketaddr_free(remote_addr);
    remote_addr = NULL;
  }

  BOOST_CHECK (p_socket_get_protocol(socket) == P_SOCKET_PROTOCOL_UDP);
  BOOST_CHECK (p_socket_get_blocking(socket) == true);
  BOOST_CHECK (p_socket_get_type(socket) == P_SOCKET_DATAGRAM);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);
  BOOST_CHECK (p_socket_is_closed(socket) == false);

  p_socket_set_listen_backlog(socket, 12);
  p_socket_set_timeout(socket, -10);
  BOOST_CHECK (p_socket_get_timeout(socket) == 0);
  p_socket_set_timeout(socket, 10);

  BOOST_CHECK (p_socket_get_listen_backlog(socket) == 12);
  BOOST_CHECK (p_socket_get_timeout(socket) == 10);

  socketaddr_t *sock_addr = p_socketaddr_new("127.0.0.1", 32111);
  BOOST_CHECK (sock_addr != NULL);

  BOOST_CHECK (p_socket_bind(socket, sock_addr, true, NULL) == true);

  /* Test creating socket from descriptor */
  socket_t *fd_socket = p_socket_new_from_fd(p_socket_get_fd(socket), NULL);
  BOOST_CHECK (fd_socket != NULL);
  BOOST_CHECK (p_socket_get_family(fd_socket) == P_SOCKET_FAMILY_INET);
  BOOST_CHECK (p_socket_get_fd(fd_socket) >= 0);
  BOOST_CHECK (p_socket_get_listen_backlog(fd_socket) == 5);
  BOOST_CHECK (p_socket_get_timeout(fd_socket) == 0);

  remote_addr = p_socket_get_remote_address(fd_socket, NULL);

  if (remote_addr != NULL) {
    BOOST_CHECK (p_socketaddr_is_any(remote_addr) == true);
    p_socketaddr_free(remote_addr);
    remote_addr = NULL;
  }

  BOOST_CHECK (p_socket_get_protocol(fd_socket) == P_SOCKET_PROTOCOL_UDP);
  BOOST_CHECK (p_socket_get_blocking(fd_socket) == true);
  BOOST_CHECK (p_socket_get_type(fd_socket) == P_SOCKET_DATAGRAM);
  BOOST_CHECK (p_socket_get_keepalive(fd_socket) == false);
  BOOST_CHECK (p_socket_is_closed(fd_socket) == false);

  p_socket_set_keepalive(fd_socket, false);
  BOOST_CHECK (p_socket_get_keepalive(fd_socket) == false);

  p_socket_set_keepalive(fd_socket, true);
  p_socket_set_keepalive(fd_socket, false);
  BOOST_CHECK (p_socket_get_keepalive(fd_socket) == false);

  /* Test UDP local address */
  socketaddr_t *addr = p_socket_get_local_address(socket, NULL);
  BOOST_CHECK (addr != NULL);

  BOOST_CHECK (compare_socketaddres(sock_addr, addr) == true);

  p_socketaddr_free(sock_addr);
  p_socketaddr_free(addr);

  /* Test UDP connecting to remote address */
  p_socket_set_timeout(socket, 1000);
  addr = p_socketaddr_new("127.0.0.1", 32115);
  BOOST_CHECK (addr != NULL);
  BOOST_CHECK (p_socket_connect(socket, addr, NULL) == true);

  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLIN, NULL)
      == false);
  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLOUT, NULL)
      == true);

  sock_addr = p_socket_get_remote_address(socket, NULL);

  /* Syllable doesn't support getpeername() for UDP sockets */
#ifdef P_OS_SYLLABLE
  BOOST_CHECK (sock_addr == NULL);
  sock_addr = p_socketaddr_new ("127.0.0.1", 32115);
  BOOST_CHECK (addr != NULL);
#else
  BOOST_CHECK (sock_addr != NULL);
  BOOST_CHECK (compare_socketaddres(sock_addr, addr) == true);
#endif

  /* Not supported on Syllable */
#ifndef P_OS_SYLLABLE
  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_RCV, 72 * 1024, NULL)
      == true);
  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_SND, 72 * 1024, NULL)
      == true);
  BOOST_CHECK (p_socket_check_connect_result(socket, NULL) == true);
#endif

  BOOST_CHECK (p_socket_is_connected(socket) == true);
  BOOST_CHECK (p_socket_close(socket, NULL) == true);

  byte_t sock_buf[10];

  BOOST_CHECK (p_socket_bind(socket, sock_addr, true, NULL) == false);
  BOOST_CHECK (p_socket_connect(socket, addr, NULL) == false);
  BOOST_CHECK (p_socket_listen(socket, NULL) == false);
  BOOST_CHECK (p_socket_accept(socket, NULL) == false);
  BOOST_CHECK (
    p_socket_receive(socket, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (
    p_socket_receive_from(socket, NULL, sock_buf, sizeof(sock_buf), NULL)
      == -1);
  BOOST_CHECK (p_socket_send(socket, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (
    p_socket_send_to(socket, addr, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (p_socket_shutdown(socket, true, true, NULL) == false);
  BOOST_CHECK (p_socket_get_local_address(socket, NULL) == NULL);
  BOOST_CHECK (p_socket_check_connect_result(socket, NULL) == false);
  BOOST_CHECK (p_socket_get_fd(socket) == -1);
  BOOST_CHECK (p_socket_is_connected(socket) == false);
  BOOST_CHECK (p_socket_is_closed(socket) == true);

  p_socket_set_keepalive(socket, true);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);

  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLIN, NULL)
      == false);
  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLOUT, NULL)
      == false);

  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_RCV, 72 * 1024, NULL)
      == false);
  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_SND, 72 * 1024, NULL)
      == false);

  p_socketaddr_free(sock_addr);
  p_socketaddr_free(addr);
  p_socket_free(socket);
  p_socket_free(fd_socket);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_general_tcp_test) {
  p_libsys_init();

  /* Test TCP socket */
  socket_t *socket = p_socket_new(P_SOCKET_FAMILY_INET,
    P_SOCKET_STREAM,
    P_SOCKET_PROTOCOL_TCP,
    NULL);
  p_socket_set_blocking(socket, false);
  p_socket_set_listen_backlog(socket, 11);

  p_socket_set_timeout(socket, -12);
  BOOST_CHECK (p_socket_get_timeout(socket) == 0);
  p_socket_set_timeout(socket, 12);

  BOOST_CHECK (socket != NULL);
  BOOST_CHECK (p_socket_get_family(socket) == P_SOCKET_FAMILY_INET);
  BOOST_CHECK (p_socket_get_fd(socket) >= 0);
  BOOST_CHECK (p_socket_get_listen_backlog(socket) == 11);
  BOOST_CHECK (p_socket_get_timeout(socket) == 12);
  BOOST_CHECK (p_socket_get_remote_address(socket, NULL) == NULL);
  BOOST_CHECK (p_socket_get_protocol(socket) == P_SOCKET_PROTOCOL_TCP);
  BOOST_CHECK (p_socket_get_blocking(socket) == false);
  BOOST_CHECK (p_socket_get_type(socket) == P_SOCKET_STREAM);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);
  BOOST_CHECK (p_socket_is_closed(socket) == false);

  p_socket_set_keepalive(socket, false);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);

  p_socket_set_keepalive(socket, true);
  p_socket_set_keepalive(socket, false);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);

  socketaddr_t *sock_addr = p_socketaddr_new("127.0.0.1", 0);
  BOOST_CHECK (sock_addr != NULL);

  BOOST_CHECK (p_socket_bind(socket, sock_addr, true, NULL) == true);

  socketaddr_t *addr = p_socket_get_local_address(socket, NULL);
  BOOST_CHECK (addr != NULL);

  BOOST_CHECK (compare_socketaddres(sock_addr, addr) == true);

  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_RCV, 72 * 1024, NULL)
      == true);
  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_SND, 72 * 1024, NULL)
      == true);

  /* In case of success p_socket_check_connect_result() marks socket as connected */
  BOOST_CHECK (p_socket_is_connected(socket) == false);
  BOOST_CHECK (p_socket_check_connect_result(socket, NULL) == true);
  BOOST_CHECK (p_socket_close(socket, NULL) == true);

  byte_t sock_buf[10];

  BOOST_CHECK (p_socket_bind(socket, sock_addr, true, NULL) == false);
  BOOST_CHECK (p_socket_connect(socket, addr, NULL) == false);
  BOOST_CHECK (p_socket_listen(socket, NULL) == false);
  BOOST_CHECK (p_socket_accept(socket, NULL) == false);
  BOOST_CHECK (
    p_socket_receive(socket, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (
    p_socket_receive_from(socket, NULL, sock_buf, sizeof(sock_buf), NULL)
      == -1);
  BOOST_CHECK (p_socket_send(socket, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (
    p_socket_send_to(socket, addr, sock_buf, sizeof(sock_buf), NULL) == -1);
  BOOST_CHECK (p_socket_shutdown(socket, true, true, NULL) == false);
  BOOST_CHECK (p_socket_get_local_address(socket, NULL) == NULL);
  BOOST_CHECK (p_socket_check_connect_result(socket, NULL) == false);
  BOOST_CHECK (p_socket_is_closed(socket) == true);
  BOOST_CHECK (p_socket_get_fd(socket) == -1);

  p_socket_set_keepalive(socket, true);
  BOOST_CHECK (p_socket_get_keepalive(socket) == false);

  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLIN, NULL)
      == false);
  BOOST_CHECK (
    p_socket_io_condition_wait(socket, P_SOCKET_IO_CONDITION_POLLOUT, NULL)
      == false);

  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_RCV, 72 * 1024, NULL)
      == false);
  BOOST_CHECK (
    p_socket_set_buffer_size(socket, P_SOCKET_DIRECTION_SND, 72 * 1024, NULL)
      == false);

  p_socketaddr_free(sock_addr);
  p_socketaddr_free(addr);

  p_socket_free(socket);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_udp_test) {
  p_libsys_init();

  is_sender_working = true;
  is_receiver_working = true;

  SocketTestData data;
  data.receiver_port = 0;
  data.sender_port = 0;
  data.shutdown_channel = false;

  uthread_t *receiver_thr =
    p_uthread_create((uthread_fn_t) udp_socket_receiver_thread, (ptr_t) &data,
      true);
  uthread_t *sender_thr =
    p_uthread_create((uthread_fn_t) udp_socket_sender_thread, (ptr_t) &data,
      true);

  BOOST_CHECK (sender_thr != NULL);
  BOOST_CHECK (receiver_thr != NULL);

  p_uthread_sleep(8000);

  is_sender_working = false;
  int_t send_counter = p_uthread_join(sender_thr);

  p_uthread_sleep(2000);

  is_receiver_working = false;
  int_t recv_counter = p_uthread_join(receiver_thr);

  BOOST_CHECK (send_counter > 0);
  BOOST_CHECK (recv_counter > 0);

  p_uthread_unref(sender_thr);
  p_uthread_unref(receiver_thr);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_tcp_test) {
  p_libsys_init();

  is_sender_working = true;
  is_receiver_working = true;

  SocketTestData data;
  data.receiver_port = 0;
  data.sender_port = 0;
  data.shutdown_channel = false;

  uthread_t *receiver_thr =
    p_uthread_create((uthread_fn_t) tcp_socket_receiver_thread, (ptr_t) &data,
      true);
  uthread_t *sender_thr =
    p_uthread_create((uthread_fn_t) tcp_socket_sender_thread, (ptr_t) &data,
      true);

  BOOST_CHECK (receiver_thr != NULL);
  BOOST_CHECK (sender_thr != NULL);

  p_uthread_sleep(8000);

  is_sender_working = false;
  int_t send_counter = p_uthread_join(sender_thr);

  p_uthread_sleep(2000);

  is_receiver_working = false;
  int_t recv_counter = p_uthread_join(receiver_thr);

  BOOST_CHECK (send_counter > 0);
  BOOST_CHECK (recv_counter > 0);

  p_uthread_unref(sender_thr);
  p_uthread_unref(receiver_thr);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (psocket_shutdown_test) {
  p_libsys_init();

  is_sender_working = true;
  is_receiver_working = true;

  SocketTestData data;
  data.receiver_port = 0;
  data.sender_port = 0;
  data.shutdown_channel = true;

  uthread_t *receiver_thr =
    p_uthread_create((uthread_fn_t) tcp_socket_receiver_thread, (ptr_t) &data,
      true);
  uthread_t *sender_thr =
    p_uthread_create((uthread_fn_t) tcp_socket_sender_thread, (ptr_t) &data,
      true);

  BOOST_CHECK (receiver_thr != NULL);
  BOOST_CHECK (sender_thr != NULL);

  p_uthread_sleep(8000);

  is_sender_working = false;
  int_t send_counter = p_uthread_join(sender_thr);

  p_uthread_sleep(2000);

  is_receiver_working = false;
  int_t recv_counter = p_uthread_join(receiver_thr);

  BOOST_CHECK (send_counter == 0);
  BOOST_CHECK (recv_counter == 0);

  p_uthread_unref(sender_thr);
  p_uthread_unref(receiver_thr);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
