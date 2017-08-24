/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file unic/socketaddr.h
 * @brief Socket address wrapper
 * @author Alexander Saprykin
 *
 * A socket address is usually represented by a network address (IPv4 or IPv6)
 * and a port number (though some other naming schemes and parameters are
 * possible).
 *
 * Socket address parameters are stored inside a special system (native)
 * structure in the binary (raw) form. The native structure varies with an
 * operating system and a network protocol. #socketaddr_t acts like a thin
 * wrapper around that native address structure and unifies manipulation of
 * socket address data.
 *
 * #socketaddr_t supports IPv4 and IPv6 addresses which consist of an IP
 * address and a port number. IPv6 support is system dependent and not provided
 * for all the platforms. Sometimes you may also need to enable IPv6 support in
 * the system to make it working.
 *
 * Convenient methods to create special addresses are provided: for the loopback
 * interface use u_socketaddr_new_loopback(), for the any-address interface
 * use u_socketaddr_new_any().
 *
 * If you want to get the underlying native address structure for further usage
 * in system calls use u_socketaddr_to_native(), and
 * u_socketaddr_new_from_native() for a vice versa conversion.
 */
#ifndef U_SOCKETADDR_H__
# define U_SOCKETADDR_H__

#include "unic/macros.h"
#include "unic/types.h"

#ifndef U_OS_WIN
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif

/*!@brief Socket address opaque structure. */
typedef struct socketaddr socketaddr_t;

/*!@brief Socket address family. */
enum socket_family {

  /*!@brief Unknown family. */
  U_SOCKET_FAMILY_UNKNOWN = 0,

  /*!@brief IPv4 family. */
  U_SOCKET_FAMILY_INET = AF_INET,

#ifdef AF_INET6
  /*!@brief IPv6 family. */
  U_SOCKET_FAMILY_INET6 = AF_INET6
#else
  /*!@brief No IPv6 family. */
  U_SOCKET_FAMILY_INET6 = -1
#endif
};

typedef enum socket_family socket_family_t;

/*!@brief Creates new #socketaddr_t from the native socket address raw data.
 * @param native Pointer to the native socket address raw data.
 * @param len Raw data length, in bytes.
 * @return Pointer to #socketaddr_t in case of success, NULL otherwise.
 * @since 0.0.1
 */
U_API socketaddr_t *
u_socketaddr_new_from_native(const_ptr_t native, size_t len);

/*!@brief Creates new #socketaddr_t.
 * @param address String representation of an address (i.e. "172.146.45.5").
 * @param port Port number.
 * @return Pointer to #socketaddr_t in case of success, NULL otherwise.
 * @since 0.0.1
 * @note It tries to automatically detect a socket family.
 *
 * If the @a address is an IPv6 address, it can also contain a scope index
 * separated from the address by the '%' literal). Most target platforms should
 * correctly parse such an address though some old operating systems may fail in
 * case of lack of the getaddrinfo() call.
 */
U_API socketaddr_t *
u_socketaddr_new(const byte_t *address, u16_t port);

/*!@brief Creates new #socketaddr_t for the any-address representation.
 * @param family Socket family.
 * @param port Port number.
 * @return Pointer to #socketaddr_t in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates a network address for the set of all possible
 * addresses, so you can't use it for receiving or sending data on a particular
 * network address. If you need to bind a socket to the specific address
 * (i.e. 127.0.0.1) use u_socketaddr_new() instead.
 */
U_API socketaddr_t *
u_socketaddr_new_any(socket_family_t family, u16_t port);

/*!@brief Creates new #socketaddr_t for the loopback interface.
 * @param family Socket family.
 * @param port Port number.
 * @return Pointer to #socketaddr_t in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates a network address for the entire loopback network
 * interface, so you can't use it for receiving or sending data on a particular
 * network address. If you need to bind a socket to the specific address
 * (i.e. 127.0.0.1) use u_socketaddr_new() instead.
 */
U_API socketaddr_t *
u_socketaddr_new_loopback(socket_family_t family, u16_t port);

/*!@brief Converts #socketaddr_t to the native socket address raw data.
 * @param addr #socketaddr_t to convert.
 * @param[out] dest Output buffer for raw data.
 * @param destlen Length in bytes of the @a dest buffer.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_socketaddr_to_native(const socketaddr_t *addr, ptr_t dest,
  size_t destlen);

/*!@brief Gets the size of the native socket address raw data, in bytes.
 * @param addr #socketaddr_t to get the size of native address raw data for.
 * @return Size of the native socket address raw data in case of success, 0
 * otherwise.
 * @since 0.0.1
 */
