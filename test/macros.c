/*
 * Copyright (C) 2014-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include <time.h>
#include "cute.h"
#include "unic.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { u_libsys_init(); }

CUTEST_TEARDOWN { u_libsys_shutdown(); }

static U_GNUC_WARN_UNUSED_RESULT int
unused_result_test_func() {
  return 0;
}

CUTEST(macros, general) {
  int rand_number;
  int unused;
  int result;

  /* Test OS detection macros */
#if !defined (U_OS_DARWIN) && !defined (U_OS_MAC9) && !defined (U_OS_BSD4) && \
    !defined (U_OS_AIX) && !defined (U_OS_HPUX) && !defined (U_OS_SOLARIS) && \
    !defined (U_OS_QNX) && !defined (U_OS_QNX6) && !defined (U_OS_UNIX) && \
    !defined (U_OS_LINUX) && !defined (U_OS_WIN) && !defined (U_OS_CYGWIN) && \
    !defined (U_OS_SCO) && !defined (U_OS_UNIXWARE) && !defined (U_OS_VMS) && \
    !defined (U_OS_IRIX) && !defined (U_OS_MSYS) && !defined (U_OS_DRAGONFLY) && \
    !defined (U_OS_HAIKU) && !defined (U_OS_TRU64) && !defined (U_OS_SYLLABLE) && \
    !defined (U_OS_BEOS) && !defined (U_OS_OS2)
  ASSERT(false);
#endif

  /* Test for Mac OS */
#if defined (U_OS_MAC9) && defined (U_OS_UNIX)
  ASSERT(false);
#endif
#if defined (U_OS_MAC9) && defined (U_OS_MAC)
  ASSERT(false);
#endif
#if defined (U_OS_MAC) && !defined (U_OS_MAC32) && !defined (U_OS_MAC64)
  ASSERT(false);
#endif
#if defined (U_OS_MAC) && (!defined (U_OS_DARWIN) || !defined (U_OS_BSD4))
  ASSERT(false);
#endif
#if defined (U_OS_MAC32) && !defined (U_OS_DARWIN32)
  ASSERT(false);
#endif
#if defined (U_OS_MAC64) && !defined (U_OS_DARWIN64)
  ASSERT(false);
#endif
#if defined (U_OS_MAC32) && defined (U_OS_MAC64)
  ASSERT(false);
#endif

  /* Test for Windows */
#if defined (U_OS_WIN64) && !defined (U_OS_WIN)
  ASSERT(false);
#endif
#if defined (U_OS_WIN) && defined (U_OS_UNIX)
  ASSERT(false);
#endif

  /* Test for FreeBSD */
#if defined (U_OS_FREEBSD) && !defined (U_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for DragonFlyBSD */
#if defined (U_OS_DRAGONFLY) && !defined (U_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for NetBSD */
#if defined (U_OS_NETBSD) && !defined (U_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for OpenBSD */
#if defined (U_OS_OPENBSD) && !defined (U_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for others */
#if defined (U_OS_HAIKU) || defined (U_OS_BEOS) || defined (U_OS_OS2) || defined (U_OS_VMS)
#  if defined (U_OS_UNIX)
  ASSERT(false);
#  endif
#endif

  /* Test for compiler detection macros */
#if !defined (U_CC_MSVC) && !defined (U_CC_GNU) && !defined (U_CC_MINGW) && \
    !defined (U_CC_INTEL) && !defined (U_CC_CLANG) && !defined (U_CC_SUN) && \
    !defined (U_CC_XLC) && !defined (U_CC_HP) && !defined (U_CC_WATCOM) && \
    !defined (U_CC_BORLAND) && !defined (U_CC_MIPS) && !defined (U_CC_USLC) && \
    !defined (U_CC_DEC) && !defined (U_CC_PGI)
  ASSERT(false);
