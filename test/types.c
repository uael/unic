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

#include "cute.h"
#include "plib.h"

CUTEST_DATA {
  int dummy;
};

CUTEST_SETUP { p_libsys_init(); }

CUTEST_TEARDOWN { p_libsys_shutdown(); }

CUTEST(types, general) {
  ASSERT(P_BYTE_ORDER == P_LITTLE_ENDIAN || P_BYTE_ORDER == P_BIG_ENDIAN);
  ASSERT(sizeof(int8_t) == 1);
  ASSERT(sizeof(uint8_t) == 1);
  ASSERT(sizeof(int16_t) == 2);
  ASSERT(sizeof(uint16_t) == 2);
  ASSERT(sizeof(int32_t) == 4);
  ASSERT(sizeof(uint32_t) == 4);
  ASSERT(sizeof(int64_t) == 8);
  ASSERT(sizeof(uint64_t) == 8);
  ASSERT(sizeof(void *) == sizeof(ptr_t));
  ASSERT(sizeof(const void *) == sizeof(const_ptr_t));
  ASSERT(sizeof(unsigned char) == sizeof(bool));
  ASSERT(sizeof(char) == sizeof(byte_t));
  ASSERT(sizeof(short) == sizeof(short));
  ASSERT(sizeof(int) == sizeof(int));
  ASSERT(sizeof(long) == sizeof(long));
  ASSERT(sizeof(unsigned char) == sizeof(ubyte_t));
  ASSERT(sizeof(unsigned short) == sizeof(ushort_t));
  ASSERT(sizeof(unsigned int) == sizeof(uint_t));
  ASSERT(sizeof(unsigned long) == sizeof(ulong_t));
  ASSERT(sizeof(float) == sizeof(float));
  ASSERT(sizeof(double) == sizeof(double));
  ASSERT(sizeof(intptr_t) == PLIBSYS_SIZEOF_VOID_P);
  ASSERT(sizeof(uintptr_t) == PLIBSYS_SIZEOF_VOID_P);
  ASSERT(sizeof(size_t) == PLIBSYS_SIZEOF_SIZE_T);
  ASSERT(sizeof(ssize_t) == PLIBSYS_SIZEOF_SIZE_T);
  ASSERT(sizeof(long) == PLIBSYS_SIZEOF_LONG);
  ASSERT(sizeof(ulong_t) == PLIBSYS_SIZEOF_LONG);
  ASSERT(sizeof(poffset) == 8);
  return CUTE_SUCCESS;
}

CUTEST(types_pointers, convert) {
  ptr_t pointer = P_INT_TO_POINTER (128);
  ASSERT(P_POINTER_TO_INT(pointer) == 128);

  int pint_val = -64;
  pointer = PINT_TO_POINTER (pint_val);
  ASSERT(PPOINTER_TO_INT(pointer) == -64);

  uint_t puint_val = 64;
  pointer = PUINT_TO_POINTER (puint_val);
  ASSERT(PPOINTER_TO_UINT(pointer) == 64);

  size_t psize_val = 1024;
  pointer = PSIZE_TO_POINTER (psize_val);
  ASSERT(PPOINTER_TO_PSIZE(psize_val) == 1024);
  return CUTE_SUCCESS;
}

CUTEST(types_min, max) {
  ASSERT(P_MININT8 == (int8_t) 0x80);
  ASSERT(P_MAXINT8 == (int8_t) 0x7F);
  ASSERT(P_MAXUINT8 == (uint8_t) 0xFF);
  ASSERT(P_MININT16 == (int16_t) 0x8000);
  ASSERT(P_MAXINT16 == (int16_t) 0x7FFF);
  ASSERT(P_MAXUINT16 == (uint16_t) 0xFFFF);
  ASSERT(P_MININT32 == (int32_t) 0x80000000);
  ASSERT(P_MAXINT32 == (int32_t) 0x7FFFFFFF);
  ASSERT(P_MAXUINT32 == (uint32_t) 0xFFFFFFFF);
  ASSERT(P_MININT64 == (int64_t) 0x8000000000000000LL);
  ASSERT(P_MAXINT64 == (int64_t) 0x7FFFFFFFFFFFFFFFLL);
  ASSERT(P_MAXUINT64 == (uint64_t) 0xFFFFFFFFFFFFFFFFULL);

  if (PLIBSYS_SIZEOF_SIZE_T == 8) {
    ASSERT(P_MINSSIZE == P_MININT64);
    ASSERT(P_MAXSSIZE == P_MAXINT64);
    ASSERT(P_MAXSIZE == P_MAXUINT64);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      ASSERT(P_MINSSIZE == P_MINLONG);
      ASSERT(P_MAXSSIZE == P_MAXLONG);
      ASSERT(P_MAXSIZE == P_MAXULONG);
    }
  } else {
    ASSERT(P_MINSSIZE == P_MININT32);
    ASSERT(P_MAXSSIZE == P_MAXINT32);
    ASSERT(P_MAXSIZE == P_MAXUINT32);

    if (PLIBSYS_SIZEOF_LONG == 4) {
      ASSERT(P_MINSSIZE == P_MINLONG);
      ASSERT(P_MAXSSIZE == P_MAXLONG);
      ASSERT(P_MAXSIZE == P_MAXULONG);
    }
  }

  return CUTE_SUCCESS;
}

