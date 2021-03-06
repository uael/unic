/*
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

#include "unic/mem.h"
#include "unic/string.h"
#include "unic/socketaddr.h"

#ifndef U_OS_WIN
# include <arpa/inet.h>
#endif

#if defined(UNIC_HAS_GETADDRINFO) && \
    !defined(UNIC_SOCKADDR_IN6_HAS_SCOPEID)
# undef UNIC_HAS_GETADDRINFO
#endif

#ifdef UNIC_HAS_GETADDRINFO
# include <netdb.h>
#endif

/* According to Open Group specifications */
#ifndef INET_ADDRSTRLEN
# ifdef U_OS_WIN
/* On Windows it includes port number  */
#   define INET_ADDRSTRLEN 22
# else
#   define INET_ADDRSTRLEN 16
# endif
#endif

#ifdef AF_INET6
# ifndef INET6_ADDRSTRLEN
#   ifdef U_OS_WIN
/* On Windows it includes port number */
#   define INET6_ADDRSTRLEN 65
#   else
#   define INET6_ADDRSTRLEN 46
#   endif
# endif
#endif

#ifdef U_OS_VMS
# if UNIC_SIZEOF_VOID_P == 8
#   define addrinfo __addrinfo64
# endif
#endif

#if defined (U_OS_BEOS) || defined (U_OS_OS2)
# ifdef AF_INET6
#   undef AF_INET6
# endif
#endif

struct socketaddr {
  socket_family_t family;
  union addr_ {
    struct in_addr sin_addr;
#ifdef AF_INET6
    struct in6_addr sin6_addr;
#endif
  } addr;
  u16_t port;
  u32_t flowinfo;
  u32_t scope_id;
};

socketaddr_t *
u_socketaddr_new_from_native(const_ptr_t native, size_t len) {
  socketaddr_t *ret;
  u16_t family;

  if (U_UNLIKELY (native == NULL || len == 0)) {
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(socketaddr_t))) == NULL)) {
    return NULL;
  }
  family = ((struct sockaddr *) native)->sa_family;
  if (family == AF_INET) {
    if (len < sizeof(struct sockaddr_in)) {
      U_WARNING (
        "socketaddr_t::u_socketaddr_new_from_native: invalid IPv4 native size");
      u_free(ret);
      return NULL;
    }
    memcpy(
      &ret->addr.sin_addr, &((struct sockaddr_in *) native)->sin_addr,
      sizeof(struct in_addr));
    ret->family = U_SOCKET_FAMILY_INET;
    ret->port = u_ntohs (((struct sockaddr_in *) native)->sin_port);
    return ret;
  }
#ifdef AF_INET6
  else if (family == AF_INET6) {
    if (len < sizeof(struct sockaddr_in6)) {
      U_WARNING (
        "socketaddr_t::u_socketaddr_new_from_native: invalid IPv6 native size");
      u_free(ret);
      return NULL;
    }
    memcpy(
      &ret->addr.sin6_addr,
      &((struct sockaddr_in6 *) native)->sin6_addr,
      sizeof(struct in6_addr));
    ret->family = U_SOCKET_FAMILY_INET6;
    ret->port = u_ntohs (((struct sockaddr_in *) native)->sin_port);
#ifdef UNIC_SOCKADDR_IN6_HAS_FLOWINFO
    ret->flowinfo = ((struct sockaddr_in6 *) native)->sin6_flowinfo;
#endif
#ifdef UNIC_SOCKADDR_IN6_HAS_SCOPEID
    ret->scope_id = ((struct sockaddr_in6 *) native)->sin6_scope_id;
#endif
    return ret;
  }
#endif
  else {
    u_free(ret);
    return NULL;
  }
}

socketaddr_t *
u_socketaddr_new(const byte_t *address, u16_t port) {
  socketaddr_t *ret;
#if defined (U_OS_WIN) || defined (UNIC_HAS_GETADDRINFO)
  struct addrinfo hints;
  struct addrinfo *res;
#endif
#ifdef U_OS_WIN
  struct sockaddr_storage sa;
  struct sockaddr_in *sin = (struct sockaddr_in *) &sa;
# ifdef AF_INET6
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &sa;
# endif /* AF_INET6 */
  int len;
#endif /* U_OS_WIN */

  if (U_UNLIKELY (address == NULL)) {
    return NULL;
  }
#if (defined(U_OS_WIN) || defined(UNIC_HAS_GETADDRINFO)) && defined(AF_INET6)
  if (strchr(address, ':') != NULL) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
# ifndef U_OS_UNIXWARE
    hints.ai_flags = AI_NUMERICHOST;
# endif
    if (U_UNLIKELY (getaddrinfo(address, NULL, &hints, &res) != 0)) {
      return NULL;
    }
    if (U_LIKELY (res->ai_family == AF_INET6 &&
      res->ai_addrlen == sizeof(struct sockaddr_in6))) {
      ((struct sockaddr_in6 *) res->ai_addr)->sin6_port = u_htons (port);
      ret = u_socketaddr_new_from_native(res->ai_addr, res->ai_addrlen);
    } else {
      ret = NULL;
    }
    freeaddrinfo(res);
    return ret;
  }
