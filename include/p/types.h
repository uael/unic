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

/*!@file p/types.h
 * @brief Types definitions
 * @author Alexander Saprykin
 *
 * Every operating system in pair with a compiler has its own set of data types.
 * Here you can find unified platform independent data types which guarantee the
 * same bit-size on every supported platform: #pint8, #pint16, #pint32, #pint64
 * and their unsigned variants. Also other types are defined for convinience:
 * #ppointer, #pboolean, #pint, #plong, #pdouble and more.
 *
 * Along with the types, length and format modifiers are defined. They can be
 * used to print and scan data from/to a variable.
 *
 * Sometimes it is useful to use an integer variable as a pointer, i.e. to
 * prevent memory allocation when using hash tables or trees. Use special macros
 * for that case: #PINT_TO_POINTER, #PPOINTER_TO_INT and their variants. Note
 * that it will not work with 64-bit data types.
 *
 * To check data type limits use P_MIN* and P_MAX* macros.
 *
 * If you need to check system endianness compare the P_BYTE_ORDER definition
 * with the #P_LITTLE_ENDIAN or #P_BIG_ENDIAN macro.
 *
 * To convert between the little and big endian byte orders use the Px_TO_LE,
 * Px_TO_BE, Px_FROM_LE and Px_FROM_BE macros. Macros for the network<->host
 * byte order conversion are also provided: #p_ntohl, #p_ntohs, #p_ntohs and
 * #p_ntohl. All the described above macros depend on the target system
 * endianness. Use PUINTx_SWAP_BYTES to manually swap data types independently
 * from the endianness.
 *
 * You can also find some of the function definitions used within the library.
 */
#ifndef P_TYPES_H__
# define P_TYPES_H__

#include "plibsysconfig.h"
#include "p/macros.h"

/*!@brief Type for signed 8 bit. */
typedef signed char int8_t;

/*!@brief Type for unsigned 8 bit. */
typedef unsigned char uint8_t;

/*!@brief Type for signed 16 bit. */
typedef signed short int16_t;

/*!@brief Type for unsigned 16 bit. */
typedef unsigned short uint16_t;

/*!@brief Type for signed 32 bit. */
typedef signed int int32_t;

/*!@brief Type for unsigned 32 bit. */
typedef unsigned int uint32_t;

/*!@var int64_t
 * @brief Type for signed 64 bit.
 */

/*!@var uint64_t
 * @brief Type for unsigned 64 bit.
 */

#if 0 && defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
# if PLIBSYS_SIZEOF_LONG == 8
typedef signed long int64_t;
typedef unsigned long uint64_t;
# else

typedef signed long long int64_t;

typedef unsigned long long uint64_t;

# endif
#endif

/*!@brief Type for a pointer. */
typedef void *ptr_t;

/*!@brief Type for a const pointer. */
typedef const void *const_ptr_t;

/*!@brief Type for a char. */
typedef char byte_t;

/*!@brief Type for a short. */
typedef short short_t;

/*!@brief Type for an int. */
typedef int int_t;

/*!@brief Type for a long. */
typedef long long_t;

/*!@brief Type for an unsigned char. */
typedef unsigned char ubyte_t;

/*!@brief Type for an unsigned short. */
typedef unsigned short ushort_t;

/*!@brief Type for an unsigned int. */
typedef unsigned int uint_t;

/*!@brief Type for an unsigned long. */
typedef unsigned long ulong_t;

/*!@brief Type for a float. */
typedef float float_t;

/*!@brief Type for a double precision float. */
typedef double double_t;

#ifndef __cplusplus
# if defined (P_CC_MSVC) && _MSC_VER < 1900
#   define bool unsigned char
#   define true 1
#   define false 0
#   define __bool_true_false_are_defined 1
# else
#   include <stdbool.h>
# endif
#endif

/*!@var ssize_t
 * @brief Type for a platform independent signed size_t.
 */

/*!@var size_t
 * @brief Type for a platform independent size_t.
 */

