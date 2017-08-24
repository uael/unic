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

#ifndef UNIC_HEADER_UNIC_PRIVATE_H
# define UNIC_HEADER_UNIC_PRIVATE_H

#include "unic/macros.h"
#include "unic/types.h"

#ifndef UNIC_HAS_SOCKLEN_T
# ifdef U_OS_VMS
typedef unsigned int socklen_t;
# else
typedef int socklen_t;
# endif
#endif
#ifndef UNIC_HAS_SOCKADDR_STORAGE
/* According to RFC 2553 */
# define _UNIC_SS_MAXSIZE  128
# define _UNIC_SS_ALIGNSIZE  (sizeof (i64_t))

# ifdef UNIC_SOCKADDR_HAS_SA_LEN
#   define _UNIC_SS_PAD1SIZE (_UNIC_SS_ALIGNSIZE - (sizeof (ubyte_t) + sizeof (ubyte_t)))
# else
#   define _UNIC_SS_PAD1SIZE  (_UNIC_SS_ALIGNSIZE - sizeof (ubyte_t))
# endif

# define _UNIC_SS_PAD2SIZE  (_UNIC_SS_MAXSIZE - (sizeof (ubyte_t) + _UNIC_SS_PAD1SIZE + _UNIC_SS_ALIGNSIZE))

struct sockaddr_storage {
# ifdef UNIC_SOCKADDR_HAS_SA_LEN
  ubyte_t  ss_len;
# endif
# ifdef UNIC_SIZEOF_SAFAMILY_T
#   if (UNIC_SIZEOF_SAFAMILY_T == 1)
  ubyte_t  ss_family;
#   elif (UNIC_SIZEOF_SAFAMILY_T == 2)
  ushort_t ss_family;
#   else
  uint_t  ss_family;
#   endif
# else
#   ifdef UNIC_SOCKADDR_HAS_SA_LEN
  ubyte_t  ss_family;
#   else
  ushort_t  ss_family;
#   endif
# endif
  byte_t __ss_pad1[_UNIC_SS_PAD1SIZE];
  i64_t __ss_align;
  byte_t __ss_pad2[_UNIC_SS_PAD2SIZE];
};
#endif
#endif /* UNIC_HEADER_UNIC_PRIVATE_H */
