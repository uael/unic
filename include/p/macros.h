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

/*!@file p/macros.h
 * @brief Miscellaneous macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * This family of macros provides various additional capabilities (compiler
 * hints, attributes, version, etc.).
 */
#ifndef P_MACROS_H__
# define P_MACROS_H__

#include "cc.h"
#include "arch.h"
#include "os.h"
#include <stdio.h>

/* For Clang */
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/*!@def P_GNUC_WARN_UNUSED_RESULT
 * @brief Gives a warning if the result returned from a function is not being
 * used.
 * @since 0.0.1
 */

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) || \
    __has_attribute(warn_unused_result)
# define P_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define P_GNUC_WARN_UNUSED_RESULT
#endif

/*!@def P_C_DECL
 * @brief Marks a symbol (variable, function) as c lang.
 * @since 0.0.4
 */

/*!@def P_INTERNAL_API
 * @brief Marks a symbol (variable, function) as local.
 * @since 0.0.4
 *
 * Local symbols are not exported during the linkage and are not available from
 * the outside of the module they are defined in. Use it for internal API.
 *
 * @note Some compilers allow to put this attribute at the beginning of the
 * symbol declaration, and some also at the end of the declaration. Thus it is
 * better to put it in the beginning for more portability.
 */

/*!@def P_GLOBAL_API
 * @brief Marks a symbol (variable, function) as global.
 * @since 0.0.4
 *
 * Global symbols are exported during the linkage and are available from the
 * outside of the module they are defined in. Use it for public API.
 *
 * @note Some compilers allow to put this attribute at the beginning of the
 * symbol declaration, and some also at the end of the declaration. Thus it is
 * better to put it in the beginning for more portability.
 */

/*
 * Oracle Solaris Studio since version 12 has visibility attribute for C
 * compiler, and since version 12.2 for C++ compiler, or since version 8.0
 * specific __global attribute which is the same.
 * IBM XL C has support for visibility attributes since version 13.1.
 * HP C/aC++ has support for visibility attributes since version A.06.15.
 */

#ifdef __cplusplus
# define P_C_DECL extern "C"
#else
# define P_C_DECL
#endif

#if defined(P_CC_MSVC) || defined(P_CC_BORLAND) || defined(P_CC_WATCOM) || \
    defined(P_OS_OS2) || (defined(P_OS_BEOS) && !defined(P_CC_GNU)) || \
    (defined(P_OS_WIN) && defined(P_CC_PGI)) || \
    ((defined(P_OS_WIN) || defined(P_OS_CYGWIN) || defined(P_OS_MSYS)) && \
    defined(P_CC_GNU))
# ifdef PLIBSYS_COMPILE
#   define P_GLOBAL_API P_C_DECL __declspec(dllexport)
# else
#   define P_GLOBAL_API P_C_DECL __declspec(dllimport)
# endif
# define P_INTERNAL_API P_C_DECL
#elif ((__GNUC__ >= 4) && !defined(P_OS_SOLARIS) && !defined(P_OS_HPUX) && \
      !defined(P_OS_AIX)) || \
      (defined(P_CC_SUN) && __SUNPRO_C >= 0x590) || \
      (defined(P_CC_SUN) && __SUNPRO_CC >= 0x5110) || \
      (defined(P_CC_XLC) && __xlC__ >= 0x0D01) || \
      (defined(P_CC_HP) && __HP_aCC >= 0x061500) || \
      (defined(P_CC_HP) && __HP_cc >= 0x061500) || \
      __has_attribute(visibility)
# define P_GLOBAL_API P_C_DECL __attribute__ ((visibility ("default")))
# define P_INTERNAL_API P_C_DECL __attribute__ ((visibility ("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define P_GLOBAL_API P_C_DECL __global
# define P_INTERNAL_API P_C_DECL __hidden
#else
# define P_GLOBAL_API P_C_DECL
# define P_INTERNAL_API P_C_DECL
#endif

/*!@def P_API
 * @brief Exports a symbol from a shared library.
 * @since 0.0.1
 */

#define P_API P_GLOBAL_API

/* Oracle Solaris Studio at least since 12.2 has ((noreturn)) attribute */