/*!@def PSIZE_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #psize or #pssize type for printing and scanning values. It is a string
 * literal, but doesn't include the percent sign so you can add precision and
 * length modifiers and append a conversion specifier.
 * @code
 * psize size_val = 256;
 * printf ("%#" PSIZE_MODIFIER "x", size_val);
 * @endcode
 */

/*!@def PSSIZE_FORMAT
 * @brief Platform dependent conversion specifier of #pssize type for printing
 * and scanning values.
 * @code
 * pssize size_val = 100;
 * printf ("%" PSSIZE_FORMAT, size_val);
 * @endcode
 */

/*!@def PSIZE_FORMAT
 * @brief Platform dependent conversion specifier of #psize type for printing
 * and scanning values.
 */

/*!@def P_MAXSIZE
 * @brief Maximum value of a #psize type.
 */

/*!@def P_MINSSIZE
 * @brief Minimum value of a #pssize type.
 */

/*!@def P_MAXSSIZE
 * @brief Maximum value of a #pssize type.
 */

#if PLIBSYS_SIZEOF_SIZE_T == 8
# if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
typedef signed __int64 ssize_t;
#   ifdef __need_size_t
typedef unsigned __int64 size_t;
#   endif
#   define PSIZE_MODIFIER "I64"
#   define PSSIZE_FORMAT "I64d"
#   define PSIZE_FORMAT "I64u"
#   define P_MAXSIZE  P_MAXUINT64
#   define P_MINSSIZE  P_MININT64
#   define P_MAXSSIZE  P_MAXINT64
# else
#   if PLIBSYS_SIZEOF_LONG == 8
typedef long ssize_t;
#     ifdef __need_size_t
typedef unsigned long size_t;
#     endif
#     define PSIZE_MODIFIER    "l"
#     define PSSIZE_FORMAT    "li"
#     define PSIZE_FORMAT    "lu"
#     define P_MAXSIZE    P_MAXULONG
#     define P_MINSSIZE    P_MINLONG
#     define P_MAXSSIZE    P_MAXLONG
#   else
typedef long long ssize_t;
#     ifdef __need_size_t
typedef unsigned long long size_t;
#     endif
#     define PSIZE_MODIFIER  "ll"
#     define PSSIZE_FORMAT  "lli"
#     define PSIZE_FORMAT  "llu"
#     define P_MAXSIZE  P_MAXUINT64
#     define P_MINSSIZE  P_MININT64
#     define P_MAXSSIZE  P_MAXINT64
#   endif
# endif
#else
typedef signed int ssize_t;
#   ifdef __need_size_t
typedef unsigned int size_t;
#   endif
# define PSIZE_MODIFIER  ""
# define PSSIZE_FORMAT    "i"
# define PSIZE_FORMAT    "u"
# define P_MAXSIZE    P_MAXUINT
# define P_MINSSIZE    P_MININT
# define P_MAXSSIZE    P_MAXINT
#endif

/*!@var intptr_t
 * @brief Type for a platform independent signed pointer represented by an
 * integer.
 */

/*!@var uintptr_t
 * @brief Type for a platform independent unsigned pointer represented by an
 * integer.
 */

/*!@def PINTPTR_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #pintptr or #puintptr type for printing and scanning values. It is a string
 * literal, but doesn't include the percent sign so you can add precision and
 * length modifiers and append a conversion specifier.
 */

/*!@def PINTPTR_FORMAT
 * @brief Platform dependent conversion specifier of #pintptr type for printing
 * and scanning values.
 */

/*!@def PUINTPTR_FORMAT
 * @brief Platform dependent conversion specifier of #puintptr type for
 * printing and scanning values.
 */