CUTEST(types, modifiers) {

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

  return CUTE_SUCCESS;
}

CUTEST(types, formats) {

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

  return CUTE_SUCCESS;
}

CUTEST(types_host, network) {

  if (P_BYTE_ORDER == P_LITTLE_ENDIAN) {
    int16_t pint16_val = PINT16_TO_BE (0xFFE0);
    ASSERT(pint16_val == (int16_t) 0xE0FF);
    ASSERT(PINT16_FROM_BE(pint16_val) == (int16_t) 0xFFE0);
    ASSERT(PINT16_TO_LE(pint16_val) == (int16_t) 0xE0FF);
    ASSERT(PINT16_FROM_LE(pint16_val) == (int16_t) 0xE0FF);

    uint16_t puint16_val = PUINT16_TO_BE (0x0020);
    ASSERT(puint16_val == (uint16_t) 0x2000);
    ASSERT(PUINT16_FROM_BE(puint16_val) == (uint16_t) 0x0020);
    ASSERT(PUINT16_TO_LE(puint16_val) == (uint16_t) 0x2000);
    ASSERT(PUINT16_FROM_LE(puint16_val) == (uint16_t) 0x2000);

    int32_t pint32_val = PINT32_TO_BE (0xFFFFFFC0);
    ASSERT(pint32_val == (int32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_BE(pint32_val) == (int32_t) 0xFFFFFFC0);
    ASSERT(PINT32_TO_LE(pint32_val) == (int32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_LE(pint32_val) == (int32_t) 0xC0FFFFFF);

    uint32_t puint32_val = PUINT32_TO_BE (0x00000040);
    ASSERT(puint32_val == (uint32_t) 0x40000000);
    ASSERT(PUINT32_FROM_BE(puint32_val) == (uint32_t) 0x00000040);
    ASSERT(PUINT32_TO_LE(puint32_val) == (uint32_t) 0x40000000);
    ASSERT(PUINT32_FROM_LE(puint32_val) == (uint32_t) 0x40000000);

    int64_t pint64_val = PINT64_TO_BE (0xFFFFFFFFFFFFFF80LL);
    ASSERT(pint64_val == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_BE(pint64_val) == (int64_t) 0xFFFFFFFFFFFFFF80LL);
    ASSERT(PINT64_TO_LE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_LE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);

    uint64_t puint64_val = PUINT64_TO_BE (0x0000000000000080ULL);
    ASSERT(puint64_val == (uint64_t) 0x8000000000000000ULL);
    ASSERT(
      PUINT64_FROM_BE(puint64_val) == (uint64_t) 0x0000000000000080ULL);
    ASSERT(
      PUINT64_TO_LE(puint64_val) == (uint64_t) 0x8000000000000000ULL);
    ASSERT(
      PUINT64_FROM_LE(puint64_val) == (uint64_t) 0x8000000000000000ULL);

    int pint_val = PINT_TO_BE (0xFFFFFC00);
    ASSERT(pint_val == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_BE(pint_val) == (int) 0xFFFFFC00);
    ASSERT(PINT_TO_LE(pint_val) == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_LE(pint_val) == (int) 0x00FCFFFF);

    uint_t puint_val = PUINT_TO_BE (0x00000400);
    ASSERT(puint_val == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_BE(puint_val) == (uint_t) 0x00000400);
    ASSERT(PUINT_TO_LE(puint_val) == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_LE(puint_val) == (uint_t) 0x00040000);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      long plong_val = PLONG_TO_BE (0xFFFFFFFFFFFFF800LL);
      ASSERT(plong_val == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0xFFFFFFFFFFFFF800LL);
      ASSERT(PLONG_TO_LE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);

      ulong_t pulong_val = PULONG_TO_BE (0x0000000000000800ULL);
      ASSERT(pulong_val == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      ASSERT(PULONG_TO_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      long plong_val = PLONG_TO_BE (0xFFFFF800);
      ASSERT(plong_val == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0xFFFFF800);
      ASSERT(PLONG_TO_LE(plong_val) == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0x00F8FFFF);

      ulong_t pulong_val = PULONG_TO_BE (0x00000800);
      ASSERT(pulong_val == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00000800);
      ASSERT(PULONG_TO_LE(pulong_val) == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00080000);
    }

    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      size_t psize_val = PSIZE_TO_BE (0x0000000000001000ULL);
      ASSERT(psize_val == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x0000000000001000ULL);
      ASSERT(PSIZE_TO_LE(psize_val) == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x0010000000000000ULL);

      ssize_t pssize_val = PSSIZE_TO_BE (0x000000000000F000LL);
      ASSERT(pssize_val == (ssize_t) 0x00F0000000000000LL);
      ASSERT(
        PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      ASSERT(PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      ASSERT(
        PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      size_t psize_val = PSIZE_TO_BE (0x00001000);
      ASSERT(psize_val == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x00001000);
      ASSERT(PSIZE_TO_LE(psize_val) == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x00100000);

      ssize_t pssize_val = PSSIZE_TO_BE (0x0000F000);
      ASSERT(pssize_val == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x0000F000);
      ASSERT(PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F00000);
    }

    puint16_val = p_htons (0x0020);
    ASSERT(puint16_val == (uint16_t) 0x2000);
    ASSERT(p_ntohs(puint16_val) == (uint16_t) 0x0020);

    puint32_val = p_htonl (0x00000040);
    ASSERT(puint32_val == (uint32_t) 0x40000000);
    ASSERT(p_ntohl(puint32_val) == (uint32_t) 0x00000040);
  } else {
    int16_t pint16_val = PINT16_TO_LE (0xFFE0);
    ASSERT(pint16_val == (int16_t) 0xE0FF);
    ASSERT(PINT16_FROM_LE(pint16_val) == (int16_t) 0xFFE0);
    ASSERT(PINT16_TO_BE(pint16_val) == (int16_t) 0xE0FF);
    ASSERT(PINT16_FROM_BE(pint16_val) == (int16_t) 0xE0FF);

    uint16_t puint16_val = PUINT16_TO_LE (0x0020);
    ASSERT(puint16_val == (uint16_t) 0x2000);
    ASSERT(PUINT16_FROM_LE(puint16_val) == (uint16_t) 0x0020);
    ASSERT(PUINT16_TO_BE(puint16_val) == (uint16_t) 0x2000);
    ASSERT(PUINT16_FROM_BE(puint16_val) == (uint16_t) 0x2000);

    int32_t pint32_val = PINT32_TO_LE (0xFFFFFFC0);
    ASSERT(pint32_val == (int32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_LE(pint32_val) == (int32_t) 0xFFFFFFC0);
    ASSERT(PINT32_TO_BE(pint32_val) == (int32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_BE(pint32_val) == (int32_t) 0xC0FFFFFF);

    uint32_t puint32_val = PUINT32_TO_LE (0x00000040);
    ASSERT(puint32_val == (uint32_t) 0x40000000);
    ASSERT(PUINT32_FROM_LE(puint32_val) == (uint32_t) 0x00000040);
    ASSERT(PUINT32_TO_BE(puint32_val) == (uint32_t) 0x40000000);
    ASSERT(PUINT32_FROM_BE(puint32_val) == (uint32_t) 0x40000000);

    int64_t pint64_val = PINT64_TO_LE (0xFFFFFFFFFFFFFF80LL);
    ASSERT(pint64_val == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_LE(pint64_val) == (int64_t) 0xFFFFFFFFFFFFFF80LL);
    ASSERT(PINT64_TO_BE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_BE(pint64_val) == (int64_t) 0x80FFFFFFFFFFFFFFLL);

    uint64_t puint64_val = PUINT64_TO_LE (0x0000000000000080ULL);
    ASSERT(puint64_val == (uint64_t) 0x8000000000000000ULL);
    ASSERT(
      PUINT64_FROM_LE(puint64_val) == (uint64_t) 0x0000000000000080ULL);
    ASSERT(
      PUINT64_TO_BE(puint64_val) == (uint64_t) 0x8000000000000000ULL);
    ASSERT(
      PUINT64_FROM_BE(puint64_val) == (uint64_t) 0x8000000000000000ULL);

    int pint_val = PINT_TO_LE (0xFFFFFC00);
    ASSERT(pint_val == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_LE(pint_val) == (int) 0xFFFFFC00);
    ASSERT(PINT_TO_BE(pint_val) == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_BE(pint_val) == (int) 0x00FCFFFF);

    uint_t puint_val = PUINT_TO_LE (0x00000400);
    ASSERT(puint_val == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_LE(puint_val) == (uint_t) 0x00000400);
    ASSERT(PUINT_TO_BE(puint_val) == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_BE(puint_val) == (uint_t) 0x00040000);

    if (PLIBSYS_SIZEOF_LONG == 8) {
      long plong_val = PLONG_TO_LE (0xFFFFFFFFFFFFF800LL);
      ASSERT(plong_val == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0xFFFFFFFFFFFFF800LL);
      ASSERT(PLONG_TO_BE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);

      ulong_t pulong_val = PULONG_TO_LE (0x0000000000000800ULL);
      ASSERT(pulong_val == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      ASSERT(PULONG_TO_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      long plong_val = PLONG_TO_LE (0xFFFFF800);
      ASSERT(plong_val == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0xFFFFF800);
      ASSERT(PLONG_TO_BE(plong_val) == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0x00F8FFFF);

      ulong_t pulong_val = PULONG_TO_LE (0x00000800);
      ASSERT(pulong_val == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00000800);
      ASSERT(PULONG_TO_BE(pulong_val) == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00080000);
    }

    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      size_t psize_val = PSIZE_TO_LE (0x0000000000001000ULL);
      ASSERT(psize_val == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x0000000000001000ULL);
      ASSERT(PSIZE_TO_BE(psize_val) == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x0010000000000000ULL);

      ssize_t pssize_val = PSSIZE_TO_LE (0x000000000000F000LL);
      ASSERT(pssize_val == (ssize_t) 0x00F0000000000000LL);
      ASSERT(
        PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      ASSERT(PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      ASSERT(
        PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      size_t psize_val = PSIZE_TO_LE (0x00001000);
      ASSERT(psize_val == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x00001000);
      ASSERT(PSIZE_TO_BE(psize_val) == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x00100000);

      ssize_t pssize_val = PSSIZE_TO_LE (0x0000F000);
      ASSERT(pssize_val == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x0000F000);
      ASSERT(PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F00000);
    }

    puint16_val = p_htons (0x0020);
    ASSERT(puint16_val == (uint16_t) 0x0020);
    ASSERT(p_ntohs(puint16_val) == (uint16_t) 0x0020);

    puint32_val = p_htonl (0x00000040);
    ASSERT(puint32_val == (uint32_t) 0x00000040);
    ASSERT(p_ntohl(puint32_val) == (uint32_t) 0x00000040);
  }

  uint16_t puint16_val = PUINT16_SWAP_BYTES (0x0020);
  ASSERT(puint16_val == (uint16_t) 0x2000);
  ASSERT(PUINT16_SWAP_BYTES(puint16_val) == (uint16_t) 0x0020);

  uint32_t puint32_val = PUINT32_SWAP_BYTES (0x00000040);
  ASSERT(puint32_val == (uint32_t) 0x40000000);
  ASSERT(PUINT32_SWAP_BYTES(puint32_val) == (uint32_t) 0x00000040);

  uint64_t puint64_val = PUINT64_SWAP_BYTES (0x0000000000000080ULL);
  ASSERT(puint64_val == (uint64_t) 0x8000000000000000ULL);
  ASSERT(
    PUINT64_SWAP_BYTES(puint64_val) == (uint64_t) 0x0000000000000080ULL);

  return CUTE_SUCCESS;
}

int
main(int ac, char **av) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(types, general);
  CUTEST_PASS(types, pointers_convert);
  CUTEST_PASS(types, min_max);
  CUTEST_PASS(types, modifiers);
  CUTEST_PASS(types, formats);
  CUTEST_PASS(types, host_network);
  return EXIT_SUCCESS;
}