/*!@def P_NO_RETURN
 * @brief Notifies a compiler that a function will never return a value (i.e.
 * due to the abort () call).
 * @since 0.0.1
 */

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
# define P_NO_RETURN _Noreturn
#elif defined(P_CC_MSVC) || (defined(P_CC_BORLAND) && __BORLANDC__ >= 0x0550)
# define P_NO_RETURN __declspec(noreturn)
#elif __has_attribute(noreturn) || \
      defined(P_CC_GNU) || \
      (defined(P_CC_SUN) && __SUNPRO_C >= 0x5110) || \
      (defined(P_CC_SUN) && __SUNPRO_CC >= 0x5110)
# define P_NO_RETURN __attribute__((noreturn))
#else
# define P_NO_RETURN
#endif

/*!@def P_LIKELY
 * @brief Hints a compiler that a condition is likely to be true so it can
 * perform code optimizations.
 * @since 0.0.1
 */

/*!@def P_UNLIKELY
 * @brief Hints a compiler that a condition is likely to be false so it can
 * perform code optimizations.
 * @since 0.0.1
 */

#if (defined(P_CC_GNU) && (__GNUC__ > 2 && __GNUC_MINOR__ > 0)) || \
    (defined(P_CC_INTEL) && __INTEL_COMPILER >= 800) || \
    (defined(P_CC_XLC) && __xlC__ >= 0x0900) || \
    __has_builtin(__builtin_expect)
# define P_LIKELY(x) __builtin_expect(!!(x), 1)
# define P_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define P_LIKELY(x) (x)
# define P_UNLIKELY(x) (x)
#endif

/*!@def P_UNUSED
 * @brief Macro to by-pass a compiler warning on unused variables.
 * @since 0.0.1
 */
#define P_UNUSED(a) ((void) (a))

/*!@def P_WARNING
 * @brief Prints a warning message when debugging.
 * @param msg Message to print.
 * @since 0.0.1
 */
#ifndef NDEBUG
# define P_WARNING(msg) fprintf(stderr, "** Warning: %s **\n", msg)
#else
# define P_WARNING(msg)
#endif

/*!@def P_ERROR
 * @brief Prints an error message when debugging.
 * @param msg Message to print.
 * @since 0.0.1
 */
#ifndef NDEBUG
# define P_ERROR(msg) fprintf(stderr, "** Error: %s **\n", msg)
#else
# define P_ERROR(msg)
#endif

/*!@def P_DEBUG
 * @brief Prints a debug message when debugging.
 * @param msg Message to print.
 * @since 0.0.1
 */
#ifndef NDEBUG
# define P_DEBUG(msg) fprintf(stderr, "** Debug: %s **\n", msg)
#else
# define P_DEBUG(msg)
#endif

#ifdef DOXYGEN
# define PLIBSYS_VERSION_MAJOR
# define PLIBSYS_VERSION_MINOR
# define PLIBSYS_VERSION_PATCH
# define PLIBSYS_VERSION_STR
# define PLIBSYS_VERSION
#endif

/*!@def PLIBSYS_VERSION_MAJOR
 * @brief Library major version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/*!@def PLIBSYS_VERSION_MINOR
 * @brief Library minor version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/*!@def PLIBSYS_VERSION_PATCH
 * @brief Library patch version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/*!@def PLIBSYS_VERSION_STR
 * @brief Library full version in the string form, i.e. "0.0.1".
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 * @sa p_libsys_version()
 */

/*!@def PLIBSYS_VERSION
 * @brief Library full version in the form 0xMMNNPP (MM = major, NN = minor,
 * PP = patch), i.e. 0x000001.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 * @sa p_libsys_version()
 */

/*!@def PLIBSYS_VERSION_CHECK
 * @brief Makes a library version number which can be used to check the library
 * version against which the application is compiled.
 * @param major Major version number to check.
 * @param minor Minor version number to check.
 * @param patch Minor version number to check.
 * @since 0.0.1
 * @sa p_libsys_version()
 *
 * @code
 * if (PLIBSYS_VERSION >= PLIBSYS_VERSION_CHECK (0, 0, 1))
 * ...
 * @endcode
 */
#define PLIBSYS_VERSION_CHECK(major, minor, patch) \
  (((major) << 16) | ((minor) << 8) | (patch))

#endif /* !P_MACROS_H__ */
