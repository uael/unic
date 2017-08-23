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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

ptr_t
pmem_alloc(size_t nbytes) {
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

ptr_t
pmem_realloc(ptr_t block, size_t nbytes) {
  P_UNUSED (block);
  P_UNUSED (nbytes);
  return (ptr_t) NULL;
}

void
pmem_free(ptr_t block) {
  P_UNUSED (block);
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
  sock_addr = p_socketaddr_new("192.168.0.1", 1058);
  ASSERT(sock_addr != NULL);
  native_size = p_socketaddr_get_native_size(sock_addr);
  ASSERT(native_size > 0);
  addr_buf = p_malloc0(native_size);
  ASSERT(addr_buf != NULL);
  ASSERT(p_socketaddr_to_native(sock_addr, addr_buf, native_size - 1) == false);
  ASSERT(p_socketaddr_to_native(sock_addr, addr_buf, native_size) == true);
  p_socketaddr_free(sock_addr);
  if (p_socketaddr_is_ipv6_supported()) {
    sock_addr6 = p_socketaddr_new(
      "2001:cdba:345f:24ab:fe45:5423:3257:9652", 1058
    );
    ASSERT(sock_addr6 != NULL);
    native_size6 = p_socketaddr_get_native_size(sock_addr6);
    ASSERT(native_size6 > 0);
    addr_buf6 = p_malloc0(native_size6);
    ASSERT(addr_buf6 != NULL);
    ASSERT(
      p_socketaddr_to_native(sock_addr6, addr_buf6, native_size6 - 1) == false
    );
    ASSERT(p_socketaddr_to_native(sock_addr6, addr_buf6, native_size6) == true);
    p_socketaddr_free(sock_addr6);
  }
  vtable.free = pmem_free;
  vtable.malloc = pmem_alloc;
  vtable.realloc = pmem_realloc;
  ASSERT(p_mem_set_vtable(&vtable) == true);
  ASSERT(p_socketaddr_new("192.168.0.1", 1058) == NULL);
  ASSERT(p_socketaddr_new_any(P_SOCKET_FAMILY_INET, 1058) == NULL);
  ASSERT(p_socketaddr_new_loopback(P_SOCKET_FAMILY_INET, 1058) == NULL);
  ASSERT(p_socketaddr_new_from_native(addr_buf, native_size) == NULL);
  if (p_socketaddr_is_ipv6_supported())
    ASSERT(p_socketaddr_new_from_native(addr_buf6, native_size6) == NULL);
  p_mem_restore_vtable();
  ASSERT(p_socketaddr_new_from_native(addr_buf, native_size - 1) == NULL);
  if (p_socketaddr_is_ipv6_supported()) {
    ASSERT(p_socketaddr_new_from_native(addr_buf6, native_size6 - 1) == NULL);
    p_free(addr_buf6);
  }
  p_free(addr_buf);
  return CUTE_SUCCESS;
}

CUTEST(socketaddr, bad_input) {
  ASSERT(p_socketaddr_new_from_native(NULL, 0) == NULL);
  ASSERT(p_socketaddr_new(NULL, 0) == NULL);
  ASSERT(p_socketaddr_new("bad_address", 0) == NULL);
  ASSERT(p_socketaddr_new_any(P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
  ASSERT(p_socketaddr_new_loopback(P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
  ASSERT(p_socketaddr_to_native(NULL, NULL, 0) == false);
  ASSERT(p_socketaddr_get_native_size(NULL) == 0);
  ASSERT(p_socketaddr_get_family(NULL) == P_SOCKET_FAMILY_UNKNOWN);
  ASSERT(p_socketaddr_get_address(NULL) == NULL);
  ASSERT(p_socketaddr_get_port(NULL) == 0);
  ASSERT(p_socketaddr_get_flow_info(NULL) == 0);
  ASSERT(p_socketaddr_get_scope_id(NULL) == 0);
  ASSERT(p_socketaddr_is_any(NULL) == false);
  ASSERT(p_socketaddr_is_loopback(NULL) == false);
  p_socketaddr_set_flow_info(NULL, 0);
  p_socketaddr_set_scope_id(NULL, 0);
  p_socketaddr_free(NULL);
  return CUTE_SUCCESS;
}

CUTEST(socketaddr, general) {
  socketaddr_t *addr;
  byte_t *addr_str;
  size_t native_size;
  ptr_t native_buf;

  /* Test IPv4 LAN address */
  addr = p_socketaddr_new("192.168.0.1", 2345);
  ASSERT(addr != NULL);
  ASSERT(p_socketaddr_is_loopback(addr) == false);
  ASSERT(p_socketaddr_is_any(addr) == false);
  ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET);
  ASSERT(p_socketaddr_get_port(addr) == 2345);
  ASSERT(p_socketaddr_get_native_size(addr) > 0);
  ASSERT(p_socketaddr_get_flow_info(addr) == 0);
  ASSERT(p_socketaddr_get_scope_id(addr) == 0);
  addr_str = p_socketaddr_get_address(addr);
  ASSERT(addr_str != NULL);
  ASSERT(strcmp(addr_str, "192.168.0.1") == 0);
  p_free(addr_str);
  p_socketaddr_free(addr);
  if (p_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 LAN address */
    addr = p_socketaddr_new("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
    ASSERT(addr != NULL);
    ASSERT(p_socketaddr_is_loopback(addr) == false);
    ASSERT(p_socketaddr_is_any(addr) == false);
    ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET6);
    ASSERT(p_socketaddr_get_port(addr) == 2345);
    ASSERT(p_socketaddr_get_native_size(addr) > 0);
    ASSERT(p_socketaddr_get_flow_info(addr) == 0);
    ASSERT(p_socketaddr_get_scope_id(addr) == 0);
    addr_str = p_socketaddr_get_address(addr);
    ASSERT(addr_str != NULL);
    ASSERT(
      strcmp(addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);
    p_free(addr_str);
    p_socketaddr_free(addr);
  }

  /* Test IPv4 loopback address */
  addr = p_socketaddr_new_loopback(P_SOCKET_FAMILY_INET, 2345);
  ASSERT(addr != NULL);
  ASSERT(p_socketaddr_is_loopback(addr) == true);
  ASSERT(p_socketaddr_is_any(addr) == false);
  ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET);
  ASSERT(p_socketaddr_get_port(addr) == 2345);
  ASSERT(p_socketaddr_get_native_size(addr) > 0);
  ASSERT(p_socketaddr_get_flow_info(addr) == 0);
  ASSERT(p_socketaddr_get_scope_id(addr) == 0);
  p_socketaddr_free(addr);
  if (p_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 loopback address */
    addr = p_socketaddr_new_loopback(P_SOCKET_FAMILY_INET6, 2345);
    ASSERT(addr != NULL);
    ASSERT(p_socketaddr_is_loopback(addr) == true);
    ASSERT(p_socketaddr_is_any(addr) == false);
    ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET6);
    ASSERT(p_socketaddr_get_port(addr) == 2345);
    ASSERT(p_socketaddr_get_native_size(addr) > 0);
    ASSERT(p_socketaddr_get_flow_info(addr) == 0);
    ASSERT(p_socketaddr_get_scope_id(addr) == 0);
    p_socketaddr_free(addr);
  }

  /* Test IPv4 any interface */
  addr = p_socketaddr_new_any(P_SOCKET_FAMILY_INET, 2345);
  ASSERT(addr != NULL);
  ASSERT(p_socketaddr_is_loopback(addr) == false);
  ASSERT(p_socketaddr_is_any(addr) == true);
  ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET);
  ASSERT(p_socketaddr_get_port(addr) == 2345);
  ASSERT(p_socketaddr_get_native_size(addr) > 0);
  ASSERT(p_socketaddr_get_flow_info(addr) == 0);
  ASSERT(p_socketaddr_get_scope_id(addr) == 0);
  native_size = p_socketaddr_get_native_size(addr);
  p_socketaddr_free(addr);

  /* Test IPv4 native raw data */
  native_buf = p_malloc0(native_size);
  ASSERT(native_buf != NULL);
  ASSERT(p_socketaddr_new_from_native(native_buf, native_size) == NULL);
  addr = p_socketaddr_new("192.168.0.2", 2345);
  ASSERT(addr != NULL);
  p_socketaddr_set_flow_info(addr, 1);
  p_socketaddr_set_scope_id(addr, 1);
  ASSERT(p_socketaddr_to_native(addr, native_buf, native_size) == true);
  p_socketaddr_free(addr);
  addr = p_socketaddr_new_from_native(native_buf, native_size);
  ASSERT(addr != NULL);
  ASSERT(p_socketaddr_is_loopback(addr) == false);
  ASSERT(p_socketaddr_is_any(addr) == false);
  ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET);
  ASSERT(p_socketaddr_get_port(addr) == 2345);
  ASSERT(p_socketaddr_get_native_size(addr) == native_size);
  ASSERT(p_socketaddr_get_flow_info(addr) == 0);
  ASSERT(p_socketaddr_get_scope_id(addr) == 0);
  addr_str = p_socketaddr_get_address(addr);
  ASSERT(addr_str != NULL);
  ASSERT(strcmp(addr_str, "192.168.0.2") == 0);
  p_free(native_buf);
  p_free(addr_str);
  p_socketaddr_free(addr);
  if (p_socketaddr_is_ipv6_supported()) {
    /* Test IPv6 any interface */
    addr = p_socketaddr_new_any(P_SOCKET_FAMILY_INET6, 2345);
    ASSERT(addr != NULL);
    ASSERT(p_socketaddr_is_loopback(addr) == false);
    ASSERT(p_socketaddr_is_any(addr) == true);
    ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET6);
    ASSERT(p_socketaddr_get_port(addr) == 2345);
    ASSERT(p_socketaddr_get_native_size(addr) > 0);
    ASSERT(p_socketaddr_get_flow_info(addr) == 0);
    ASSERT(p_socketaddr_get_scope_id(addr) == 0);
    native_size = p_socketaddr_get_native_size(addr);
    p_socketaddr_free(addr);

    /* Test IPv6 native raw data */
    native_buf = p_malloc0(native_size);
    ASSERT(native_buf != NULL);
    ASSERT(p_socketaddr_new_from_native(native_buf, native_size) == NULL);
    addr = p_socketaddr_new("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
    ASSERT(addr != NULL);
    p_socketaddr_set_flow_info(addr, 1);
    p_socketaddr_set_scope_id(addr, 1);
    ASSERT(p_socketaddr_to_native(addr, native_buf, native_size) == true);
    p_socketaddr_free(addr);
    addr = p_socketaddr_new_from_native(native_buf, native_size);
    ASSERT(addr != NULL);
    ASSERT(p_socketaddr_is_loopback(addr) == false);
    ASSERT(p_socketaddr_is_any(addr) == false);
    ASSERT(p_socketaddr_get_family(addr) == P_SOCKET_FAMILY_INET6);
    ASSERT(p_socketaddr_get_port(addr) == 2345);
    ASSERT(p_socketaddr_get_native_size(addr) == native_size);
    if (p_socketaddr_is_flow_info_supported())
      ASSERT(p_socketaddr_get_flow_info(addr) == 1);
    if (p_socketaddr_is_scope_id_supported())
      ASSERT(p_socketaddr_get_scope_id(addr) == 1);
    addr_str = p_socketaddr_get_address(addr);
    ASSERT(addr_str != NULL);
    ASSERT(strcmp(addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);
    p_free(native_buf);
    p_free(addr_str);
    p_socketaddr_free(addr);
  }
  if (p_socketaddr_is_flow_info_supported()
    || p_socketaddr_is_scope_id_supported())
    ASSERT(p_socketaddr_is_ipv6_supported() == true);
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
