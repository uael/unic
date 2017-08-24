/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file unic/arch.h
 * @brief CPU detection macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * This family of macros provides CPU detection and defines one or several of
 * U_ARCH_x macros.
 */
#ifndef U_ARCH_H__
# define U_ARCH_H__

/*
 * List of supported CPU architectures (U_ARCH_x):
 *
 * ALPHA           - Alpha
 * ARM             - ARM architecture revision:
 *                   v2, v3, v4, v5, v6, v7, v8
 * ARM_32          - ARM 32-bit
 * ARM_64          - ARM 64-bit
 * ARM_V2          - ARMv2 instruction set
 * ARM_V3          - ARMv3 instruction set
 * ARM_V4          - ARMv4 instruction set
 * ARM_V5          - ARMv5 instruction set
 * ARM_V6          - ARMv6 instruction set
 * ARM_V7          - ARMv7 instruction set
 * ARM_V8          - ARMv8 instruction set
 * X86             - x86 architecture revision:
 *                   3, 4, 5, 6 (Intel P6 or better)
 * X86_32          - x86 32-bit
 * X86_64          - x86 64-bit
 * IA64            - Intel Itanium (IA-64)
 * MIPS            - MIPS
 * MIPS_I          - MIPS I
 * MIPS_II         - MIPS II
 * MIPS_III        - MIPS III
 * MIPS_IV         - MIPS IV
 * MIPS_32         - MIPS32
 * MIPS_64         - MIPS64
 * POWER           - PowerPC
 * POWER_32        - PowerPC 32-bit
 * POWER_64        - PowerPC 64-bit
 * SPARC           - Sparc
 * SPARC_V8        - Sparc V8
 * SPARC_V9        - Sparc V9
 * HPPA            - HPPA-RISC
 * HPPA_32         - HPPA-RISC 32-bit
 * HPPA_64         - HPPA-RISC 64-bit
 */

/*!@def U_ARCH_ALPHA
 * @brief DEC Alpha architecture.
 * @since 0.0.3
 */

/*!@def U_ARCH_ARM
 * @brief ARM architecture.
 * @since 0.0.3
 *
 * This macro is defined for any ARM target. It contains an architecture
 * revision number. One of the revision specific macros (U_ARCH_ARM_Vx) is also
 * defined, as well as #U_ARCH_ARM_32 or #U_ARCH_ARM_64.
 */

/*!@def U_ARCH_ARM_32
 * @brief ARM 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for ARM 32-bit target. One of the revision specific
 * macros (U_ARCH_ARM_Vx) is also defined, as well as #U_ARCH_ARM.
 */

/*!@def U_ARCH_ARM_64
 * @brief ARM 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for ARM 64-bit target. One of the revision specific
 * macros (U_ARCH_ARM_Vx) is also defined, as well as #U_ARCH_ARM.
 */

/*!@def U_ARCH_ARM_V2
 * @brief ARMv2 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv2 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V3
 * @brief ARMv3 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv3 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V4
 * @brief ARMv4 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv4 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V5
 * @brief ARMv5 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv5 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V6
 * @brief ARMv6 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv6 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V7
 * @brief ARMv7 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv7 target. #U_ARCH_ARM_32 and #U_ARCH_ARM macros
 * are also defined.
 */

/*!@def U_ARCH_ARM_V8
 * @brief ARMv8 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv8 target. #U_ARCH_ARM_32 or #U_ARCH_ARM_64 macro
 * is defined, as well as #U_ARCH_ARM.
 */

/*!@def U_ARCH_X86
 * @brief Intel x86 architecture.
 * @since 0.0.3
 *
 * This macro is defined for any x86 target. It contains an architecture
 * revision number (3 for i386 and lower, 4 for i486, 5 for i586, 6 for i686 and
 * better). One of the architecture specific macros (U_ARCH_X86_xx) is also
 * defined.
 */

/*!@def U_ARCH_X86_32
 * @brief Intel x86 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for x86 32-bit target. #U_ARCH_X86 macro is also
 * defined.
 */

/*!@def U_ARCH_X86_64
 * @brief Intel x86 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for x86 64-bit target. #U_ARCH_X86 macro is also
 * defined.
 */

/*!@def U_ARCH_IA64
 * @brief Intel Itanium (IA-64) architecture.
 * @since 0.0.3
 *
 * This macro is defined for Intel Itanium (IA-64) target.
 */