#endif
#if defined (U_CC_MSVC)
#  if !defined (U_OS_WIN)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_INTEL)
#  if !defined (U_OS_WIN)   && !defined (U_OS_MAC)     && \
      !defined (U_OS_LINUX) && !defined (U_OS_FREEBSD) && \
      !defined (U_OS_QNX6)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_SUN)
#  if !defined (U_OS_SOLARIS) && !defined (U_OS_LINUX)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_XLC)
#  if !defined (U_OS_AIX) && !defined (U_OS_LINUX)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_HP)
#  if !defined (U_OS_HPUX)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_WATCOM)
#  if !defined (U_OS_WIN) && !defined (U_OS_LINUX) && \
      !defined (U_OS_OS2) && !defined (U_OS_QNX)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_BORLAND)
#  if !defined (U_OS_WIN)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_MIPS)
#  if !defined (U_OS_IRIX)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_USLC)
#  if !defined (U_OS_SCO) && !defined (U_OS_UNIXWARE)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_DEC)
#  if !defined (U_OS_VMS) && !defined (U_OS_TRU64)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_PGI)
#  if !defined (U_OS_WIN) && !defined (U_OS_MAC) && !defined (U_OS_LINUX)
  ASSERT(false);
#  endif
#endif

  /* Test for CPU architecture detection macros */
#if defined (U_OS_VMS)
#  if !defined (U_ARCH_ALPHA) && !defined (U_ARCH_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_TRU64)
#  if !defined (U_ARCH_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_AIX)
#  if !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_HPUX)
#  if !defined (U_ARCH_HPPA) && !defined (U_ARCH_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_SOLARIS)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_SPARC) && !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_QNX)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_QNX6)
#  if !defined (U_ARCH_X86)  && !defined (U_ARCH_ARM) && \
      !defined (U_ARCH_MIPS) && !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_BB10)
#  if !defined(U_ARCH_X86) && !defined (U_ARCH_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_SCO)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_UNIXWARE)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_IRIX)
#  if !defined (U_ARCH_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_HAIKU)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_SYLLABLE)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_BEOS)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_OS2)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_MAC9)
#  if !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_MAC)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_OS_WIN)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_ARM) && !defined (U_ARCH_IA64) && \
      !defined (U_ARCH_MIPS) && !defined (U_ARCH_POWER) && !defined (U_ARCH_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_MSVC)
#  if !defined (U_ARCH_X86)  && !defined (U_ARCH_ARM)   && !defined (U_ARCH_IA64) && \
      !defined (U_ARCH_MIPS) && !defined (U_ARCH_POWER) && !defined (U_ARCH_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_SUN)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_SPARC)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_XLC)
#  if !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_HP)
#  if !defined (U_ARCH_HPPA) && !defined (U_ARCH_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_DEC)
#  if !defined (U_ARCH_ALPHA) && !defined (U_ARCH_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_MIPS)
#  if !defined (U_ARCH_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_USLC)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_WATCOM)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_BORLAND)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_CC_PGI)
#  if !defined (U_ARCH_X86) && !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM)
#  if !defined (U_ARCH_ARM_V2) && !defined (U_ARCH_ARM_V3) && !defined (U_ARCH_ARM_V4) && \
      !defined (U_ARCH_ARM_V5) && !defined (U_ARCH_ARM_V6) && !defined (U_ARCH_ARM_V7) && \
      !defined (U_ARCH_ARM_V8)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V2)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 2)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V3)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 3)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V4)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 4)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V5)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 5)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V6)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 6)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V7)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 7)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_V8)
#  if !defined (U_ARCH_ARM) || !(U_ARCH_ARM - 0 == 8)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM) && !(U_ARCH_ARM - 0 > 0)
  ASSERT(false);