#if PLIBSYS_SIZEOF_VOID_P == 8
# if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
typedef signed __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#   define PINTPTR_MODIFIER "I64"
#   define PINTPTR_FORMAT "I64i"
#   define PUINTPTR_FORMAT "I64u"
# else
#   if PLIBSYS_SIZEOF_LONG == 8
typedef long intptr_t;
typedef unsigned long uintptr_t;
#     define PINTPTR_MODIFIER    "l"
#     define PINTPTR_FORMAT    "li"
#     define PUINTPTR_FORMAT    "lu"
#   else
typedef long long  intptr_t;
typedef unsigned long long uintptr_t;
#     define PINTPTR_MODIFIER  "ll"
#     define PINTPTR_FORMAT  "lli"
#     define PUINTPTR_FORMAT  "llu"
#   endif
# endif
#else
typedef signed int intptr_t;
typedef unsigned int uintptr_t;
# define PINTPTR_MODIFIER  ""
# define PINTPTR_FORMAT  "i"
# define PUINTPTR_FORMAT  "u"
#endif

/*!@brief Platform independent offset_t definition. */
typedef int64_t poffset;

#if PLIBSYS_SIZEOF_VOID_P == 8
# define P_INT_TO_POINTER(i) ((void *) (long long) (i))
# define P_POINTER_TO_INT(p) ((int) (long long) (p))
# define PPOINTER_TO_INT(p) ((int_t) ((int64_t) (p)))
# define PPOINTER_TO_UINT(p) ((uint_t) ((uint64_t) (p)))
# define PINT_TO_POINTER(i) ((ptr_t) (int64_t) (i))
# define PUINT_TO_POINTER(u) ((ptr_t) (uint64_t) (u))
#else
# define P_INT_TO_POINTER(i) ((void *) (long) (i))
# define P_POINTER_TO_INT(p) ((int) (long) (p))
# define PPOINTER_TO_INT(p) ((int_t) ((long_t) (p)))
# define PPOINTER_TO_UINT(p) ((uint_t) ((ulong_t) (p)))
# define PINT_TO_POINTER(i) ((ptr_t) (long_t) (i))
# define PUINT_TO_POINTER(u) ((ptr_t) (ulong_t) (u))
#endif

/*!@def P_INT_TO_POINTER
 * @brief Casts an int to a pointer.
 * @param i Variable to cast.
 * @return Casted variable.
 * @since 0.0.1
 */

/*!@def P_POINTER_TO_INT
 * @brief Casts a pointer to an int.
 * @param p Pointer to cast.
 * @return Casted pointer.
 * @since 0.0.1
 */

/*!@def PPOINTER_TO_INT
 * @brief Casts a #ppointer to a #pint value.
 * @param p #ppointer to cast.
 * @return Casted #ppointer.
 * @since 0.0.1
 */

/*!@def PPOINTER_TO_UINT
 * @brief Casts a #ppointer to a #pint value.
 * @param p #ppointer to cast.
 * @return Casted #ppointer.
 * @since 0.0.1
 */

/*!@def PINT_TO_POINTER
 * @brief Casts a #pint value to a #ppointer.
 * @param i #pint to cast.
 * @return Casted #pint.
 * @since 0.0.1
 */

/*!@def PUINT_TO_POINTER
 * @brief Casts a #puint value to a #ppointer.
 * @param u #puint to cast.
 * @return Casted #puint.
 * @since 0.0.1
 */

/*!@brief Casts a #psize value to a #ppointer. */
#define PSIZE_TO_POINTER(i) ((ptr_t) ((size_t) (i)))
/*!@brief Casts a #ppointer to a #psize value. */
#define PPOINTER_TO_PSIZE(p) ((size_t) (p))

/*!@brief Min value for a 8-bit int. */
#define P_MININT8 ((int8_t) 0x80)
/*!@brief Max value for a 8-bit int. */
#define P_MAXINT8 ((int8_t) 0x7F)
/*!@brief Max value for a 8-bit unsigned int. */
#define P_MAXUINT8 ((uint8_t) 0xFF)

/*!@brief Min value for a 16-bit int. */
#define P_MININT16 ((int16_t) 0x8000)
/*!@brief Max value for a 16-bit int. */
#define P_MAXINT16 ((int16_t) 0x7FFF)
/*!@brief Max value for a 16-bit unsigned int. */
#define P_MAXUINT16 ((uint16_t) 0xFFFF)