/*!@def U_ARCH_MIPS
 * @brief MIPS architecture.
 * @since 0.0.3
 *
 * This macro is defined for any MIPS target. Some other specific macros
 * (U_ARCH_MIPS_xx) for different MIPS ISAs may be defined.
 */

/*!@def U_ARCH_MIPS_I
 * @brief MIPS I ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS I target. #U_ARCH_MIPS is also defined, as well
 * as probably some other ISA macros (U_ARCH_MIPS_xx).
 */

/*!@def U_ARCH_MIPS_II
 * @brief MIPS II ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS II target. #U_ARCH_MIPS and #U_ARCH_MIPS_I are
 * also defined, as well as probably some other ISA macros (U_ARCH_MIPS_xx).
 */

/*!@def U_ARCH_MIPS_III
 * @brief MIPS III ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS III target. #U_ARCH_MIPS, #U_ARCH_MIPS_I and
 * #U_ARCH_MIPS_II are also defined, as well as probably some other ISA macros
 * (U_ARCH_MIPS_xx).
 */

/*!@def U_ARCH_MIPS_IV
 * @brief MIPS IV ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS IV target. #U_ARCH_MIPS, #U_ARCH_MIPS_I,
 * #U_ARCH_MIPS_II and #U_ARCH_MIPS_III are also defined, as well as probably some
 * other ISA macros (U_ARCH_MIPS_xx).
 */

/*!@def U_ARCH_MIPS_32
 * @brief MIPS32 ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS32 target. #U_ARCH_MIPS, #U_ARCH_MIPS_I and
 * #U_ARCH_MIPS_II.
 */

/*!@def U_ARCH_MIPS_64
 * @brief MIPS64 ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS64 target. #U_ARCH_MIPS, #U_ARCH_MIPS_I,
 * #U_ARCH_MIPS_II, #U_ARCH_MIPS_III, #U_ARCH_MIPS_IV and are also defined.
 */

/*!@def U_ARCH_POWER
 * @brief PowerPC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any PowerPC target. One of the architecture
 * specific macros (U_ARCH_POWER_xx) is also defined.
 */

/*!@def U_ARCH_POWER_32
 * @brief PowerPC 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for PowerPC 32-bit target. #U_ARCH_POWER macro is also
 * defined.
 */

/*!@def U_ARCH_POWER_64
 * @brief PowerPC 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for PowerPC 64-bit target. #U_ARCH_POWER macro is also
 * defined.
 */

/*!@def U_ARCH_SPARC
 * @brief Sun SPARC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any SPARC target. One of the architecture
 * specific macros (U_ARCH_SPARC_xx) is also may be defined.
 */

/*!@def U_ARCH_SPARC_V8
 * @brief Sun SPARC V8 architecture.
 * @since 0.0.3
 *
 * This macro is defined for SPARC V8 target. #U_ARCH_SPARC macro is also
 * defined.
 */

/*!@def U_ARCH_SPARC_V9
 * @brief Sun SPARC V9 architecture.
 * @since 0.0.3
 *
 * This macro is defined for SPARC V9 target. #U_ARCH_SPARC macro is also
 * defined.
 */

/*!@def U_ARCH_HPPA
 * @brief HP PA-RISC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any PA-RISC target. One of the architecture
 * specific macros (U_ARCH_HPPA_xx) is also defined.
 */

/*!@def U_ARCH_HPPA_32
 * @brief HP PA-RISC 32-bit (1.0, 1.1) architecture.
 * @since 0.0.3
 *
 * This macro is defined for PA-RISC 32-bit target. #U_ARCH_HPPA macro is also
 * defined.
 */

/*!@def U_ARCH_HPPA_64
 * @brief HP PA-RISC 64-bit (2.0) architecture.
 * @since 0.0.3
 *
 * This macro is defined for PA-RISC 64-bit target. #U_ARCH_HPPA macro is also
 * defined.
 */

#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
# define U_ARCH_ALPHA
#elif defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) || \
      defined(_M_ARM_) || defined(__arm) || defined(__aarch64__)
# if defined(__aarch64__)
#   define U_ARCH_ARM_64
# else
#   define U_ARCH_ARM_32
# endif
# if defined(__ARM_ARCH) && __ARM_ARCH > 1
#   define U_ARCH_ARM __ARM_ARCH
# elif defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM > 1
#   define U_ARCH_ARM __TARGET_ARCH_ARM
# elif defined(_M_ARM) && _M_ARM > 1
#   define U_ARCH_ARM _M_ARM
# elif defined(__ARM64_ARCH_8__) || defined(__aarch64__) || \
       defined(__CORE_CORTEXAV8__)