#endif
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(socketaddr_t))) == NULL)) {
    U_ERROR ("socketaddr_t::u_socketaddr_new: failed to allocate memory");
    return NULL;
  }
  ret->port = port;
#ifdef U_OS_WIN
  memset(&sa, 0, sizeof(sa));
  len = sizeof(sa);
  sin->sin_family = AF_INET;
  if (WSAStringToAddressA((LPSTR) address, AF_INET, NULL, (LPSOCKADDR) &sa, &len
  ) == 0) {
    memcpy(&ret->addr.sin_addr, &sin->sin_addr, sizeof(struct in_addr));
    ret->family = U_SOCKET_FAMILY_INET;
    return ret;
  }
# ifdef AF_INET6
  else {
    sin6->sin6_family = AF_INET6;
    if (WSAStringToAddressA((LPSTR) address, AF_INET6, NULL, (LPSOCKADDR) &sa,
      &len
    ) == 0) {
      memcpy(&ret->addr.sin6_addr, &sin6->sin6_addr, sizeof(struct in6_addr));
      ret->family = U_SOCKET_FAMILY_INET6;
      return ret;
    }
  }
# endif /* AF_INET6 */
#else /* U_OS_WIN */
  if (inet_pton(AF_INET, address, &ret->addr.sin_addr) > 0) {
    ret->family = U_SOCKET_FAMILY_INET;
    return ret;
  }
# ifdef AF_INET6
  else if (inet_pton(AF_INET6, address, &ret->addr.sin6_addr) > 0) {
    ret->family = U_SOCKET_FAMILY_INET6;
    return ret;
  }
# endif /* AF_INET6 */
#endif /* U_OS_WIN */
  u_free(ret);
  return NULL;
}

socketaddr_t *
u_socketaddr_new_any(socket_family_t family, u16_t port) {
  socketaddr_t *ret;
  ubyte_t any_addr[] = {0, 0, 0, 0};
#ifdef AF_INET6
  struct in6_addr any6_addr = IN6ADDR_ANY_INIT;
#endif

  if (U_UNLIKELY ((ret = u_malloc0(sizeof(socketaddr_t))) == NULL)) {
    U_ERROR (
      "socketaddr_t::u_socketaddr_new_any: failed to allocate memory");
    return NULL;
  }
  if (family == U_SOCKET_FAMILY_INET) {
    memcpy(&ret->addr.sin_addr, any_addr, sizeof(any_addr));
  }
#ifdef AF_INET6
  else if (family == U_SOCKET_FAMILY_INET6) {
    memcpy(&ret->addr.sin6_addr, &any6_addr.s6_addr, sizeof(any6_addr.s6_addr));
  }
#endif
  else {
    u_free(ret);
    return NULL;
  }
  ret->family = family;
  ret->port = port;
  return ret;
}

socketaddr_t *
u_socketaddr_new_loopback(socket_family_t family, u16_t port) {
  socketaddr_t *ret;
  ubyte_t loop_addr[] = {127, 0, 0, 0};
#ifdef AF_INET6
  struct in6_addr loop6_addr = IN6ADDR_LOOPBACK_INIT;
#endif

  if (U_UNLIKELY ((ret = u_malloc0(sizeof(socketaddr_t))) == NULL)) {
    U_ERROR (
      "socketaddr_t::u_socketaddr_new_loopback: failed to allocate memory");
    return NULL;
  }
  if (family == U_SOCKET_FAMILY_INET) {
    memcpy(&ret->addr.sin_addr, loop_addr, sizeof(loop_addr));
  }
#ifdef AF_INET6
  else if (family == U_SOCKET_FAMILY_INET6) {
    memcpy(
      &ret->addr.sin6_addr, &loop6_addr.s6_addr,
      sizeof(loop6_addr.s6_addr));
  }
#endif
  else {
    u_free(ret);
    return NULL;
  }
  ret->family = family;
  ret->port = port;
  return ret;
}

