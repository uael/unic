/*
 * Copyright (C) 2014-2016 Alexander Saprykin <xelfium@gmail.com>
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

#define BOOST_TEST_MODULE ptypes_test

#include "plib.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (ptypes_general_test) {
  p_libsys_init();

  BOOST_CHECK (P_BYTE_ORDER == P_LITTLE_ENDIAN ||
    P_BYTE_ORDER == P_BIG_ENDIAN);

  BOOST_CHECK (sizeof(int8_t) == 1);
  BOOST_CHECK (sizeof(uint8_t) == 1);
  BOOST_CHECK (sizeof(int16_t) == 2);
  BOOST_CHECK (sizeof(uint16_t) == 2);
  BOOST_CHECK (sizeof(int32_t) == 4);
  BOOST_CHECK (sizeof(uint32_t) == 4);
  BOOST_CHECK (sizeof(int64_t) == 8);
  BOOST_CHECK (sizeof(uint64_t) == 8);
  BOOST_CHECK (sizeof(void *) == sizeof(ptr_t));
  BOOST_CHECK (sizeof(const void *) == sizeof(const_ptr_t));
  BOOST_CHECK (sizeof(int) == sizeof(bool));
  BOOST_CHECK (sizeof(char) == sizeof(byte_t));
  BOOST_CHECK (sizeof(short) == sizeof(short_t));
  BOOST_CHECK (sizeof(int) == sizeof(int_t));
  BOOST_CHECK (sizeof(long) == sizeof(long_t));
  BOOST_CHECK (sizeof(unsigned char) == sizeof(ubyte_t));
  BOOST_CHECK (sizeof(unsigned short) == sizeof(ushort_t));
  BOOST_CHECK (sizeof(unsigned int) == sizeof(uint_t));
  BOOST_CHECK (sizeof(unsigned long) == sizeof(ulong_t));
  BOOST_CHECK (sizeof(float) == sizeof(float_t));
  BOOST_CHECK (sizeof(double) == sizeof(double_t));
  BOOST_CHECK (sizeof(intptr_t) == PLIBSYS_SIZEOF_VOID_P);
  BOOST_CHECK (sizeof(uintptr_t) == PLIBSYS_SIZEOF_VOID_P);
  BOOST_CHECK (sizeof(size_t) == PLIBSYS_SIZEOF_SIZE_T);
  BOOST_CHECK (sizeof(ssize_t) == PLIBSYS_SIZEOF_SIZE_T);
  BOOST_CHECK (sizeof(long_t) == PLIBSYS_SIZEOF_LONG);
  BOOST_CHECK (sizeof(ulong_t) == PLIBSYS_SIZEOF_LONG);
  BOOST_CHECK (sizeof(poffset) == 8);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptypes_pointers_convert_test) {
  p_libsys_init();

  ptr_t pointer = P_INT_TO_POINTER (128);
  BOOST_CHECK (P_POINTER_TO_INT(pointer) == 128);

  int_t pint_val = -64;
  pointer = PINT_TO_POINTER (pint_val);
  BOOST_CHECK (PPOINTER_TO_INT(pointer) == -64);

  uint_t puint_val = 64;
  pointer = PUINT_TO_POINTER (puint_val);
  BOOST_CHECK (PPOINTER_TO_UINT(pointer) == 64);

  size_t psize_val = 1024;
  pointer = PSIZE_TO_POINTER (psize_val);
  BOOST_CHECK (PPOINTER_TO_PSIZE(psize_val) == 1024);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptypes_min_max_test) {
  p_libsys_init();

  BOOST_CHECK (P_MININT8 == (int8_t) 0x80);
  BOOST_CHECK (P_MAXINT8 == (int8_t) 0x7F);
  BOOST_CHECK (P_MAXUINT8 == (uint8_t) 0xFF);
  BOOST_CHECK (P_MININT16 == (int16_t) 0x8000);
  BOOST_CHECK (P_MAXINT16 == (int16_t) 0x7FFF);
  BOOST_CHECK (P_MAXUINT16 == (uint16_t) 0xFFFF);
  BOOST_CHECK (P_MININT32 == (int32_t) 0x80000000);
  BOOST_CHECK (P_MAXINT32 == (int32_t) 0x7FFFFFFF);
  BOOST_CHECK (P_MAXUINT32 == (uint32_t) 0xFFFFFFFF);
  BOOST_CHECK (P_MININT64 == (int64_t) 0x8000000000000000LL);
  BOOST_CHECK (P_MAXINT64 == (int64_t) 0x7FFFFFFFFFFFFFFFLL);
  BOOST_CHECK (P_MAXUINT64 == (uint64_t) 0xFFFFFFFFFFFFFFFFULL);

  if (PLIBSYS_SIZEOF_SIZE_T == 8) {
    BOOST_CHECK (P_MINSSIZE == P_MININT64);
    BOOST_CHECK (P_MAXSSIZE == P_MAXINT64);
    BOOST_CHECK (P_MAXSIZE == P_MAXUINT64);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      BOOST_CHECK (P_MINSSIZE == P_MINLONG);
      BOOST_CHECK (P_MAXSSIZE == P_MAXLONG);
      BOOST_CHECK (P_MAXSIZE == P_MAXULONG);
    }
  } else {
    BOOST_CHECK (P_MINSSIZE == P_MININT32);
    BOOST_CHECK (P_MAXSSIZE == P_MAXINT32);
    BOOST_CHECK (P_MAXSIZE == P_MAXUINT32);

    if (PLIBSYS_SIZEOF_LONG == 4) {
      BOOST_CHECK (P_MINSSIZE == P_MINLONG);
      BOOST_CHECK (P_MAXSSIZE == P_MAXLONG);
      BOOST_CHECK (P_MAXSIZE == P_MAXULONG);
    }
  }

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptypes_modifiers_test) {
  p_libsys_init();

  size_t size_val = 256;
  printf("%#" PSIZE_MODIFIER "x\n", size_val);
  ssize_t ssize_val = -256;
  printf("%#" PSIZE_MODIFIER "x\n", ssize_val);

  uintptr_t puintptr_val = 512;
  printf("%#" PINTPTR_MODIFIER "x\n", puintptr_val);
  intptr_t pintptr_val = -512;
  printf("%#" PINTPTR_MODIFIER "x\n", pintptr_val);

  uint16_t puint16_val = 1024;
  printf("%#" PINT16_MODIFIER "x\n", puint16_val);
  int16_t pint16_val = -1024;
  printf("%#" PINT16_MODIFIER "x\n", pint16_val);

  uint32_t puint32_val = 2048;
  printf("%#" PINT32_MODIFIER "x\n", puint32_val);
  int32_t pint32_val = -2048;
  printf("%#" PINT32_MODIFIER "x\n", pint32_val);

  uint64_t puint64_val = 4096;
  printf("%#" PINT64_MODIFIER "x\n", puint64_val);
  int64_t pint64_val = -4096;
  printf("%#" PINT64_MODIFIER "x\n", pint64_val);

  poffset poffset_val = 8192;
  printf("%#" POFFSET_MODIFIER "x\n", poffset_val);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptypes_formats_test) {
  p_libsys_init();

  ssize_t ssize_val = -256;
  printf("%" PSSIZE_FORMAT "\n", ssize_val);
  size_t size_val = 256;
  printf("%" PSIZE_FORMAT "\n", size_val);

  uintptr_t puintptr_val = 512;
  printf("%" PUINTPTR_FORMAT "\n", puintptr_val);
  intptr_t pintptr_val = -512;
  printf("%" PINTPTR_FORMAT "\n", pintptr_val);

  uint16_t puint16_val = 1024;
  printf("%" PUINT16_FORMAT "\n", puint16_val);
  int16_t pint16_val = -1024;
  printf("%" PINT16_FORMAT "\n", pint16_val);

  uint32_t puint32_val = 2048;
  printf("%" PUINT32_FORMAT "\n", puint32_val);
  int32_t pint32_val = -2048;
  printf("%" PINT32_FORMAT "\n", pint32_val);

  uint64_t puint64_val = 4096;
  printf("%" PUINT64_FORMAT "\n", puint64_val);
  int64_t pint64_val = -4096;
  printf("%" PINT64_FORMAT "\n", pint64_val);

  poffset poffset_val = 8192;
  printf("%" POFFSET_FORMAT "\n", poffset_val);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_CASE (ptypes_host_network_test) {
  p_libsys_init();

  if (P_BYTE_ORDER == P_LITTLE_ENDIAN) {
    int16_t pint16_val = PINT16_TO_BE (0xFFE0);
    BOOST_CHECK (pint16_val == (int16_t) 0xE0FF);
    BOOST_CHECK (PINT16_FROM_BE(pint16_val) == (int16_t) 0xFFE0);
    BOOST_CHECK (PINT16_TO_LE(pint16_val) == (int16_t) 0xE0FF);
    BOOST_CHECK (PINT16_FROM_LE(pint16_val) == (int16_t) 0xE0FF);

    uint16_t puint16_val = PUINT16_TO_BE (0x0020);
    BOOST_CHECK (puint16_val == (uint16_t) 0x2000);
    BOOST_CHECK (PUINT16_FROM_BE(puint16_val) == (uint16_t) 0x0020);
    BOOST_CHECK (PUINT16_TO_LE(puint16_val) == (uint16_t) 0x2000);
    BOOST_CHECK (PUINT16_FROM_LE(puint16_val) == (uint16_t) 0x2000);

    int32_t pint32_val = PINT32_TO_BE (0xFFFFFFC0);
    BOOST_CHECK (pint32_val == (int32_t) 0xC0FFFFFF);
    BOOST_CHECK (PINT32_FROM_BE(pint32_val) == (int32_t) 0xFFFFFFC0);
    BOOST_CHECK (PINT32_TO_LE(pint32_val) == (int32_t) 0xC0FFFFFF);
    BOOST_CHECK (PINT32_FROM_LE(pint32_val) == (int32_t) 0xC0FFFFFF);

    uint32_t puint32_val = PUINT32_TO_BE (0x00000040);
    BOOST_CHECK (puint32_val == (uint32_t) 0x40000000);
    BOOST_CHECK (PUINT32_FROM_BE(puint32_val) == (uint32_t) 0x00000040);
    BOOST_CHECK (PUINT32_TO_LE(puint32_val) == (uint32_t) 0x40000000);
    BOOST_CHECK (PUINT32_FROM_LE(puint32_val) == (uint32_t) 0x40000000);

    int64_t pint64_val = PINT64_TO_BE (0xFFFFFFFFFFFFFF80LL);
    BOOST_CHECK (pint64_val == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    BOOST_CHECK (PINT64_FROM_BE(pint64_val) == (int64_t) 0xFFFFFFFFFFFFFF80LL);
    BOOST_CHECK (PINT64_TO_LE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    BOOST_CHECK (PINT64_FROM_LE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);

    uint64_t puint64_val = PUINT64_TO_BE (0x0000000000000080ULL);
    BOOST_CHECK (puint64_val == (uint64_t) 0x8000000000000000ULL);
    BOOST_CHECK (
      PUINT64_FROM_BE(puint64_val) == (uint64_t) 0x0000000000000080ULL);
    BOOST_CHECK (
      PUINT64_TO_LE(puint64_val) == (uint64_t) 0x8000000000000000ULL);
    BOOST_CHECK (
      PUINT64_FROM_LE(puint64_val) == (uint64_t) 0x8000000000000000ULL);

    int_t pint_val = PINT_TO_BE (0xFFFFFC00);
    BOOST_CHECK (pint_val == (int_t) 0x00FCFFFF);
    BOOST_CHECK (PINT_FROM_BE(pint_val) == (int_t) 0xFFFFFC00);
    BOOST_CHECK (PINT_TO_LE(pint_val) == (int_t) 0x00FCFFFF);
    BOOST_CHECK (PINT_FROM_LE(pint_val) == (int_t) 0x00FCFFFF);

    uint_t puint_val = PUINT_TO_BE (0x00000400);
    BOOST_CHECK (puint_val == (uint_t) 0x00040000);
    BOOST_CHECK (PUINT_FROM_BE(puint_val) == (uint_t) 0x00000400);
    BOOST_CHECK (PUINT_TO_LE(puint_val) == (uint_t) 0x00040000);
    BOOST_CHECK (PUINT_FROM_LE(puint_val) == (uint_t) 0x00040000);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      long_t plong_val = PLONG_TO_BE (0xFFFFFFFFFFFFF800LL);
      BOOST_CHECK (plong_val == (long_t) 0x00F8FFFFFFFFFFFFLL);
      BOOST_CHECK (PLONG_FROM_BE(plong_val) == (long_t) 0xFFFFFFFFFFFFF800LL);
      BOOST_CHECK (PLONG_TO_LE(plong_val) == (long_t) 0x00F8FFFFFFFFFFFFLL);
      BOOST_CHECK (PLONG_FROM_LE(plong_val) == (long_t) 0x00F8FFFFFFFFFFFFLL);

      ulong_t pulong_val = PULONG_TO_BE (0x0000000000000800ULL);
      BOOST_CHECK (pulong_val == (ulong_t) 0x0008000000000000ULL);
      BOOST_CHECK (
        PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      BOOST_CHECK (PULONG_TO_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      BOOST_CHECK (
        PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      long_t plong_val = PLONG_TO_BE (0xFFFFF800);
      BOOST_CHECK (plong_val == (long_t) 0x00F8FFFF);
      BOOST_CHECK (PLONG_FROM_BE(plong_val) == (long_t) 0xFFFFF800);
      BOOST_CHECK (PLONG_TO_LE(plong_val) == (long_t) 0x00F8FFFF);
      BOOST_CHECK (PLONG_FROM_LE(plong_val) == (long_t) 0x00F8FFFF);

      ulong_t pulong_val = PULONG_TO_BE (0x00000800);
      BOOST_CHECK (pulong_val == (ulong_t) 0x00080000);
      BOOST_CHECK (PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00000800);
      BOOST_CHECK (PULONG_TO_LE(pulong_val) == (ulong_t) 0x00080000);
      BOOST_CHECK (PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00080000);
    }

    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      size_t psize_val = PSIZE_TO_BE (0x0000000000001000ULL);
      BOOST_CHECK (psize_val == (size_t) 0x0010000000000000ULL);
      BOOST_CHECK (PSIZE_FROM_BE(psize_val) == (size_t) 0x0000000000001000ULL);
      BOOST_CHECK (PSIZE_TO_LE(psize_val) == (size_t) 0x0010000000000000ULL);
      BOOST_CHECK (PSIZE_FROM_LE(psize_val) == (size_t) 0x0010000000000000ULL);

      ssize_t pssize_val = PSSIZE_TO_BE (0x000000000000F000LL);
      BOOST_CHECK (pssize_val == (ssize_t) 0x00F0000000000000LL);
      BOOST_CHECK (
        PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      BOOST_CHECK (PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      BOOST_CHECK (
        PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      size_t psize_val = PSIZE_TO_BE (0x00001000);
      BOOST_CHECK (psize_val == (size_t) 0x00100000);
      BOOST_CHECK (PSIZE_FROM_BE(psize_val) == (size_t) 0x00001000);
      BOOST_CHECK (PSIZE_TO_LE(psize_val) == (size_t) 0x00100000);
      BOOST_CHECK (PSIZE_FROM_LE(psize_val) == (size_t) 0x00100000);

      ssize_t pssize_val = PSSIZE_TO_BE (0x0000F000);
      BOOST_CHECK (pssize_val == (ssize_t) 0x00F00000);
      BOOST_CHECK (PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x0000F000);
      BOOST_CHECK (PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F00000);
      BOOST_CHECK (PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F00000);
    }

    puint16_val = p_htons (0x0020);
    BOOST_CHECK (puint16_val == (uint16_t) 0x2000);
    BOOST_CHECK (p_ntohs(puint16_val) == (uint16_t) 0x0020);

    puint32_val = p_htonl (0x00000040);
    BOOST_CHECK (puint32_val == (uint32_t) 0x40000000);
    BOOST_CHECK (p_ntohl(puint32_val) == (uint32_t) 0x00000040);
  } else {
    int16_t pint16_val = PINT16_TO_LE (0xFFE0);
    BOOST_CHECK (pint16_val == (int16_t) 0xE0FF);
    BOOST_CHECK (PINT16_FROM_LE(pint16_val) == (int16_t) 0xFFE0);
    BOOST_CHECK (PINT16_TO_BE(pint16_val) == (int16_t) 0xE0FF);
    BOOST_CHECK (PINT16_FROM_BE(pint16_val) == (int16_t) 0xE0FF);

    uint16_t puint16_val = PUINT16_TO_LE (0x0020);
    BOOST_CHECK (puint16_val == (uint16_t) 0x2000);
    BOOST_CHECK (PUINT16_FROM_LE(puint16_val) == (uint16_t) 0x0020);
    BOOST_CHECK (PUINT16_TO_BE(puint16_val) == (uint16_t) 0x2000);
    BOOST_CHECK (PUINT16_FROM_BE(puint16_val) == (uint16_t) 0x2000);

    int32_t pint32_val = PINT32_TO_LE (0xFFFFFFC0);
    BOOST_CHECK (pint32_val == (int32_t) 0xC0FFFFFF);
    BOOST_CHECK (PINT32_FROM_LE(pint32_val) == (int32_t) 0xFFFFFFC0);
    BOOST_CHECK (PINT32_TO_BE(pint32_val) == (int32_t) 0xC0FFFFFF);
    BOOST_CHECK (PINT32_FROM_BE(pint32_val) == (int32_t) 0xC0FFFFFF);

    uint32_t puint32_val = PUINT32_TO_LE (0x00000040);
    BOOST_CHECK (puint32_val == (uint32_t) 0x40000000);
    BOOST_CHECK (PUINT32_FROM_LE(puint32_val) == (uint32_t) 0x00000040);
    BOOST_CHECK (PUINT32_TO_BE(puint32_val) == (uint32_t) 0x40000000);
    BOOST_CHECK (PUINT32_FROM_BE(puint32_val) == (uint32_t) 0x40000000);

    int64_t pint64_val = PINT64_TO_LE (0xFFFFFFFFFFFFFF80LL);
    BOOST_CHECK (pint64_val == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    BOOST_CHECK (PINT64_FROM_LE(pint64_val) == (int64_t) 0xFFFFFFFFFFFFFF80LL);
    BOOST_CHECK (PINT64_TO_BE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    BOOST_CHECK (PINT64_FROM_BE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);

    uint64_t puint64_val = PUINT64_TO_LE (0x0000000000000080ULL);
    BOOST_CHECK (puint64_val == (uint64_t) 0x8000000000000000ULL);
    BOOST_CHECK (
      PUINT64_FROM_LE(puint64_val) == (uint64_t) 0x0000000000000080ULL);
    BOOST_CHECK (
      PUINT64_TO_BE(puint64_val) == (uint64_t) 0x8000000000000000ULL);
    BOOST_CHECK (
      PUINT64_FROM_BE(puint64_val) == (uint64_t) 0x8000000000000000ULL);

    int_t pint_val = PINT_TO_LE (0xFFFFFC00);
    BOOST_CHECK (pint_val == (int_t) 0x00FCFFFF);
    BOOST_CHECK (PINT_FROM_LE(pint_val) == (int_t) 0xFFFFFC00);
    BOOST_CHECK (PINT_TO_BE(pint_val) == (int_t) 0x00FCFFFF);
    BOOST_CHECK (PINT_FROM_BE(pint_val) == (int_t) 0x00FCFFFF);

    uint_t puint_val = PUINT_TO_LE (0x00000400);
    BOOST_CHECK (puint_val == (uint_t) 0x00040000);
    BOOST_CHECK (PUINT_FROM_LE(puint_val) == (uint_t) 0x00000400);
    BOOST_CHECK (PUINT_TO_BE(puint_val) == (uint_t) 0x00040000);
    BOOST_CHECK (PUINT_FROM_BE(puint_val) == (uint_t) 0x00040000);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      long_t plong_val = PLONG_TO_LE (0xFFFFFFFFFFFFF800LL);
      BOOST_CHECK (plong_val == (long_t) 0x00F8FFFFFFFFFFFFLL);
      BOOST_CHECK (PLONG_FROM_LE(plong_val) == (long_t) 0xFFFFFFFFFFFFF800LL);
      BOOST_CHECK (PLONG_TO_BE(plong_val) == (long_t) 0x00F8FFFFFFFFFFFFLL);
      BOOST_CHECK (PLONG_FROM_BE(plong_val) == (long_t) 0x00F8FFFFFFFFFFFFLL);

      ulong_t pulong_val = PULONG_TO_LE (0x0000000000000800ULL);
      BOOST_CHECK (pulong_val == (ulong_t) 0x0008000000000000ULL);
      BOOST_CHECK (
        PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      BOOST_CHECK (PULONG_TO_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      BOOST_CHECK (
        PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      long_t plong_val = PLONG_TO_LE (0xFFFFF800);
      BOOST_CHECK (plong_val == (long_t) 0x00F8FFFF);
      BOOST_CHECK (PLONG_FROM_LE(plong_val) == (long_t) 0xFFFFF800);
      BOOST_CHECK (PLONG_TO_BE(plong_val) == (long_t) 0x00F8FFFF);
      BOOST_CHECK (PLONG_FROM_BE(plong_val) == (long_t) 0x00F8FFFF);

      ulong_t pulong_val = PULONG_TO_LE (0x00000800);
      BOOST_CHECK (pulong_val == (ulong_t) 0x00080000);
      BOOST_CHECK (PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00000800);
      BOOST_CHECK (PULONG_TO_BE(pulong_val) == (ulong_t) 0x00080000);
      BOOST_CHECK (PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00080000);
    }

    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      size_t psize_val = PSIZE_TO_LE (0x0000000000001000ULL);
      BOOST_CHECK (psize_val == (size_t) 0x0010000000000000ULL);
      BOOST_CHECK (PSIZE_FROM_LE(psize_val) == (size_t) 0x0000000000001000ULL);
      BOOST_CHECK (PSIZE_TO_BE(psize_val) == (size_t) 0x0010000000000000ULL);
      BOOST_CHECK (PSIZE_FROM_BE(psize_val) == (size_t) 0x0010000000000000ULL);

      ssize_t pssize_val = PSSIZE_TO_LE (0x000000000000F000LL);
      BOOST_CHECK (pssize_val == (ssize_t) 0x00F0000000000000LL);
      BOOST_CHECK (
        PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      BOOST_CHECK (PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      BOOST_CHECK (
        PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      size_t psize_val = PSIZE_TO_LE (0x00001000);
      BOOST_CHECK (psize_val == (size_t) 0x00100000);
      BOOST_CHECK (PSIZE_FROM_LE(psize_val) == (size_t) 0x00001000);
      BOOST_CHECK (PSIZE_TO_BE(psize_val) == (size_t) 0x00100000);
      BOOST_CHECK (PSIZE_FROM_BE(psize_val) == (size_t) 0x00100000);

      ssize_t pssize_val = PSSIZE_TO_LE (0x0000F000);
      BOOST_CHECK (pssize_val == (ssize_t) 0x00F00000);
      BOOST_CHECK (PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x0000F000);
      BOOST_CHECK (PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F00000);
      BOOST_CHECK (PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F00000);
    }

    puint16_val = p_htons (0x0020);
    BOOST_CHECK (puint16_val == (uint16_t) 0x0020);
    BOOST_CHECK (p_ntohs(puint16_val) == (uint16_t) 0x0020);

    puint32_val = p_htonl (0x00000040);
    BOOST_CHECK (puint32_val == (uint32_t) 0x00000040);
    BOOST_CHECK (p_ntohl(puint32_val) == (uint32_t) 0x00000040);
  }

  uint16_t puint16_val = PUINT16_SWAP_BYTES (0x0020);
  BOOST_CHECK (puint16_val == (uint16_t) 0x2000);
  BOOST_CHECK (PUINT16_SWAP_BYTES(puint16_val) == (uint16_t) 0x0020);

  uint32_t puint32_val = PUINT32_SWAP_BYTES (0x00000040);
  BOOST_CHECK (puint32_val == (uint32_t) 0x40000000);
  BOOST_CHECK (PUINT32_SWAP_BYTES(puint32_val) == (uint32_t) 0x00000040);

  uint64_t puint64_val = PUINT64_SWAP_BYTES (0x0000000000000080ULL);
  BOOST_CHECK (puint64_val == (uint64_t) 0x8000000000000000ULL);
  BOOST_CHECK (
    PUINT64_SWAP_BYTES(puint64_val) == (uint64_t) 0x0000000000000080ULL);

  p_libsys_shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