/*!@brief Min value for a 32-bit int. */
#define P_MININT32 ((int32_t) 0x80000000)
/*!@brief Max value for a 32-bit int. */
#define P_MAXINT32 ((int32_t) 0x7FFFFFFF)
/*!@brief Max value for a 32-bit unsigned int. */
#define P_MAXUINT32 ((uint32_t) 0xFFFFFFFF)

/*!@brief Min value for a 64-bit int. */
#define P_MININT64 ((int64_t) 0x8000000000000000LL)
/*!@brief Max value for a 64-bit int. */
#define P_MAXINT64 ((int64_t) 0x7FFFFFFFFFFFFFFFLL)
/*!@brief Max value for a 64-bit unsigned int. */
#define P_MAXUINT64 ((uint64_t) 0xFFFFFFFFFFFFFFFFULL)

/*!@def PINT16_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #pint16 or #puint16 type for printing and scanning values. It is a string
 * literal, but doesn't include the percent sign so you can add precision and
 * length modifiers and append a conversion specifier.
 */

/*!@def PINT16_FORMAT
 * @brief Platform dependent conversion specifier of #pint16 type for printing
 * and scanning values.
 */

/*!@def PUINT16_FORMAT
 * @brief Platform dependent conversion specifier of #puint16 type for printing
 * and scanning values.
 */

/*!@def PINT32_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #pint32 or #puint32 type for printing and scanning values. It is a string
 * literal, but doesn't include the percent sign so you can add precision and
 * length modifiers and append a conversion specifier.
 */

/*!@def PINT32_FORMAT
 * @brief Platform dependent conversion specifier of #pint32 type for printing
 * and scanning values.
 */

/*!@def PUINT32_FORMAT
 * @brief Platform dependent conversion specifier of #puint32 type for printing
 * and scanning values.
 */

/*!@def PINT64_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #pint64 or #puint64 type for printing and scanning values. It is a string
 * literal, but doesn't include the percent sign so you can add precision and
 * length modifiers and append a conversion specifier.
 */

/*!@def PINT64_FORMAT
 * @brief Platform dependent conversion specifier of #pint64 type for printing
 * and scanning values.
 */

/*!@def PUINT64_FORMAT
 * @brief Platform dependent conversion specifier of #puint64 type for printing
 * and scanning values.
 */

/*!@def POFFSET_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers of
 * #poffset type for printing and scanning values. It is a string literal, but
 * doesn't include the percent sign so you can add precision and length
 * modifiers and append a conversion specifier.
 */

/*!@def POFFSET_FORMAT
 * @brief Platform dependent conversion specifier of #poffset type for printing
 * and scanning values.
 */

#if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
# define PINT16_MODIFIER "h"
#else
# define PINT16_MODIFIER ""
#endif

#define PINT16_FORMAT "hi"
#define PUINT16_FORMAT "hu"
#define PINT32_MODIFIER ""
#define PINT32_FORMAT "i"
#define PUINT32_FORMAT "u"

#if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
# define PINT64_MODIFIER "I64"
# define PINT64_FORMAT "I64i"
# define PUINT64_FORMAT "I64u"
#else
# if PLIBSYS_SIZEOF_LONG == 8
#   define PINT64_MODIFIER "l"
#   define PINT64_FORMAT "li"
#   define PUINT64_FORMAT "lu"
# else
#   define PINT64_MODIFIER "ll"
#   define PINT64_FORMAT "lli"
#   define PUINT64_FORMAT "llu"
# endif
#endif

#define POFFSET_MODIFIER PINT64_MODIFIER
#define POFFSET_FORMAT PINT64_FORMAT

/* Endian checks, see P_BYTE_ORDER in plibsysconfig.h */

/*!@brief Little endian mark. */
#define P_LITTLE_ENDIAN 1234
/*!@brief Big endian mark. */
#define P_BIG_ENDIAN 4321

