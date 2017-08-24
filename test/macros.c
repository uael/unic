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
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

static P_GNUC_WARN_UNUSED_RESULT int
unused_result_test_func() {
  return 0;
}

CUTEST(macros, general) {
  int rand_number;
  int unused;
  int result;

  /* Test OS detection macros */
#if !defined (P_OS_DARWIN) && !defined (P_OS_MAC9) && !defined (P_OS_BSD4) && \
    !defined (P_OS_AIX) && !defined (P_OS_HPUX) && !defined (P_OS_SOLARIS) && \
    !defined (P_OS_QNX) && !defined (P_OS_QNX6) && !defined (P_OS_UNIX) && \
    !defined (P_OS_LINUX) && !defined (P_OS_WIN) && !defined (P_OS_CYGWIN) && \
    !defined (P_OS_SCO) && !defined (P_OS_UNIXWARE) && !defined (P_OS_VMS) && \
    !defined (P_OS_IRIX) && !defined (P_OS_MSYS) && !defined (P_OS_DRAGONFLY) && \
    !defined (P_OS_HAIKU) && !defined (P_OS_TRU64) && !defined (P_OS_SYLLABLE) && \
    !defined (P_OS_BEOS) && !defined (P_OS_OS2)
  ASSERT(false);
#endif

  /* Test for Mac OS */
#if defined (P_OS_MAC9) && defined (P_OS_UNIX)
  ASSERT(false);
#endif
#if defined (P_OS_MAC9) && defined (P_OS_MAC)
  ASSERT(false);
#endif
#if defined (P_OS_MAC) && !defined (P_OS_MAC32) && !defined (P_OS_MAC64)
  ASSERT(false);
#endif
#if defined (P_OS_MAC) && (!defined (P_OS_DARWIN) || !defined (P_OS_BSD4))
  ASSERT(false);
#endif
#if defined (P_OS_MAC32) && !defined (P_OS_DARWIN32)
  ASSERT(false);
#endif
#if defined (P_OS_MAC64) && !defined (P_OS_DARWIN64)
  ASSERT(false);
#endif
#if defined (P_OS_MAC32) && defined (P_OS_MAC64)
  ASSERT(false);
#endif

  /* Test for Windows */
#if defined (P_OS_WIN64) && !defined (P_OS_WIN)
  ASSERT(false);
#endif
#if defined (P_OS_WIN) && defined (P_OS_UNIX)
  ASSERT(false);
#endif

  /* Test for FreeBSD */
#if defined (P_OS_FREEBSD) && !defined (P_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for DragonFlyBSD */
#if defined (P_OS_DRAGONFLY) && !defined (P_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for NetBSD */
#if defined (P_OS_NETBSD) && !defined (P_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for OpenBSD */
#if defined (P_OS_OPENBSD) && !defined (P_OS_BSD4)
  ASSERT(false);
#endif

  /* Test for others */
#if defined (P_OS_HAIKU) || defined (P_OS_BEOS) || defined (P_OS_OS2) || defined (P_OS_VMS)
#  if defined (P_OS_UNIX)
  ASSERT(false);
#  endif
#endif

  /* Test for compiler detection macros */
#if !defined (P_CC_MSVC) && !defined (P_CC_GNU) && !defined (P_CC_MINGW) && \
    !defined (P_CC_INTEL) && !defined (P_CC_CLANG) && !defined (P_CC_SUN) && \
    !defined (P_CC_XLC) && !defined (P_CC_HP) && !defined (P_CC_WATCOM) && \
    !defined (P_CC_BORLAND) && !defined (P_CC_MIPS) && !defined (P_CC_USLC) && \
    !defined (P_CC_DEC) && !defined (P_CC_PGI)
  ASSERT(false);
#endif
#if defined (P_CC_MSVC)
#  if !defined (P_OS_WIN)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_INTEL)
#  if !defined (P_OS_WIN)   && !defined (P_OS_MAC)     && \
      !defined (P_OS_LINUX) && !defined (P_OS_FREEBSD) && \
      !defined (P_OS_QNX6)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_SUN)
#  if !defined (P_OS_SOLARIS) && !defined (P_OS_LINUX)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_XLC)
#  if !defined (P_OS_AIX) && !defined (P_OS_LINUX)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_HP)
#  if !defined (P_OS_HPUX)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_WATCOM)
#  if !defined (P_OS_WIN) && !defined (P_OS_LINUX) && \
      !defined (P_OS_OS2) && !defined (P_OS_QNX)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_BORLAND)
#  if !defined (P_OS_WIN)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_MIPS)
#  if !defined (P_OS_IRIX)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_USLC)
#  if !defined (P_OS_SCO) && !defined (P_OS_UNIXWARE)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_DEC)
#  if !defined (P_OS_VMS) && !defined (P_OS_TRU64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_PGI)
#  if !defined (P_OS_WIN) && !defined (P_OS_MAC) && !defined (P_OS_LINUX)
  ASSERT(false);
#  endif
#endif

  /* Test for CPU architecture detection macros */