bool
u_socketaddr_to_native(const socketaddr_t *addr, ptr_t dest, size_t destlen) {
  struct sockaddr_in *sin;
#ifdef AF_INET6
  struct sockaddr_in6 *sin6;
#endif

  if (U_UNLIKELY (addr == NULL || dest == NULL || destlen == 0)) {
    return false;
  }
  sin = (struct sockaddr_in *) dest;
#ifdef AF_INET6
  sin6 = (struct sockaddr_in6 *) dest;
#endif
  if (addr->family == U_SOCKET_FAMILY_INET) {
    if (U_UNLIKELY (destlen < sizeof(struct sockaddr_in))) {
      U_WARNING (
        "socketaddr_t::u_socketaddr_to_native: invalid buffer size for IPv4");
      return false;
    }
    memcpy(&sin->sin_addr, &addr->addr.sin_addr, sizeof(struct in_addr));
    sin->sin_family = AF_INET;
    sin->sin_port = u_htons (addr->port);
    memset(sin->sin_zero, 0, sizeof(sin->sin_zero));
    return true;
  }
#ifdef AF_INET6
  else if (addr->family == U_SOCKET_FAMILY_INET6) {
    if (U_UNLIKELY (destlen < sizeof(struct sockaddr_in6))) {
      U_ERROR (
        "socketaddr_t::u_socketaddr_to_native: invalid buffer size for IPv6");
      return false;
    }
    memcpy(&sin6->sin6_addr, &addr->addr.sin6_addr, sizeof(struct in6_addr));
    sin6->sin6_family = AF_INET6;
    sin6->sin6_port = u_htons (addr->port);
#ifdef UNIC_SOCKADDR_IN6_HAS_FLOWINFO
    sin6->sin6_flowinfo = addr->flowinfo;
#endif
#ifdef UNIC_SOCKADDR_IN6_HAS_SCOPEID
    sin6->sin6_scope_id = addr->scope_id;
#endif
    return true;
  }
#endif
  else {
    U_WARNING (
      "socketaddr_t::u_socketaddr_to_native: unsupported socket address");
    return false;
  }
}

size_t
u_socketaddr_get_native_size(const socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return 0;
  }
  if (addr->family == U_SOCKET_FAMILY_INET) {
    return sizeof(struct sockaddr_in);
  }
#ifdef AF_INET6
  else if (addr->family == U_SOCKET_FAMILY_INET6) {
    return sizeof(struct sockaddr_in6);
  }
#endif
  else {
    U_WARNING (
      "socketaddr_t::u_socketaddr_get_native_size: unsupported socket family");
    return 0;
  }
}

socket_family_t
u_socketaddr_get_family(const socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return U_SOCKET_FAMILY_UNKNOWN;
  }
  return addr->family;
}

byte_t *
u_socketaddr_get_address(const socketaddr_t *addr) {
#ifdef AF_INET6
  byte_t buffer[INET6_ADDRSTRLEN];
#else
  byte_t   buffer[INET_ADDRSTRLEN];
#endif
#ifdef U_OS_WIN
  DWORD buflen = sizeof(buffer);
  DWORD addrlen;
  struct sockaddr_storage sa;
  struct sockaddr_in *sin;
# ifdef AF_INET6
  struct sockaddr_in6 *sin6;
# endif /* AF_INET6 */
#endif /* U_OS_WIN */

  if (U_UNLIKELY (addr == NULL || addr->family == U_SOCKET_FAMILY_UNKNOWN)) {
    return NULL;
  }
#ifdef U_OS_WIN
  sin = (struct sockaddr_in *) &sa;
# ifdef AF_INET6
  sin6 = (struct sockaddr_in6 *) &sa;
# endif /* AF_INET6 */
#endif /* U_OS_WIN */
#ifdef U_OS_WIN
  memset(&sa, 0, sizeof(sa));
#endif
#ifdef U_OS_WIN
  sa.ss_family = addr->family;
  if (addr->family == U_SOCKET_FAMILY_INET) {
    addrlen = sizeof(struct sockaddr_in);
    memcpy(&sin->sin_addr, &addr->addr.sin_addr, sizeof(struct in_addr));
  }
# ifdef AF_INET6
  else {
    addrlen = sizeof(struct sockaddr_in6);
    memcpy(&sin6->sin6_addr, &addr->addr.sin6_addr, sizeof(struct in6_addr));
  }
# endif /* AF_INET6 */
  if (U_UNLIKELY (WSAAddressToStringA((LPSOCKADDR) &sa,
    addrlen,
    NULL,
    (LPSTR) buffer,
    &buflen
  ) != 0)) {
    return NULL;
  }
#else /* !U_OS_WIN */
  if (addr->family == U_SOCKET_FAMILY_INET)
    inet_ntop(AF_INET, &addr->addr.sin_addr, buffer, sizeof(buffer));
# ifdef AF_INET6
  else
    inet_ntop(AF_INET6, &addr->addr.sin6_addr, buffer, sizeof(buffer));
# endif /* AF_INET6 */
#endif /* U_OS_WIN */
  return u_strdup(buffer);
}