/*!@def PINT16_TO_LE
 * @brief Swaps a #pint16 variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT16_TO_LE
 * @brief Swaps a #puint16 variable from the host to the little the endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT16_TO_BE
 * @brief Swaps a #pint16 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT16_TO_BE
 * @brief Swaps a #puint16 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT32_TO_LE
 * @brief Swaps a #pint32 variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT32_TO_LE
 * @brief Swaps a #puint32 variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT32_TO_BE
 * @brief Swaps a #pint32 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT32_TO_BE
 * @brief Swaps a #puint32 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT64_TO_LE
 * @brief Swaps a #pint64 variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT64_TO_LE
 * @brief Swaps a #puint64 variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT64_TO_BE
 * @brief Swaps a #pint64 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT64_TO_BE
 * @brief Swaps a #puint64 variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PLONG_TO_LE
 * @brief Swaps a #plong variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PULONG_TO_LE
 * @brief Swaps a #pulong variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PLONG_TO_BE
 * @brief Swaps a #plong variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PULONG_TO_BE
 * @brief Swaps a #pulong variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PSSIZE_TO_LE
 * @brief Swaps a #pssize variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PSIZE_TO_LE
 * @brief Swaps a #psize variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PSSIZE_TO_BE
 * @brief Swaps a #pssize variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PSIZE_TO_BE
 * @brief Swaps a #psize variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT_TO_LE
 * @brief Swaps a #pint variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT_TO_LE
 * @brief Swaps a #puint variable from the host to the little endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PINT_TO_BE
 * @brief Swaps a #pint variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/*!@def PUINT_TO_BE
 * @brief Swaps a #puint variable from the host to the big endian order.
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

#if P_BYTE_ORDER == P_LITTLE_ENDIAN
#define PINT16_TO_LE(val) ((int16_t)(val))
#define PUINT16_TO_LE(val) ((uint16_t)(val))
#define PINT16_TO_BE(val) ((int16_t) PUINT16_SWAP_BYTES(val))
#define PUINT16_TO_BE(val) (PUINT16_SWAP_BYTES(val))
#define PINT32_TO_LE(val) ((int32_t)(val))
#define PUINT32_TO_LE(val) ((uint32_t)(val))
#define PINT32_TO_BE(val) ((int32_t) PUINT32_SWAP_BYTES(val))
#define PUINT32_TO_BE(val) (PUINT32_SWAP_BYTES(val))
#define PINT64_TO_LE(val) ((int64_t)(val))
#define PUINT64_TO_LE(val) ((uint64_t)(val))
#define PINT64_TO_BE(val) ((int64_t) PUINT64_SWAP_BYTES(val))
#define PUINT64_TO_BE(val) (PUINT64_SWAP_BYTES(val))
# if PLIBSYS_SIZEOF_LONG == 8
#   define PLONG_TO_LE(val) ((long_t) PINT64_TO_LE(val))
#   define PULONG_TO_LE(val) ((ulong_t) PUINT64_TO_LE(val))
#   define PLONG_TO_BE(val) ((long_t) PINT64_TO_BE(val))
#   define PULONG_TO_BE(val) ((ulong_t) PUINT64_TO_BE(val))
# else
#   define PLONG_TO_LE(val) ((long_t) PINT32_TO_LE(val))
#   define PULONG_TO_LE(val) ((ulong_t) PUINT32_TO_LE(val))
#   define PLONG_TO_BE(val) ((long_t) PINT32_TO_BE(val))
#   define PULONG_TO_BE(val) ((ulong_t) PUINT32_TO_BE(val))
# endif
# if PLIBSYS_SIZEOF_SIZE_T == 8
#   define PSIZE_TO_LE(val) ((size_t) PUINT64_TO_LE(val))
#   define PSSIZE_TO_LE(val) ((ssize_t) PINT64_TO_LE(val))
#   define PSIZE_TO_BE(val) ((size_t) PUINT64_TO_BE(val))
#   define PSSIZE_TO_BE(val) ((ssize_t) PINT64_TO_BE(val))
# else
#   define PSIZE_TO_LE(val) ((size_t) PUINT32_TO_LE(val))
#   define PSSIZE_TO_LE(val) ((ssize_t) PINT32_TO_LE(val))
#   define PSIZE_TO_BE(val) ((size_t) PUINT32_TO_BE(val))
#   define PSSIZE_TO_BE(val) ((ssize_t) PINT32_TO_BE(val))
# endif
# define PINT_TO_LE(val) ((int_t) PINT32_TO_LE(val))
# define PUINT_TO_LE(val) ((uint_t) PUINT32_TO_LE(val))
# define PINT_TO_BE(val) ((int_t) PINT32_TO_BE(val))
# define PUINT_TO_BE(val) ((uint_t) PUINT32_TO_BE(val))
#else
# define PINT16_TO_LE(val) ((int16_t) PUINT16_SWAP_BYTES(val))
# define PUINT16_TO_LE(val) (PUINT16_SWAP_BYTES(val))
# define PINT16_TO_BE(val) ((int16_t)(val))
# define PUINT16_TO_BE(val) ((uint16_t)(val))
# define PINT32_TO_LE(val) ((int32_t) PUINT32_SWAP_BYTES(val))
# define PUINT32_TO_LE(val) (PUINT32_SWAP_BYTES(val))
# define PINT32_TO_BE(val) ((int32_t)(val))
# define PUINT32_TO_BE(val) ((uint32_t)(val))
# define PINT64_TO_LE(val) ((int64_t) PUINT64_SWAP_BYTES *)
# define PUINT64_TO_LE(val) (PUINT64_SWAP_BYTES(val))
# define PINT64_TO_BE(val) ((int64_t)(val))
# define PUINT64_TO_BE(val) ((uint64_t)(val))
# if PLIBSYS_SIZEOF_LONG == 8
#   define PLONG_TO_LE(val) ((long_t) PINT64_TO_LE(val))
#   define PULONG_TO_LE(val) ((ulong_t) PUINT64_TO_LE(val))
#   define PLONG_TO_BE(val) ((long_t) PINT64_TO_BE(val))
#   define PULONG_TO_BE(val) ((ulong_t) PUINT64_TO_BE(val))
# else
#   define PLONG_TO_LE(val) ((long_t) PINT32_TO_LE(val))
#   define PULONG_TO_LE(val) ((ulong_t) PUINT32_TO_LE(val))
#   define PLONG_TO_BE(val) ((long_t) PINT32_TO_BE(val))
#   define PULONG_TO_BE(val) ((ulong_t) PUINT32_TO_BE(val))
# endif
# if PLIBSYS_SIZEOF_SIZE_T == 8
#   define PSIZE_TO_LE(val) ((size_t) PUINT64_TO_LE(val))
#   define PSSIZE_TO_LE(val) ((ssize_t) PINT64_TO_LE(val))
#   define PSIZE_TO_BE(val) ((size_t) PUINT64_TO_BE(val))
#   define PSSIZE_TO_BE(val) ((ssize_t) PINT64_TO_BE(val))
# else
#   define PSIZE_TO_LE(val) ((size_t) PUINT32_TO_LE(val))
#   define PSSIZE_TO_LE(val) ((ssize_t) PINT32_TO_LE(val))
#   define PSIZE_TO_BE(val) ((size_t) PUINT32_TO_BE(val))
#   define PSSIZE_TO_BE(val) ((ssize_t) PINT32_TO_BE(val))
# endif
#   define PINT_TO_LE(val) ((int_t) PINT32_TO_LE(val))
#   define PUINT_TO_LE(val) ((uint_t) PUINT32_TO_LE(val))
#   define PINT_TO_BE(val) ((int_t) PINT32_TO_BE(val))
#   define PUINT_TO_BE(val) ((uint_t) PUINT32_TO_BE(val))
#endif

/* Functions for bit swapping */