#   define U_ARCH_ARM 8
#   define U_ARCH_ARM_V8
# elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || \
       defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || \
       defined(__ARM_ARCH_7S__) || defined(_ARM_ARCH_7) || \
       defined(__CORE_CORTEXA__)
#   define U_ARCH_ARM 7
#   define U_ARCH_ARM_V7
# elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || \
       defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || \
       defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6ZK__) || \
       defined(__ARM_ARCH_6M__)
#   define U_ARCH_ARM 6
#   define U_ARCH_ARM_V6
# elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || \
       defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || \
       defined(__ARM_ARCH_5TEJ__)
#   define U_ARCH_ARM 5
#   define U_ARCH_ARM_V5
# elif defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__)
#   define U_ARCH_ARM 4
#   define U_ARCH_ARM_V4
# elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#   define U_ARCH_ARM 3
#   define U_ARCH_ARM_V3
# elif defined(__ARM_ARCH_2__)
#   define U_ARCH_ARM 2
#   define U_ARCH_ARM_V2
# endif
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
# define U_ARCH_X86_32
# if defined(_M_IX86)
#   if (_M_IX86 <= 600)
#     define U_ARCH_X86 (_M_IX86 / 100)
#   else
#     define U_ARCH_X86 6
#   endif
# elif defined(__i686__) || defined(__athlon__) || defined(__SSE__) || \
       defined(__pentiumpro__)
#   define U_ARCH_X86 6
# elif defined(__i586__) || defined(__k6__) || defined(__pentium__)
#   define U_ARCH_X86 5
# elif defined(__i486__) || defined(__80486__)
#   define U_ARCH_X86 4
# else
#   define U_ARCH_X86 3
# endif
#elif defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || \
      defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
# define U_ARCH_X86_64
# define U_ARCH_X86 6
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
# define U_ARCH_IA64
#elif defined(__mips__) || defined(__mips) || defined(_M_MRX000)
# define U_ARCH_MIPS
# if defined(_M_MRX000)
#   if (_M_MRX000 >= 10000)
#     define U_ARCH_MIPS_IV
#   else
#     define U_ARCH_MIPS_III
#   endif
# endif
# if defined(_MIPS_ARCH_MIPS64) || (defined(__mips) && __mips - 0 >= 64) || \
     (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS64) && \
     __MIPS_ISA - 0 >= _MIPS_ISA_MIPS64)
#   define U_ARCH_MIPS_64
# elif defined(_MIPS_ARCH_MIPS32) || (defined(__mips) && __mips - 0 >= 32) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS32) && \
       __MIPS_ISA - 0 >= _MIPS_ISA_MIPS32)
#   define U_ARCH_MIPS_32
# elif defined(_MIPS_ARCH_MIPS4) || (defined(__mips) && __mips - 0 >= 4) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS4) && \
       __MIPS_ISA - 0 >= _MIPS_ISA_MIPS4)
#   define U_ARCH_MIPS_IV
# elif defined(_MIPS_ARCH_MIPS3) || (defined(__mips) && __mips - 0 >= 3) || \
       (defined(_MIPS_ISA)&& defined(_MIPS_ISA_MIPS3) && \
       __MIPS_ISA - 0 >= _MIPS_ISA_MIPS3)
#   define U_ARCH_MIPS_III
# elif defined(_MIPS_ARCH_MIPS2) || (defined(__mips) && __mips - 0 >= 2) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS2) && \
       __MIPS_ISA - 0 >= _MIPS_ISA_MIPS2)
#   define U_ARCH_MIPS_II
# elif defined(_MIPS_ARCH_MIPS1) || (defined(__mips) && __mips - 0 >= 1) || \
     (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS1) && \
     __MIPS_ISA - 0 >= _MIPS_ISA_MIPS1)
#   define U_ARCH_MIPS_I
# endif
# if defined(U_ARCH_MIPS_64)
#   define U_ARCH_MIPS_IV
# endif
# if defined(U_ARCH_MIPS_IV)
#   define U_ARCH_MIPS_III
# endif
# if defined(U_ARCH_MIPS_32) || defined(U_ARCH_MIPS_III)
#   define U_ARCH_MIPS_II
# endif
# if defined(U_ARCH_MIPS_II)
#   define U_ARCH_MIPS_I
# endif
#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || \
      defined(__ppc) || defined(_ARCH_PPC) || defined(_ARCH_PWR) || \
      defined(_ARCH_COM) || defined(_M_PPC) || defined(_M_MPPC)