u16_t
u_socketaddr_get_port(const socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return 0;
  }
  return addr->port;
}

u32_t
u_socketaddr_get_flow_info(const socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return 0;
  }
#if !defined (AF_INET6) || !defined (UNIC_SOCKADDR_IN6_HAS_FLOWINFO)
  return 0;
#else
  if (U_UNLIKELY (addr->family != U_SOCKET_FAMILY_INET6)) {
    return 0;
  }
  return addr->flowinfo;
#endif
}

u32_t
u_socketaddr_get_scope_id(const socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return 0;
  }
#if !defined (AF_INET6) || !defined (UNIC_SOCKADDR_IN6_HAS_SCOPEID)
  return 0;
#else
  if (U_UNLIKELY (addr->family != U_SOCKET_FAMILY_INET6)) {
    return 0;
  }
  return addr->scope_id;
#endif
}

void
u_socketaddr_set_flow_info(socketaddr_t *addr, u32_t flowinfo) {
  if (U_UNLIKELY (addr == NULL)) {
    return;
  }
#if !defined (AF_INET6) || !defined (UNIC_SOCKADDR_IN6_HAS_FLOWINFO)
  U_UNUSED (flowinfo);
  return;
#else
  if (U_UNLIKELY (addr->family != U_SOCKET_FAMILY_INET6)) {
    return;
  }
  addr->flowinfo = flowinfo;
#endif
}

void
u_socketaddr_set_scope_id(socketaddr_t *addr, u32_t scope_id) {
  if (U_UNLIKELY (addr == NULL)) {
    return;
  }
#if !defined (AF_INET6) || !defined (UNIC_SOCKADDR_IN6_HAS_SCOPEID)
  U_UNUSED (scope_id);
  return;
#else
  if (U_UNLIKELY (addr->family != U_SOCKET_FAMILY_INET6)) {
    return;
  }
  addr->scope_id = scope_id;
#endif
}

bool
u_socketaddr_is_flow_info_supported(void) {
#ifdef AF_INET6
# ifdef UNIC_SOCKADDR_IN6_HAS_FLOWINFO
  return true;
# else
  return false;
# endif
#else
  return false;
#endif
}

bool
u_socketaddr_is_scope_id_supported(void) {
#ifdef AF_INET6
# ifdef UNIC_SOCKADDR_IN6_HAS_SCOPEID
  return true;
# else
  return false;
# endif
#else
  return false;
#endif
}

bool
u_socketaddr_is_ipv6_supported(void) {
#ifdef AF_INET6
  return true;
#else
  return false;
#endif
}

bool
u_socketaddr_is_any(const socketaddr_t *addr) {
  u32_t addr4;

  if (U_UNLIKELY (addr == NULL || addr->family == U_SOCKET_FAMILY_UNKNOWN)) {
    return false;
  }
  if (addr->family == U_SOCKET_FAMILY_INET) {
    addr4 = u_ntohl (*((u32_t *) &addr->addr.sin_addr));
    return (addr4 == INADDR_ANY);
  }
#ifdef AF_INET6
  else {
    return IN6_IS_ADDR_UNSPECIFIED (&addr->addr.sin6_addr);
  }
#else
  else
    return false;
#endif
}

bool
u_socketaddr_is_loopback(const socketaddr_t *addr) {
  u32_t addr4;

  if (U_UNLIKELY (addr == NULL || addr->family == U_SOCKET_FAMILY_UNKNOWN)) {
    return false;
  }
  if (addr->family == U_SOCKET_FAMILY_INET) {
    addr4 = u_ntohl (*((u32_t *) &addr->addr.sin_addr));

    /* 127.0.0.0/8 */
    return ((addr4 & 0xff000000) == 0x7f000000);
  }
#ifdef AF_INET6
  else {
    return IN6_IS_ADDR_LOOPBACK (&addr->addr.sin6_addr);
  }
#else
  else
    return false;
#endif
}

void
u_socketaddr_free(socketaddr_t *addr) {
  if (U_UNLIKELY (addr == NULL)) {
    return;
  }
  u_free(addr);
}
