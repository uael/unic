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

#define BOOST_TEST_MODULE psocketaddress_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

extern "C" ptr_t pmem_alloc (size_t nbytes)
{
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" ptr_t pmem_realloc (ptr_t block, size_t nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" void pmem_free (ptr_t block)
{
	P_UNUSED (block);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psocketaddress_nomem_test)
{
	p_libsys_init ();

	socketaddr_t *sock_addr = p_socketaddr_new ("192.168.0.1", 1058);
	BOOST_CHECK (sock_addr != NULL);

	size_t native_size = p_socketaddr_get_native_size (sock_addr);
	BOOST_CHECK (native_size > 0);

	ptr_t addr_buf = p_malloc0 (native_size);
	BOOST_CHECK (addr_buf != NULL);

	BOOST_CHECK (p_socketaddr_to_native (sock_addr, addr_buf, native_size - 1) == false);
	BOOST_CHECK (p_socketaddr_to_native (sock_addr, addr_buf, native_size) == true);
	p_socketaddr_free (sock_addr);

	socketaddr_t *sock_addr6;
	size_t native_size6;
	ptr_t addr_buf6;

	if (p_socketaddr_is_ipv6_supported ()) {
		sock_addr6 = p_socketaddr_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 1058);
		BOOST_CHECK (sock_addr6 != NULL);

		native_size6 = p_socketaddr_get_native_size (sock_addr6);
		BOOST_CHECK (native_size6 > 0);

		addr_buf6 = p_malloc0 (native_size6);
		BOOST_CHECK (addr_buf6 != NULL);

		BOOST_CHECK (p_socketaddr_to_native (sock_addr6, addr_buf6, native_size6 - 1) == false);
		BOOST_CHECK (p_socketaddr_to_native (sock_addr6, addr_buf6, native_size6) == true);
		p_socketaddr_free (sock_addr6);
	}

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_socketaddr_new ("192.168.0.1", 1058) == NULL);
	BOOST_CHECK (p_socketaddr_new_any (P_SOCKET_FAMILY_INET, 1058) == NULL);
	BOOST_CHECK (p_socketaddr_new_loopback (P_SOCKET_FAMILY_INET, 1058) == NULL);
	BOOST_CHECK (p_socketaddr_new_from_native (addr_buf, native_size) == NULL);

	if (p_socketaddr_is_ipv6_supported ())
		BOOST_CHECK (p_socketaddr_new_from_native (addr_buf6, native_size6) == NULL);

	p_mem_restore_vtable ();

	BOOST_CHECK (p_socketaddr_new_from_native (addr_buf, native_size - 1) == NULL);

	if (p_socketaddr_is_ipv6_supported ()) {
		BOOST_CHECK (p_socketaddr_new_from_native (addr_buf6, native_size6 - 1) == NULL);
		p_free (addr_buf6);
	}

	p_free (addr_buf);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocketaddress_bad_input_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_socketaddr_new_from_native (NULL, 0) == NULL);
	BOOST_CHECK (p_socketaddr_new (NULL, 0) == NULL);
	BOOST_CHECK (p_socketaddr_new ("bad_address", 0) == NULL);
	BOOST_CHECK (p_socketaddr_new_any (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	BOOST_CHECK (p_socketaddr_new_loopback (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	BOOST_CHECK (p_socketaddr_to_native (NULL, NULL, 0) == false);
	BOOST_CHECK (p_socketaddr_get_native_size (NULL) == 0);
	BOOST_CHECK (p_socketaddr_get_family (NULL) == P_SOCKET_FAMILY_UNKNOWN);
	BOOST_CHECK (p_socketaddr_get_address (NULL) == NULL);
	BOOST_CHECK (p_socketaddr_get_port (NULL) == 0);
	BOOST_CHECK (p_socketaddr_get_flow_info (NULL) == 0);
	BOOST_CHECK (p_socketaddr_get_scope_id (NULL) == 0);
	BOOST_CHECK (p_socketaddr_is_any (NULL) == false);
	BOOST_CHECK (p_socketaddr_is_loopback (NULL) == false);

	p_socketaddr_set_flow_info (NULL, 0);
	p_socketaddr_set_scope_id (NULL, 0);
	p_socketaddr_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocketaddress_general_test)
{
	p_libsys_init ();

	/* Test IPv4 LAN address */
	socketaddr_t *addr = p_socketaddr_new ("192.168.0.1", 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
	BOOST_CHECK (p_socketaddr_is_any (addr) == false);
	BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
	BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
	BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
	BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

	byte_t *addr_str = p_socketaddr_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.1") == 0);

	p_free (addr_str);
	p_socketaddr_free (addr);

	if (p_socketaddr_is_ipv6_supported ()) {
		/* Test IPv6 LAN address */
		addr = p_socketaddr_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);

		BOOST_REQUIRE (addr != NULL);
		BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
		BOOST_CHECK (p_socketaddr_is_any (addr) == false);
		BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET6);
		BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
		BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
		BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
		BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

		addr_str = p_socketaddr_get_address (addr);

		BOOST_REQUIRE (addr_str != NULL);
		BOOST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

		p_free (addr_str);
		p_socketaddr_free (addr);
	}

	/* Test IPv4 loopback address */
	addr = p_socketaddr_new_loopback (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socketaddr_is_loopback (addr) == true);
	BOOST_CHECK (p_socketaddr_is_any (addr) == false);
	BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
	BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
	BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
	BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

	p_socketaddr_free (addr);

	if (p_socketaddr_is_ipv6_supported ()) {
		/* Test IPv6 loopback address */
		addr = p_socketaddr_new_loopback (P_SOCKET_FAMILY_INET6, 2345);

		BOOST_REQUIRE (addr != NULL);
		BOOST_CHECK (p_socketaddr_is_loopback (addr) == true);
		BOOST_CHECK (p_socketaddr_is_any (addr) == false);
		BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET6);
		BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
		BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
		BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
		BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

		p_socketaddr_free (addr);
	}

	/* Test IPv4 any interface */
	addr = p_socketaddr_new_any (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
	BOOST_CHECK (p_socketaddr_is_any (addr) == true);
	BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
	BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
	BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
	BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

	size_t native_size = p_socketaddr_get_native_size (addr);

	p_socketaddr_free (addr);

	/* Test IPv4 native raw data */
	ptr_t native_buf = p_malloc0 (native_size);
	BOOST_CHECK (native_buf != NULL);
	BOOST_CHECK (p_socketaddr_new_from_native (native_buf, native_size) == NULL);
	addr = p_socketaddr_new ("192.168.0.2", 2345);
	BOOST_REQUIRE (addr != NULL);

	p_socketaddr_set_flow_info (addr, 1);
	p_socketaddr_set_scope_id (addr, 1);

	BOOST_CHECK (p_socketaddr_to_native (addr, native_buf, native_size) == true);
	p_socketaddr_free (addr);

	addr = p_socketaddr_new_from_native (native_buf, native_size);

	BOOST_CHECK (addr != NULL);
	BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
	BOOST_CHECK (p_socketaddr_is_any (addr) == false);
	BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
	BOOST_CHECK (p_socketaddr_get_native_size (addr) == native_size);
	BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
	BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

	addr_str = p_socketaddr_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.2") == 0);

	p_free (native_buf);
	p_free (addr_str);
	p_socketaddr_free (addr);

	if (p_socketaddr_is_ipv6_supported ()) {
		/* Test IPv6 any interface */
		addr = p_socketaddr_new_any (P_SOCKET_FAMILY_INET6, 2345);

		BOOST_REQUIRE (addr != NULL);
		BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
		BOOST_CHECK (p_socketaddr_is_any (addr) == true);
		BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET6);
		BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
		BOOST_CHECK (p_socketaddr_get_native_size (addr) > 0);
		BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 0);
		BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 0);

		native_size = p_socketaddr_get_native_size (addr);

		p_socketaddr_free (addr);

		/* Test IPv6 native raw data */
		native_buf = p_malloc0 (native_size);
		BOOST_CHECK (native_buf != NULL);
		BOOST_CHECK (p_socketaddr_new_from_native (native_buf, native_size) == NULL);
		addr = p_socketaddr_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
		BOOST_REQUIRE (addr != NULL);

		p_socketaddr_set_flow_info (addr, 1);
		p_socketaddr_set_scope_id (addr, 1);

		BOOST_CHECK (p_socketaddr_to_native (addr, native_buf, native_size) == true);
		p_socketaddr_free (addr);

		addr = p_socketaddr_new_from_native (native_buf, native_size);

		BOOST_CHECK (addr != NULL);
		BOOST_CHECK (p_socketaddr_is_loopback (addr) == false);
		BOOST_CHECK (p_socketaddr_is_any (addr) == false);
		BOOST_CHECK (p_socketaddr_get_family (addr) == P_SOCKET_FAMILY_INET6);
		BOOST_CHECK (p_socketaddr_get_port (addr) == 2345);
		BOOST_CHECK (p_socketaddr_get_native_size (addr) == native_size);

		if (p_socketaddr_is_flow_info_supported ())
			BOOST_CHECK (p_socketaddr_get_flow_info (addr) == 1);

		if (p_socketaddr_is_scope_id_supported ())
			BOOST_CHECK (p_socketaddr_get_scope_id (addr) == 1);

		addr_str = p_socketaddr_get_address (addr);

		BOOST_REQUIRE (addr_str != NULL);
		BOOST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

		p_free (native_buf);
		p_free (addr_str);
		p_socketaddr_free (addr);
	}

	if (p_socketaddr_is_flow_info_supported () || p_socketaddr_is_scope_id_supported ())
		BOOST_CHECK (p_socketaddr_is_ipv6_supported () == true);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