#endif
#if defined (U_ARCH_ARM)
#  if !defined (U_ARCH_ARM_32) && !defined (U_ARCH_ARM_64)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_32) || defined (U_ARCH_ARM_64)
#  if !defined (U_ARCH_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_32)
#  ifdef U_ARCH_ARM_64
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_ARM_64)
#  ifdef U_ARCH_ARM_32
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_X86)
#  if !((U_ARCH_X86 >= 3) && (U_ARCH_X86 <= 6))
  ASSERT(false);
#  endif
#  if !defined (U_ARCH_X86_32) && !defined (U_ARCH_X86_64)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_X86_32) || defined (U_ARCH_X86_64)
#  if !defined (U_ARCH_X86)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_X86_32) && defined (U_ARCH_X86_64)
  ASSERT(false);
#endif
#if defined (U_ARCH_X86_64)
#  if !defined (U_ARCH_X86) || !(U_ARCH_X86 - 0 == 6)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS)
#  if !defined (U_ARCH_MIPS_I)  && !defined (U_ARCH_MIPS_II) && !defined (U_ARCH_MIPS_III) && \
      !defined (U_ARCH_MIPS_IV) && !defined (U_ARCH_MIPS_V)  && !defined (U_ARCH_MIPS_32)  && \
      !defined (U_ARCH_MIPS_64)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_I) || defined (U_ARCH_MIPS_II) || defined (U_ARCH_MIPS_III) || \
    defined (U_ARCH_MIPS_IV) || defined (U_ARCH_MIPS_V) || defined (U_ARCH_MIPS_32) || \
    defined (U_ARCH_MIPS_64)
#  if !defined (U_ARCH_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_II)
#  if !defined (U_ARCH_MIPS_I)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_III)
#  if !defined (U_ARCH_MIPS_II)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_IV)
#  if !defined (U_ARCH_MIPS_III)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_V)
#  if !defined (U_ARCH_MIPS_IV)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_32)
#  if !defined (U_ARCH_MIPS_II)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_MIPS_64)
#  if !defined (U_ARCH_MIPS_V)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_POWER)
#  if !defined (U_ARCH_POWER_32) && !defined (U_ARCH_POWER_64)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_POWER_32) || defined (U_ARCH_POWER_64)
#  if !defined (U_ARCH_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_POWER_32) && defined (U_ARCH_POWER_64)
  ASSERT(false);
#endif
#if defined (U_ARCH_SPARC_V8) || defined (U_ARCH_SPARC_V9)
#  if !defined (U_ARCH_SPARC)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_SPARC_V8) && defined (U_ARCH_SPARC_V9)
  ASSERT(false);
#endif
#if defined (U_ARCH_HPPA)
#  if !defined (U_ARCH_HPPA_32) && !defined (U_ARCH_HPPA_64)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_HPPA_32) || defined (U_ARCH_HPPA_64)
#  if !defined (U_ARCH_HPPA)
  ASSERT(false);
#  endif
#endif
#if defined (U_ARCH_HPPA_32) && defined (U_ARCH_HPPA_64)
  ASSERT(false);
#endif

  /* Test other macros */
  unused = 8;
  U_UNUSED (unused);
  result = unused_result_test_func();
  U_WARNING ("Test warning output");
  U_ERROR ("Test error output");
  U_DEBUG ("Test debug output");
  srand((unsigned int) time(NULL));
  rand_number = rand();
  if (U_LIKELY (rand_number > 0))
    U_DEBUG ("Likely condition triggered");
  if (U_UNLIKELY (rand_number == 0))
    U_DEBUG ("Unlikely condition triggered");

  /* Test version macros */
  ASSERT(UNIC_VERSION_MAJOR >= 0);
  ASSERT(UNIC_VERSION_MINOR >= 0);
  ASSERT(UNIC_VERSION_PATCH >= 0);
  ASSERT(UNIC_VERSION >= 0);
#if !defined (UNIC_VERSION_STR)
  ASSERT(false);
#endif
  ASSERT(UNIC_VERSION >= UNIC_VERSION_CHECK(0, 0, 1));
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(macros, general);
  return EXIT_SUCCESS;
}