# define U_ARCH_POWER
# if defined(__powerpc64__) || defined(__powerpc64) || defined(__ppc64__) || \
     defined(__ppc64) || defined(__64BIT__) || defined(__LP64__) || \
     defined(_LP64)
#   define U_ARCH_POWER_64
# else
#   define U_ARCH_POWER_32
# endif
#elif defined(__sparc__) || defined(__sparc)
# define U_ARCH_SPARC
# if defined(__sparc_v9__) || defined(__sparcv9)
#   define U_ARCH_SPARC_V9
# elif defined(__sparc_v8__) || defined(__sparcv8)
#   define U_ARCH_SPARC_V8
# endif
#elif defined(__hppa__) || defined(__hppa)
# define U_ARCH_HPPA
# if defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || \
     defined(__PA8000__)
#   define U_ARCH_HPPA_64
# else
#   define U_ARCH_HPPA_32
# endif
#endif

/* We need this to generate full Doxygen documentation */

#ifdef DOXYGEN
# ifndef U_ARCH_ALPHA
#   define U_ARCH_ALPHA
# endif
# ifndef U_ARCH_ARM
#   define U_ARCH_ARM
# endif
# ifndef U_ARCH_ARM_32
#   define U_ARCH_ARM_32
# endif
# ifndef U_ARCH_ARM_64
#   define U_ARCH_ARM_64
# endif
# ifndef U_ARCH_ARM_V2
#   define U_ARCH_ARM_V2
# endif
# ifndef U_ARCH_ARM_V3
#   define U_ARCH_ARM_V3
# endif
# ifndef U_ARCH_ARM_V4
#   define U_ARCH_ARM_V4
# endif
# ifndef U_ARCH_ARM_V5
#   define U_ARCH_ARM_V5
# endif
# ifndef U_ARCH_ARM_V6
#   define U_ARCH_ARM_V6
# endif
# ifndef U_ARCH_ARM_V7
#   define U_ARCH_ARM_V7
# endif
# ifndef U_ARCH_ARM_V8
#   define U_ARCH_ARM_V8
# endif
# ifndef U_ARCH_X86
#   define U_ARCH_X86
# endif
# ifndef U_ARCH_X86_32
#   define U_ARCH_X86_32
# endif
# ifndef U_ARCH_X86_64
#   define U_ARCH_X86_64
# endif
# ifndef U_ARCH_IA64
#   define U_ARCH_IA64
# endif
# ifndef U_ARCH_MIPS
#   define U_ARCH_MIPS
# endif
# ifndef U_ARCH_MIPS_I
#   define U_ARCH_MIPS_I
# endif
# ifndef U_ARCH_MIPS_II
#   define U_ARCH_MIPS_II
# endif
# ifndef U_ARCH_MIPS_III
#   define U_ARCH_MIPS_III
# endif
# ifndef U_ARCH_MIPS_IV
#   define U_ARCH_MIPS_IV
# endif
# ifndef U_ARCH_MIPS_32
#   define U_ARCH_MIPS_32
# endif
# ifndef U_ARCH_MIPS_64
#   define U_ARCH_MIPS_64
# endif
# ifndef U_ARCH_POWER
#   define U_ARCH_POWER
# endif
# ifndef U_ARCH_POWER_32
#   define U_ARCH_POWER_32
# endif
# ifndef U_ARCH_POWER_64
#   define U_ARCH_POWER_64
# endif
# ifndef U_ARCH_SPARC
#   define U_ARCH_SPARC
# endif
# ifndef U_ARCH_SPARC_V8
#   define U_ARCH_SPARC_V8
# endif
# ifndef U_ARCH_SPARC_V9
#   define U_ARCH_SPARC_V9
# endif
# ifndef U_ARCH_HPPA
#   define U_ARCH_HPPA
# endif
# ifndef U_ARCH_HPPA_32
#   define U_ARCH_HPPA_32
# endif
# ifndef U_ARCH_HPPA_64
#   define U_ARCH_HPPA_64
# endif
#endif

#endif /* !U_ARCH_H__ */