U_API size_t
u_socketaddr_get_native_size(const socketaddr_t *addr);

/*!@brief Gets a family of a socket address.
 * @param addr #socketaddr_t to get the family for.
 * @return #socket_family_t of the socket address.
 * @since 0.0.1
 */
U_API socket_family_t
u_socketaddr_get_family(const socketaddr_t *addr);

/*!@brief Gets a socket address in a string representation, i.e. "172.146.45.5".
 * @param addr #socketaddr_t to get address string for.
 * @return Pointer to the string representation of the socket address in case of
 * success, NULL otherwise. The caller takes ownership of the returned pointer.
 * @since 0.0.1
 */
U_API byte_t *
u_socketaddr_get_address(const socketaddr_t *addr);

/*!@brief Gets a port number of a socket address.
 * @param addr #socketaddr_t to get the port number for.
 * @return Port number in case of success, 0 otherwise.
 * @since 0.0.1
 */
U_API u16_t
u_socketaddr_get_port(const socketaddr_t *addr);

/*!@brief Gets IPv6 traffic class and flow information.
 * @param addr #socketaddr_t to get flow information for.
 * @return IPv6 traffic class and flow information.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address, otherwise 0 is returned.
 * @note Some operating systems may not support this property.
 * @sa u_socketaddr_is_flow_info_supported()
 */
U_API u32_t
u_socketaddr_get_flow_info(const socketaddr_t *addr);

/*!@brief Gets an IPv6 set of interfaces for a scope.
 * @param addr #socketaddr_t to get the set of interfaces for.
 * @return Index that identifies the set of interfaces for a scope.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address, otherwise 0 is returned.
 * @note Some operating systems may not support this property.
 * @sa u_socketaddr_is_scope_id_supported()
 */
U_API u32_t
u_socketaddr_get_scope_id(const socketaddr_t *addr);

/*!@brief Sets IPv6 traffic class and flow information.
 * @param addr #socketaddr_t to set flow information for.
 * @param flowinfo Flow information to set.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address.
 * @note Some operating systems may not support this property.
 * @sa u_socketaddr_is_flow_info_supported()
 */
U_API void
u_socketaddr_set_flow_info(socketaddr_t *addr, u32_t flowinfo);

/*!@brief Sets an IPv6 set of interfaces for a scope.
 * @param addr #socketaddr_t to set the set of interfaces for.
 * @param scope_id Index that identifies the set of interfaces for a scope.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address.
 * @note Some operating systems may not support this property.
 * @sa u_socketaddr_is_scope_id_supported()
 */
U_API void
u_socketaddr_set_scope_id(socketaddr_t *addr, u32_t scope_id);

/*!@brief Checks whether flow information is supported in IPv6.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_socketaddr_is_flow_info_supported(void);

/*!@brief Checks whether a set of interfaces for a scope is supported in IPv6.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_socketaddr_is_scope_id_supported(void);

/*!@brief Checks whether IPv6 protocol is supported.
 * @return true in case of success, false otherwise.
 * @since 0.0.3
 */
U_API bool
u_socketaddr_is_ipv6_supported(void);

/*!@brief Checks whether a given socket address is an any-address
 * representation. Such an address is a 0.0.0.0.
 * @param addr #socketaddr_t to check.
 * @return true if the @a addr is the any-address representation, false
 * otherwise.
 * @since 0.0.1
 * @sa u_socketaddr_new_any()
 */
U_API bool
u_socketaddr_is_any(const socketaddr_t *addr);

/*!@brief Checks whether a given socket address is for the loopback interface.
 * Such an address is a 127.x.x.x.
 * @param addr #socketaddr_t to check.
 * @return true if the @a addr is for the loopback interface, false otherwise.
 * @since 0.0.1
 * @sa u_socketaddr_new_loopback()
 */
U_API bool
u_socketaddr_is_loopback(const socketaddr_t *addr);

/*!@brief Frees a socket address structure and its resources.
 * @param addr #socketaddr_t to free.
 * @since 0.0.1
 */
U_API void
u_socketaddr_free(socketaddr_t *addr);

#endif /* !U_SOCKETADDR_H__ */
