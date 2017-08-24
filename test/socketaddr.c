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

#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

ptr_t
pmem_alloc(size_t nbytes) {
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  U_UNUSED (block);
  U_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  U_UNUSED (block);
}

CUTEST(socketaddr, nomem) {
  socketaddr_t *sock_addr;
  size_t native_size;
  ptr_t addr_buf;
  socketaddr_t *sock_addr6;
  size_t native_size6;
  ptr_t addr_buf6;
  mem_vtable_t vtable;

  addr_buf6 = NULL;
  native_size6 = 0;
  sock_addr = u_socketaddr_new("192.168.0.1", 1058);
  ASSERT(sock_addr != NULL);
  native_size = u_socketaddr_get_native_size(sock_addr);
  ASSERT(native_size > 0);
  addr_buf = u_malloc0(native_size);
  ASSERT(addr_buf != NULL);
  ASSERT(u_socketaddr_to_native(sock_addr, addr_buf, native_size - 1) == false);
  ASSERT(u_socketaddr_to_native(sock_addr, addr_buf, native_size) == true);
  u_socketaddr_free(sock_addr);
  if (u_socketaddr_is_ipv6_supported()) {
    sock_addr6 = u_socketaddr_new(
      "2001:cdba:345f:24ab:fe45:5423:3257:9652", 1058
    );
    ASSERT(sock_addr6 != NULL);
    native_size6 = u_socketaddr_get_native_size(sock_addr6);
    ASSERT(native_size6 > 0);
    addr_buf6 = u_malloc0(native_size6);
    ASSERT(addr_buf6 != NULL);
    ASSERT(
      u_socketaddr_to_native(sock_addr6, addr_buf6, native_size6 - 1) == false
    );
    ASSERT(u_socketaddr_to_native(sock_addr6, addr_buf6, native_size6) == true);
    u_socketaddr_free(sock_addr6);
  }
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(u_mem_set_vtable(&vtable) == true);
  ASSERT(u_socketaddr_new("192.168.0.1", 1058) == NULL);
  ASSERT(u_socketaddr_new_any(U_SOCKET_FAMILY_INET, 1058) == NULL);
  ASSERT(u_socketaddr_new_loopback(U_SOCKET_FAMILY_INET, 1058) == NULL);
  ASSERT(u_socketaddr_new_from_native(addr_buf, native_size) == NULL);
  if (u_socketaddr_is_ipv6_supported())
    ASSERT(u_socketaddr_new_from_native(addr_buf6, native_size6) == NULL);
  u_mem_restore_vtable();
  ASSERT(u_socketaddr_new_from_native(addr_buf, native_size - 1) == NULL);
  if (u_socketaddr_is_ipv6_supported()) {
    ASSERT(u_socketaddr_new_from_native(addr_buf6, native_size6 - 1) == NULL);
    u_free(addr_buf6);
  }
  u_free(addr_buf);
  return CUTE_SUCCESS;
}