/*!@brief Swaps a 16-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_SWAP_BYTES(val) \
  ((uint16_t) (((uint16_t)(val)) >> 8 | ((uint16_t)(val)) << 8))

/*!@brief Swaps a 32-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_SWAP_BYTES(val) ((uint32_t) ( \
  (((uint32_t)(val)) >> 24) | \
  ((((uint32_t)(val)) << 8) & ((uint32_t) 0x00FF0000U)) | \
  ((((uint32_t)(val)) >> 8) & ((uint32_t) 0x0000FF00U)) | \
  (((uint32_t)(val)) << 24)))

/*!@brief Swaps a 64-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_SWAP_BYTES(val) ((uint64_t) ( \
  (((uint64_t)(val))  >> 56) | \
  ((((uint64_t)(val)) << 40) & ((uint64_t) 0x00FF000000000000ULL)) | \
  ((((uint64_t)(val)) << 24) & ((uint64_t) 0x0000FF0000000000ULL)) | \
  ((((uint64_t)(val)) <<  8) & ((uint64_t) 0x000000FF00000000ULL)) | \
  ((((uint64_t)(val)) >>  8) & ((uint64_t) 0x00000000FF000000ULL)) | \
  ((((uint64_t)(val)) >> 24) & ((uint64_t) 0x0000000000FF0000ULL)) | \
  ((((uint64_t)(val)) >> 40) & ((uint64_t) 0x000000000000FF00ULL)) | \
  (((uint64_t)(val))  << 56)))

/* Functions, similar to ?_TO_? functions */