#if defined (P_OS_VMS)
#  if !defined (P_CPU_ALPHA) && !defined (P_CPU_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_TRU64)
#  if !defined (P_CPU_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_AIX)
#  if !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_HPUX)
#  if !defined (P_CPU_HPPA) && !defined (P_CPU_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_SOLARIS)
#  if !defined (P_CPU_X86) && !defined (P_CPU_SPARC) && !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_QNX)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_QNX6)
#  if !defined (P_CPU_X86)  && !defined (P_CPU_ARM) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_BB10)
#  if !defined(P_CPU_X86) && !defined (P_CPU_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_SCO)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_UNIXWARE)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_IRIX)
#  if !defined (P_CPU_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_HAIKU)
#  if !defined (P_CPU_X86) && !defined (P_CPU_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_SYLLABLE)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_BEOS)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_OS2)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_MAC9)
#  if !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_MAC)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_OS_WIN)
#  if !defined (P_CPU_X86) && !defined (P_CPU_ARM) && !defined (P_CPU_IA64) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER) && !defined (P_CPU_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_MSVC)
#  if !defined (P_CPU_X86)  && !defined (P_CPU_ARM)   && !defined (P_CPU_IA64) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER) && !defined (P_CPU_ALPHA)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_SUN)
#  if !defined (P_CPU_X86) && !defined (P_CPU_SPARC)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_XLC)
#  if !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_HP)
#  if !defined (P_CPU_HPPA) && !defined (P_CPU_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_DEC)
#  if !defined (P_CPU_ALPHA) && !defined (P_CPU_IA64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_MIPS)
#  if !defined (P_CPU_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_USLC)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_WATCOM)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_BORLAND)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_CC_PGI)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM)
#  if !defined (P_CPU_ARM_V2) && !defined (P_CPU_ARM_V3) && !defined (P_CPU_ARM_V4) && \
      !defined (P_CPU_ARM_V5) && !defined (P_CPU_ARM_V6) && !defined (P_CPU_ARM_V7) && \
      !defined (P_CPU_ARM_V8)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V2)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 2)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V3)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 3)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V4)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 4)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V5)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 5)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V6)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 6)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V7)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 7)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_V8)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 8)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM) && !(P_CPU_ARM - 0 > 0)
  ASSERT(false);
#endif
#if defined (P_CPU_ARM)
#  if !defined (P_CPU_ARM_32) && !defined (P_CPU_ARM_64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_32) || defined (P_CPU_ARM_64)
#  if !defined (P_CPU_ARM)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_32)
#  ifdef P_CPU_ARM_64
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_ARM_64)
#  ifdef P_CPU_ARM_32
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_X86)
#  if !((P_CPU_X86 >= 3) && (P_CPU_X86 <= 6))
  ASSERT(false);
#  endif
#  if !defined (P_CPU_X86_32) && !defined (P_CPU_X86_64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_X86_32) || defined (P_CPU_X86_64)
#  if !defined (P_CPU_X86)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_X86_32) && defined (P_CPU_X86_64)
  ASSERT(false);
#endif
#if defined (P_CPU_X86_64)
#  if !defined (P_CPU_X86) || !(P_CPU_X86 - 0 == 6)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS)
#  if !defined (P_CPU_MIPS_I)  && !defined (P_CPU_MIPS_II) && !defined (P_CPU_MIPS_III) && \
      !defined (P_CPU_MIPS_IV) && !defined (P_CPU_MIPS_V)  && !defined (P_CPU_MIPS_32)  && \
      !defined (P_CPU_MIPS_64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_I) || defined (P_CPU_MIPS_II) || defined (P_CPU_MIPS_III) || \
    defined (P_CPU_MIPS_IV) || defined (P_CPU_MIPS_V) || defined (P_CPU_MIPS_32) || \
    defined (P_CPU_MIPS_64)
#  if !defined (P_CPU_MIPS)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_II)
#  if !defined (P_CPU_MIPS_I)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_III)
#  if !defined (P_CPU_MIPS_II)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_IV)
#  if !defined (P_CPU_MIPS_III)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_V)
#  if !defined (P_CPU_MIPS_IV)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_32)
#  if !defined (P_CPU_MIPS_II)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_MIPS_64)
#  if !defined (P_CPU_MIPS_V)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_POWER)
#  if !defined (P_CPU_POWER_32) && !defined (P_CPU_POWER_64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_POWER_32) || defined (P_CPU_POWER_64)
#  if !defined (P_CPU_POWER)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_POWER_32) && defined (P_CPU_POWER_64)
  ASSERT(false);
#endif
#if defined (P_CPU_SPARC_V8) || defined (P_CPU_SPARC_V9)
#  if !defined (P_CPU_SPARC)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_SPARC_V8) && defined (P_CPU_SPARC_V9)
  ASSERT(false);
#endif
#if defined (P_CPU_HPPA)
#  if !defined (P_CPU_HPPA_32) && !defined (P_CPU_HPPA_64)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_HPPA_32) || defined (P_CPU_HPPA_64)
#  if !defined (P_CPU_HPPA)
  ASSERT(false);
#  endif
#endif
#if defined (P_CPU_HPPA_32) && defined (P_CPU_HPPA_64)
  ASSERT(false);
#endif

  /* Test other macros */
  unused = 8;
  P_UNUSED (unused);
  result = unused_result_test_func();
  P_WARNING ("Test warning output");
  P_ERROR ("Test error output");
  P_DEBUG ("Test debug output");
  srand((unsigned int) time(NULL));
  rand_number = rand();
  if (P_LIKELY (rand_number > 0))
    P_DEBUG ("Likely condition triggered");
  if (P_UNLIKELY (rand_number == 0))
    P_DEBUG ("Unlikely condition triggered");

  /* Test version macros */
  ASSERT(PLIBSYS_VERSION_MAJOR >= 0);
  ASSERT(PLIBSYS_VERSION_MINOR >= 0);
  ASSERT(PLIBSYS_VERSION_PATCH >= 0);
  ASSERT(PLIBSYS_VERSION >= 0);
#if !defined (PLIBSYS_VERSION_STR)
  ASSERT(false);
#endif
  ASSERT(PLIBSYS_VERSION >= PLIBSYS_VERSION_CHECK(0, 0, 1));
  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(macros, general);
  return EXIT_SUCCESS;
}