CUTEST(socketaddr, bad_input) {
  ASSERT(u_socketaddr_new_from_native(NULL, 0) == NULL);
  ASSERT(u_socketaddr_new(NULL, 0) == NULL);
  ASSERT(u_socketaddr_new("bad_address", 0) == NULL);
  ASSERT(u_socketaddr_new_any(U_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
  ASSERT(u_socketaddr_new_loopback(U_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
  ASSERT(u_socketaddr_to_native(NULL, NULL, 0) == false);
  ASSERT(u_socketaddr_get_native_size(NULL) == 0);
  ASSERT(u_socketaddr_get_family(NULL) == U_SOCKET_FAMILY_UNKNOWN);
  ASSERT(u_socketaddr_get_address(NULL) == NULL);
  ASSERT(u_socketaddr_get_port(NULL) == 0);
  ASSERT(u_socketaddr_get_flow_info(NULL) == 0);
  ASSERT(u_socketaddr_get_scope_id(NULL) == 0);
  ASSERT(u_socketaddr_is_any(NULL) == false);
  ASSERT(u_socketaddr_is_loopback(NULL) == false);
  u_socketaddr_set_flow_info(NULL, 0);
  u_socketaddr_set_scope_id(NULL, 0);
  u_socketaddr_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(socketaddr, general) {
  socketaddr_t *addr;
  byte_t *addr_str;
  size_t native_size;
  ptr_t native_buf;

  /* Test IPv4 LAN address */
  addr = u_socketaddr_new("192.168.0.1", 2345);
  ASSERT(addr != NULL);
  ASSERT(u_socketaddr_is_loopback(addr) == false);
  ASSERT(u_socketaddr_is_any(addr) == false);
  ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET);
  ASSERT(u_socketaddr_get_port(addr) == 2345);
  ASSERT(u_socketaddr_get_native_size(addr) > 0);
  ASSERT(u_socketaddr_get_flow_info(addr) == 0);
  ASSERT(u_socketaddr_get_scope_id(addr) == 0);
  addr_str = u_socketaddr_get_address(addr);
  ASSERT(addr_str != NULL);
  ASSERT(strcmp(addr_str, "192.168.0.1") == 0);
  u_free(addr_str);
  u_socketaddr_free(addr);
  if (u_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 LAN address */
    addr = u_socketaddr_new("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
    ASSERT(addr != NULL);
    ASSERT(u_socketaddr_is_loopback(addr) == false);
    ASSERT(u_socketaddr_is_any(addr) == false);
    ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET6);
    ASSERT(u_socketaddr_get_port(addr) == 2345);
    ASSERT(u_socketaddr_get_native_size(addr) > 0);
    ASSERT(u_socketaddr_get_flow_info(addr) == 0);
    ASSERT(u_socketaddr_get_scope_id(addr) == 0);
    addr_str = u_socketaddr_get_address(addr);
    ASSERT(addr_str != NULL);
    ASSERT(
      strcmp(addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);
    u_free(addr_str);
    u_socketaddr_free(addr);
  }

  /* Test IPv4 loopback address */
  addr = u_socketaddr_new_loopback(U_SOCKET_FAMILY_INET, 2345);
  ASSERT(addr != NULL);
  ASSERT(u_socketaddr_is_loopback(addr) == true);
  ASSERT(u_socketaddr_is_any(addr) == false);
  ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET);
  ASSERT(u_socketaddr_get_port(addr) == 2345);
  ASSERT(u_socketaddr_get_native_size(addr) > 0);
  ASSERT(u_socketaddr_get_flow_info(addr) == 0);
  ASSERT(u_socketaddr_get_scope_id(addr) == 0);
  u_socketaddr_free(addr);
  if (u_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 loopback address */
    addr = u_socketaddr_new_loopback(U_SOCKET_FAMILY_INET6, 2345);
    ASSERT(addr != NULL);
    ASSERT(u_socketaddr_is_loopback(addr) == true);
    ASSERT(u_socketaddr_is_any(addr) == false);
    ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET6);
    ASSERT(u_socketaddr_get_port(addr) == 2345);
    ASSERT(u_socketaddr_get_native_size(addr) > 0);
    ASSERT(u_socketaddr_get_flow_info(addr) == 0);
    ASSERT(u_socketaddr_get_scope_id(addr) == 0);
    u_socketaddr_free(addr);
  }

  /* Test IPv4 any interface */
  addr = u_socketaddr_new_any(U_SOCKET_FAMILY_INET, 2345);
  ASSERT(addr != NULL);
  ASSERT(u_socketaddr_is_loopback(addr) == false);
  ASSERT(u_socketaddr_is_any(addr) == true);
  ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET);
  ASSERT(u_socketaddr_get_port(addr) == 2345);
  ASSERT(u_socketaddr_get_native_size(addr) > 0);
  ASSERT(u_socketaddr_get_flow_info(addr) == 0);
  ASSERT(u_socketaddr_get_scope_id(addr) == 0);
  native_size = u_socketaddr_get_native_size(addr);
  u_socketaddr_free(addr);

  /* Test IPv4 native raw data */
  native_buf = u_malloc0(native_size);
  ASSERT(native_buf != NULL);
  ASSERT(u_socketaddr_new_from_native(native_buf, native_size) == NULL);
  addr = u_socketaddr_new("192.168.0.2", 2345);
  ASSERT(addr != NULL);
  u_socketaddr_set_flow_info(addr, 1);
  u_socketaddr_set_scope_id(addr, 1);
  ASSERT(u_socketaddr_to_native(addr, native_buf, native_size) == true);
  u_socketaddr_free(addr);
  addr = u_socketaddr_new_from_native(native_buf, native_size);
  ASSERT(addr != NULL);
  ASSERT(u_socketaddr_is_loopback(addr) == false);
  ASSERT(u_socketaddr_is_any(addr) == false);
  ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET);
  ASSERT(u_socketaddr_get_port(addr) == 2345);
  ASSERT(u_socketaddr_get_native_size(addr) == native_size);
  ASSERT(u_socketaddr_get_flow_info(addr) == 0);
  ASSERT(u_socketaddr_get_scope_id(addr) == 0);
  addr_str = u_socketaddr_get_address(addr);
  ASSERT(addr_str != NULL);
  ASSERT(strcmp(addr_str, "192.168.0.2") == 0);
  u_free(native_buf);
  u_free(addr_str);
  u_socketaddr_free(addr);
  if (u_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 any interface */
    addr = u_socketaddr_new_any(U_SOCKET_FAMILY_INET6, 2345);
    ASSERT(addr != NULL);
    ASSERT(u_socketaddr_is_loopback(addr) == false);
    ASSERT(u_socketaddr_is_any(addr) == true);
    ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET6);
    ASSERT(u_socketaddr_get_port(addr) == 2345);
    ASSERT(u_socketaddr_get_native_size(addr) > 0);
    ASSERT(u_socketaddr_get_flow_info(addr) == 0);
    ASSERT(u_socketaddr_get_scope_id(addr) == 0);
    native_size = u_socketaddr_get_native_size(addr);
    u_socketaddr_free(addr);

    /* Test IPv6 native raw data */
    native_buf = u_malloc0(native_size);
    ASSERT(native_buf != NULL);
    ASSERT(u_socketaddr_new_from_native(native_buf, native_size) == NULL);
    addr = u_socketaddr_new("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
    ASSERT(addr != NULL);
    u_socketaddr_set_flow_info(addr, 1);
    u_socketaddr_set_scope_id(addr, 1);
    ASSERT(u_socketaddr_to_native(addr, native_buf, native_size) == true);
    u_socketaddr_free(addr);
    addr = u_socketaddr_new_from_native(native_buf, native_size);
    ASSERT(addr != NULL);
    ASSERT(u_socketaddr_is_loopback(addr) == false);
    ASSERT(u_socketaddr_is_any(addr) == false);
    ASSERT(u_socketaddr_get_family(addr) == U_SOCKET_FAMILY_INET6);
    ASSERT(u_socketaddr_get_port(addr) == 2345);
    ASSERT(u_socketaddr_get_native_size(addr) == native_size);
    if (u_socketaddr_is_flow_info_supported())
      ASSERT(u_socketaddr_get_flow_info(addr) == 1);
    if (u_socketaddr_is_scope_id_supported())
      ASSERT(u_socketaddr_get_scope_id(addr) == 1);
    addr_str = u_socketaddr_get_address(addr);
    ASSERT(addr_str != NULL);
    ASSERT(strcmp(addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);
    u_free(native_buf);
    u_free(addr_str);
    u_socketaddr_free(addr);
  }
  if (u_socketaddr_is_flow_info_supported()
    || u_socketaddr_is_scope_id_supported())
    ASSERT(u_socketaddr_is_ipv6_supported() == true);
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(socketaddr, nomem);
  CUTEST_PASS(socketaddr, bad_input);
  CUTEST_PASS(socketaddr, general);
  return EXIT_SUCCESS;
}