/*!@brief Swaps a 16-bit int from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 16-bit int.
 * @since 0.0.1
 */
#define PINT16_FROM_LE(val) (PINT16_TO_LE(val))

/*!@brief Swaps a 16-bit unsigned int from the little endian byte order to the
 * host one.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_FROM_LE(val) (PUINT16_TO_LE(val))

/*!@brief Swaps a 16-bit int from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 16-bit int.
 * @since 0.0.1
 */
#define PINT16_FROM_BE(val) (PINT16_TO_BE(val))

/*!@brief Swaps a 16-bit unsigned int from the big endian byte order to the host
 * one.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_FROM_BE(val) (PUINT16_TO_BE(val))


/*!@brief Swaps a 32-bit int from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 32-bit int.
 * @since 0.0.1
 */
#define PINT32_FROM_LE(val) (PINT32_TO_LE(val))

/*!@brief Swaps a 32-bit unsigned int from the little endian byte order to the
 * host one.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_FROM_LE(val) (PUINT32_TO_LE(val))

/*!@brief Swaps a 32-bit int from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 32-bit int.
 * @since 0.0.1
 */
#define PINT32_FROM_BE(val) (PINT32_TO_BE(val))

/*!@brief Swaps a 32-bit unsigned int from the big endian byte order to the host
 * one.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_FROM_BE(val) (PUINT32_TO_BE(val))

/*!@brief Swaps a 64-bit int from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 64-bit int.
 * @since 0.0.1
 */
#define PINT64_FROM_LE(val) (PINT64_TO_LE(val))

/*!@brief Swaps a 64-bit unsigned int from the little endian byte order to the
 * host one.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_FROM_LE(val) (PUINT64_TO_LE(val))

/*!@brief Swaps a 64-bit int from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped 64-bit int.
 * @since 0.0.1
 */
#define PINT64_FROM_BE(val) (PINT64_TO_BE(val))

/*!@brief Swaps a 64-bit unsigned int from the big endian byte order to the host
 * one.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_FROM_BE(val) (PUINT64_TO_BE(val))

/*!@brief Swaps a long int from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define PLONG_FROM_LE(val) (PLONG_TO_LE(val))

/*!@brief Swaps an unsigned long int from the little endian byte order to the
 * host one.
 * @param val Value to swap.
 * @return Swapped unsigned long int.
 * @since 0.0.1
 */
#define PULONG_FROM_LE(val) (PULONG_TO_LE(val))

/*!@brief Swaps a long int from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define PLONG_FROM_BE(val) (PLONG_TO_BE(val))

/*!@brief Swaps an unsigned long int from the big endian byte order to the host
 * one.
 * @param val Value to swap.
 * @return Swapped unsigned long int.
 * @since 0.0.1
 */
#define PULONG_FROM_BE(val) (PULONG_TO_BE(val))

/*!@brief Swaps a #pint from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #pint.
 * @since 0.0.1
 */
#define PINT_FROM_LE(val) (PINT_TO_LE(val))

/*!@brief Swaps a #puint from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #puint.
 * @since 0.0.1
 */
#define PUINT_FROM_LE(val) (PUINT_TO_LE(val))

/*!@brief Swaps a #pint from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #pint.
 * @since 0.0.1
 */
#define PINT_FROM_BE(val) (PINT_TO_BE(val))

/*!@brief Swaps a #puint from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #puint.
 * @since 0.0.1
 */
#define PUINT_FROM_BE(val) (PUINT_TO_BE(val))

/*!@brief Swaps a #psize from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #psize.
 * @since 0.0.1
 */
#define PSIZE_FROM_LE(val) (PSIZE_TO_LE(val))

/*!@brief Swaps a #pssize from the little endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #pssize.
 * @since 0.0.1
 */
#define PSSIZE_FROM_LE(val) (PSSIZE_TO_LE(val))

/*!@brief Swaps a #psize from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #psize.
 * @since 0.0.1
 */
#define PSIZE_FROM_BE(val) (PSIZE_TO_BE(val))

/*!@brief Swaps a #pssize from the big endian byte order to the host one.
 * @param val Value to swap.
 * @return Swapped #pssize.
 * @since 0.0.1
 */
#define PSSIZE_FROM_BE(val) (PSSIZE_TO_BE(val))

/* Host-network order functions */

/*!@brief Swaps a long int from the network byte order to the host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define p_ntohl(val) (PUINT32_FROM_BE(val))

/*!@brief Swaps a short int from the network byte order to the host one.
 * @param val Value to swap.
 * @return Swapped short int.
 * @since 0.0.1
 */
#define p_ntohs(val) (PUINT16_FROM_BE(val))

/*!@brief Swaps a long int from the host byte order to the network one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define p_htonl(val) (PUINT32_TO_BE(val))

/*!@brief Swaps a short int from the host byte order to the network one.
 * @param val Value to swap.
 * @return Swapped short int.
 * @since 0.0.1
 */
#define p_htons(val) (PUINT16_TO_BE(val))

/*!@brief Platform independent system handle. */
typedef void *P_HANDLE;

/*!@brief Function to traverse through a key-value container.
 * @param key The key of an item.
 * @param value The value of the item.
 * @param user_data Data provided by a user, maybe NULL.
 * @return FALSE to continue traversing, TRUE to stop it.
 * @since 0.0.1
 */
typedef bool (*traverse_fn_t)(ptr_t key, ptr_t value, ptr_t user_data);

/*!@brief General purpose function.
 * @param data Main argument related to a context value.
 * @param user_data Additional (maybe NULL) user-provided data.
 * @since 0.0.1
 */
typedef void (*fn_t)(ptr_t data, ptr_t user_data);

/*!@brief Object destroy notification function.
 * @param data Pointer to an object to be destroyed.
 * @since 0.0.1
 */
typedef void (*destroy_fn_t)(ptr_t data);

/*!@brief Compares two values.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @return -1 if the first value is less than the second, 1 if the first value
 * is greater than the second, 0 otherwise.
 * @since 0.0.1
 */
typedef int_t (*cmp_fn_t)(const_ptr_t a, const_ptr_t b);

/*!@brief Compares two values with additional data.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data Addition data, may be NULL.
 * @return -1 if the first value is less than the second, 1 if the first value
 * is greater than the second, 0 otherwise.
 * @since 0.0.1
 */
typedef int_t(*cmp_data_fn_t)(const_ptr_t a, const_ptr_t b, ptr_t data);

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#endif /* !P_TYPES_H__ */
