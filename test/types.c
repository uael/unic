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
  ASSERT(sizeof(i8_t) == 1);
  ASSERT(sizeof(u8_t) == 1);
  ASSERT(sizeof(i16_t) == 2);
  ASSERT(sizeof(u16_t) == 2);
  ASSERT(sizeof(i32_t) == 4);
  ASSERT(sizeof(u32_t) == 4);
  ASSERT(sizeof(i64_t) == 8);
  ASSERT(sizeof(u64_t) == 8);
  ASSERT(sizeof(void *) == sizeof(ptr_t));
  ASSERT(sizeof(const void *) == sizeof(const_ptr_t));
  ASSERT(sizeof(unsigned char) == sizeof(bool));
  ASSERT(sizeof(char) == sizeof(byte_t));
  ASSERT(sizeof(unsigned char) == sizeof(ubyte_t));
  ASSERT(sizeof(unsigned short) == sizeof(ushort_t));
  ASSERT(sizeof(unsigned int) == sizeof(uint_t));
  ASSERT(sizeof(unsigned long) == sizeof(ulong_t));
  ASSERT(sizeof(iptr_t) == PLIBSYS_SIZEOF_VOID_P);
  ASSERT(sizeof(uptr_t) == PLIBSYS_SIZEOF_VOID_P);
  ASSERT(sizeof(size_t) == PLIBSYS_SIZEOF_SIZE_T);
  ASSERT(sizeof(ssize_t) == PLIBSYS_SIZEOF_SIZE_T);
  ASSERT(sizeof(long) == PLIBSYS_SIZEOF_LONG);
  ASSERT(sizeof(ulong_t) == PLIBSYS_SIZEOF_LONG);
  ASSERT(sizeof(offset_t) == 8);
  return CUTE_SUCCESS;
}

CUTEST(types_pointers, convert) {
  ptr_t pointer;
  int pint_val;
  uint_t puint_val;
  size_t psize_val;

  pointer = P_INT_TO_POINTER (128);
  ASSERT(P_POINTER_TO_INT(pointer) == 128);
  pint_val = -64;
  pointer = PINT_TO_POINTER (pint_val);
  ASSERT(PPOINTER_TO_INT(pointer) == -64);
  puint_val = 64;
  pointer = PUINT_TO_POINTER (puint_val);
  ASSERT(PPOINTER_TO_UINT(pointer) == 64);
  psize_val = 1024;
  pointer = PSIZE_TO_POINTER (psize_val);
  ASSERT(PPOINTER_TO_PSIZE(psize_val) == 1024);
  return CUTE_SUCCESS;
}

CUTEST(types_min, max) {
  ASSERT(P_MININT8 == (i8_t) 0x80);
  ASSERT(P_MAXINT8 == (i8_t) 0x7F);
  ASSERT(P_MAXUINT8 == (u8_t) 0xFF);
  ASSERT(P_MININT16 == (i16_t) 0x8000);
  ASSERT(P_MAXINT16 == (i16_t) 0x7FFF);
  ASSERT(P_MAXUINT16 == (u16_t) 0xFFFF);
  ASSERT(P_MININT32 == (i32_t) 0x80000000);
  ASSERT(P_MAXINT32 == (i32_t) 0x7FFFFFFF);
  ASSERT(P_MAXUINT32 == (u32_t) 0xFFFFFFFF);
  ASSERT(P_MININT64 == (i64_t) 0x8000000000000000LL);
  ASSERT(P_MAXINT64 == (i64_t) 0x7FFFFFFFFFFFFFFFLL);
  ASSERT(P_MAXUINT64 == (u64_t) 0xFFFFFFFFFFFFFFFFULL);
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
  size_t size_val;
  ssize_t ssize_val;
  uptr_t puintptr_val;
  iptr_t pintptr_val;
  i16_t pint16_val;
  u32_t puint32_val;
  i32_t pint32_val;
  u64_t puint64_val;
  i64_t pint64_val;
  offset_t poffset_val;

  size_val = 256;
  printf("%#" PSIZE_MODIFIER "x\n", size_val);
  ssize_val = -256;
  printf("%#" PSIZE_MODIFIER "x\n", ssize_val);
  puintptr_val = 512;
  printf("%#" PINTPTR_MODIFIER "x\n", puintptr_val);
  pintptr_val = -512;
  printf("%#" PINTPTR_MODIFIER "x\n", pintptr_val);
  u16_t puint16_val;
  puint16_val = 1024;
  printf("%#" PINT16_MODIFIER "x\n", puint16_val);
  pint16_val = -1024;
  printf("%#" PINT16_MODIFIER "x\n", pint16_val);
  puint32_val = 2048;
  printf("%#" PINT32_MODIFIER "x\n", puint32_val);
  pint32_val = -2048;
  printf("%#" PINT32_MODIFIER "x\n", pint32_val);
  puint64_val = 4096;
  printf("%#" PINT64_MODIFIER "x\n", puint64_val);
  pint64_val = -4096;
  printf("%#" PINT64_MODIFIER "x\n", pint64_val);
  poffset_val = 8192;
  printf("%#" POFFSET_MODIFIER "x\n", poffset_val);
  return CUTE_SUCCESS;
}

CUTEST(types, formats) {
  ssize_t ssize_val;
  size_t size_val;
  uptr_t puintptr_val;
  iptr_t pintptr_val;
  u16_t puint16_val;
  i16_t pint16_val;
  u32_t puint32_val;
  i32_t pint32_val;
  u64_t puint64_val;
  i64_t pint64_val;
  offset_t poffset_val;

  ssize_val = -256;
  printf("%" PSSIZE_FORMAT "\n", ssize_val);
  size_val = 256;
  printf("%" PSIZE_FORMAT "\n", size_val);
  puintptr_val = 512;
  printf("%" PUINTPTR_FORMAT "\n", puintptr_val);
  pintptr_val = -512;
  printf("%" PINTPTR_FORMAT "\n", pintptr_val);
  puint16_val = 1024;
  printf("%" PUINT16_FORMAT "\n", puint16_val);
  pint16_val = -1024;
  printf("%" PINT16_FORMAT "\n", pint16_val);
  puint32_val = 2048;
  printf("%" PUINT32_FORMAT "\n", puint32_val);
  pint32_val = -2048;
  printf("%" PINT32_FORMAT "\n", pint32_val);
  puint64_val = 4096;
  printf("%" PUINT64_FORMAT "\n", puint64_val);
  pint64_val = -4096;
  printf("%" PINT64_FORMAT "\n", pint64_val);
  poffset_val = 8192;
  printf("%" POFFSET_FORMAT "\n", poffset_val);
  return CUTE_SUCCESS;
}

CUTEST(types_host, network) {
  i16_t pint16_val;
  u16_t puint16_val;
  i32_t pint32_val;
  u32_t puint32_val;
  i64_t pint64_val;
  u64_t puint64_val;
  int pint_val;
  uint_t puint_val;
  long plong_val;
  ulong_t pulong_val;
  size_t psize_val;
  ssize_t pssize_val;

  if (P_BYTE_ORDER == P_LITTLE_ENDIAN) {
    pint16_val = PINT16_TO_BE (0xFFE0);
    ASSERT(pint16_val == (i16_t) 0xE0FF);
    ASSERT(PINT16_FROM_BE(pint16_val) == (i16_t) 0xFFE0);
    ASSERT(PINT16_TO_LE(pint16_val) == (i16_t) 0xE0FF);
    ASSERT(PINT16_FROM_LE(pint16_val) == (i16_t) 0xE0FF);
    puint16_val = PUINT16_TO_BE (0x0020);
    ASSERT(puint16_val == (u16_t) 0x2000);
    ASSERT(PUINT16_FROM_BE(puint16_val) == (u16_t) 0x0020);
    ASSERT(PUINT16_TO_LE(puint16_val) == (u16_t) 0x2000);
    ASSERT(PUINT16_FROM_LE(puint16_val) == (u16_t) 0x2000);
    pint32_val = PINT32_TO_BE (0xFFFFFFC0);
    ASSERT(pint32_val == (i32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_BE(pint32_val) == (i32_t) 0xFFFFFFC0);
    ASSERT(PINT32_TO_LE(pint32_val) == (i32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_LE(pint32_val) == (i32_t) 0xC0FFFFFF);
    puint32_val = PUINT32_TO_BE (0x00000040);
    ASSERT(puint32_val == (u32_t) 0x40000000);
    ASSERT(PUINT32_FROM_BE(puint32_val) == (u32_t) 0x00000040);
    ASSERT(PUINT32_TO_LE(puint32_val) == (u32_t) 0x40000000);
    ASSERT(PUINT32_FROM_LE(puint32_val) == (u32_t) 0x40000000);
    pint64_val = PINT64_TO_BE (0xFFFFFFFFFFFFFF80LL);
    ASSERT(pint64_val == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_BE(pint64_val) == (i64_t) 0xFFFFFFFFFFFFFF80LL);
    ASSERT(PINT64_TO_LE(pint64_val) == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_LE(pint64_val) == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    puint64_val = PUINT64_TO_BE (0x0000000000000080ULL);
    ASSERT(puint64_val == (u64_t) 0x8000000000000000ULL);
    ASSERT(PUINT64_FROM_BE(puint64_val) == (u64_t) 0x0000000000000080ULL);
    ASSERT(PUINT64_TO_LE(puint64_val) == (u64_t) 0x8000000000000000ULL);
    ASSERT(PUINT64_FROM_LE(puint64_val) == (u64_t) 0x8000000000000000ULL);
    pint_val = PINT_TO_BE (0xFFFFFC00);
    ASSERT(pint_val == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_BE(pint_val) == (int) 0xFFFFFC00);
    ASSERT(PINT_TO_LE(pint_val) == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_LE(pint_val) == (int) 0x00FCFFFF);
    puint_val = PUINT_TO_BE (0x00000400);
    ASSERT(puint_val == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_BE(puint_val) == (uint_t) 0x00000400);
    ASSERT(PUINT_TO_LE(puint_val) == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_LE(puint_val) == (uint_t) 0x00040000);
    if (PLIBSYS_SIZEOF_LONG == 8) {
      plong_val = PLONG_TO_BE (0xFFFFFFFFFFFFF800LL);
      ASSERT(plong_val == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0xFFFFFFFFFFFFF800LL);
      ASSERT(PLONG_TO_LE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      pulong_val = PULONG_TO_BE (0x0000000000000800ULL);
      ASSERT(pulong_val == (ulong_t) 0x0008000000000000ULL);
      ASSERT(PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      ASSERT(PULONG_TO_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      ASSERT(PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      plong_val = PLONG_TO_BE (0xFFFFF800);
      ASSERT(plong_val == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0xFFFFF800);
      ASSERT(PLONG_TO_LE(plong_val) == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0x00F8FFFF);
      pulong_val = PULONG_TO_BE (0x00000800);
      ASSERT(pulong_val == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00000800);
      ASSERT(PULONG_TO_LE(pulong_val) == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00080000);
    }
    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      psize_val = PSIZE_TO_BE (0x0000000000001000ULL);
      ASSERT(psize_val == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x0000000000001000ULL);
      ASSERT(PSIZE_TO_LE(psize_val) == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x0010000000000000ULL);
      pssize_val = PSSIZE_TO_BE (0x000000000000F000LL);
      ASSERT(pssize_val == (ssize_t) 0x00F0000000000000LL);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      ASSERT(PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      psize_val = PSIZE_TO_BE (0x00001000);
      ASSERT(psize_val == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x00001000);
      ASSERT(PSIZE_TO_LE(psize_val) == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x00100000);
      pssize_val = PSSIZE_TO_BE (0x0000F000);
      ASSERT(pssize_val == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x0000F000);
      ASSERT(PSSIZE_TO_LE(pssize_val) == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x00F00000);
    }
    puint16_val = p_htons (0x0020);
    ASSERT(puint16_val == (u16_t) 0x2000);
    ASSERT(p_ntohs(puint16_val) == (u16_t) 0x0020);
    puint32_val = p_htonl (0x00000040);
    ASSERT(puint32_val == (u32_t) 0x40000000);
    ASSERT(p_ntohl(puint32_val) == (u32_t) 0x00000040);
  } else {
    pint16_val = PINT16_TO_LE (0xFFE0);
    ASSERT(pint16_val == (i16_t) 0xE0FF);
    ASSERT(PINT16_FROM_LE(pint16_val) == (i16_t) 0xFFE0);
    ASSERT(PINT16_TO_BE(pint16_val) == (i16_t) 0xE0FF);
    ASSERT(PINT16_FROM_BE(pint16_val) == (i16_t) 0xE0FF);
    puint16_val = PUINT16_TO_LE (0x0020);
    ASSERT(puint16_val == (u16_t) 0x2000);
    ASSERT(PUINT16_FROM_LE(puint16_val) == (u16_t) 0x0020);
    ASSERT(PUINT16_TO_BE(puint16_val) == (u16_t) 0x2000);
    ASSERT(PUINT16_FROM_BE(puint16_val) == (u16_t) 0x2000);
    pint32_val = PINT32_TO_LE (0xFFFFFFC0);
    ASSERT(pint32_val == (i32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_LE(pint32_val) == (i32_t) 0xFFFFFFC0);
    ASSERT(PINT32_TO_BE(pint32_val) == (i32_t) 0xC0FFFFFF);
    ASSERT(PINT32_FROM_BE(pint32_val) == (i32_t) 0xC0FFFFFF);
    puint32_val = PUINT32_TO_LE (0x00000040);
    ASSERT(puint32_val == (u32_t) 0x40000000);
    ASSERT(PUINT32_FROM_LE(puint32_val) == (u32_t) 0x00000040);
    ASSERT(PUINT32_TO_BE(puint32_val) == (u32_t) 0x40000000);
    ASSERT(PUINT32_FROM_BE(puint32_val) == (u32_t) 0x40000000);
    pint64_val = PINT64_TO_LE (0xFFFFFFFFFFFFFF80LL);
    ASSERT(pint64_val == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_LE(pint64_val) == (i64_t) 0xFFFFFFFFFFFFFF80LL);
    ASSERT(PINT64_TO_BE(pint64_val) == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    ASSERT(PINT64_FROM_BE(pint64_val) == (i64_t) 0x80FFFFFFFFFFFFFFLL);
    puint64_val = PUINT64_TO_LE (0x0000000000000080ULL);
    ASSERT(puint64_val == (u64_t) 0x8000000000000000ULL);
    ASSERT(PUINT64_FROM_LE(puint64_val) == (u64_t) 0x0000000000000080ULL);
    ASSERT(PUINT64_TO_BE(puint64_val) == (u64_t) 0x8000000000000000ULL);
    ASSERT(PUINT64_FROM_BE(puint64_val) == (u64_t) 0x8000000000000000ULL);
    pint_val = PINT_TO_LE (0xFFFFFC00);
    ASSERT(pint_val == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_LE(pint_val) == (int) 0xFFFFFC00);
    ASSERT(PINT_TO_BE(pint_val) == (int) 0x00FCFFFF);
    ASSERT(PINT_FROM_BE(pint_val) == (int) 0x00FCFFFF);
    puint_val = PUINT_TO_LE (0x00000400);
    ASSERT(puint_val == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_LE(puint_val) == (uint_t) 0x00000400);
    ASSERT(PUINT_TO_BE(puint_val) == (uint_t) 0x00040000);
    ASSERT(PUINT_FROM_BE(puint_val) == (uint_t) 0x00040000);
    if (PLIBSYS_SIZEOF_LONG == 8) {
      plong_val = PLONG_TO_LE (0xFFFFFFFFFFFFF800LL);
      ASSERT(plong_val == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0xFFFFFFFFFFFFF800LL);
      ASSERT(PLONG_TO_BE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0x00F8FFFFFFFFFFFFLL);
      pulong_val = PULONG_TO_LE (0x0000000000000800ULL);
      ASSERT(pulong_val == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_LE(pulong_val) == (ulong_t) 0x0000000000000800ULL);
      ASSERT(PULONG_TO_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
      ASSERT(
        PULONG_FROM_BE(pulong_val) == (ulong_t) 0x0008000000000000ULL);
    } else {
      plong_val = PLONG_TO_LE (0xFFFFF800);
      ASSERT(plong_val == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_LE(plong_val) == (long) 0xFFFFF800);
      ASSERT(PLONG_TO_BE(plong_val) == (long) 0x00F8FFFF);
      ASSERT(PLONG_FROM_BE(plong_val) == (long) 0x00F8FFFF);
      pulong_val = PULONG_TO_LE (0x00000800);
      ASSERT(pulong_val == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_LE(pulong_val) == (ulong_t) 0x00000800);
      ASSERT(PULONG_TO_BE(pulong_val) == (ulong_t) 0x00080000);
      ASSERT(PULONG_FROM_BE(pulong_val) == (ulong_t) 0x00080000);
    }
    if (PLIBSYS_SIZEOF_SIZE_T == 8) {
      psize_val = PSIZE_TO_LE (0x0000000000001000ULL);
      ASSERT(psize_val == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x0000000000001000ULL);
      ASSERT(PSIZE_TO_BE(psize_val) == (size_t) 0x0010000000000000ULL);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x0010000000000000ULL);
      pssize_val = PSSIZE_TO_LE (0x000000000000F000LL);
      ASSERT(pssize_val == (ssize_t) 0x00F0000000000000LL);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x000000000000F000LL);
      ASSERT(PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F0000000000000LL);
    } else {
      psize_val = PSIZE_TO_LE (0x00001000);
      ASSERT(psize_val == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_LE(psize_val) == (size_t) 0x00001000);
      ASSERT(PSIZE_TO_BE(psize_val) == (size_t) 0x00100000);
      ASSERT(PSIZE_FROM_BE(psize_val) == (size_t) 0x00100000);
      pssize_val = PSSIZE_TO_LE (0x0000F000);
      ASSERT(pssize_val == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_LE(pssize_val) == (ssize_t) 0x0000F000);
      ASSERT(PSSIZE_TO_BE(pssize_val) == (ssize_t) 0x00F00000);
      ASSERT(PSSIZE_FROM_BE(pssize_val) == (ssize_t) 0x00F00000);
    }
    puint16_val = p_htons (0x0020);
    ASSERT(puint16_val == (u16_t) 0x0020);
    ASSERT(p_ntohs(puint16_val) == (u16_t) 0x0020);
    puint32_val = p_htonl (0x00000040);
    ASSERT(puint32_val == (u32_t) 0x00000040);
    ASSERT(p_ntohl(puint32_val) == (u32_t) 0x00000040);
  }
  puint16_val = PUINT16_SWAP_BYTES (0x0020);
  ASSERT(puint16_val == (u16_t) 0x2000);
  ASSERT(PUINT16_SWAP_BYTES(puint16_val) == (u16_t) 0x0020);
  puint32_val = PUINT32_SWAP_BYTES (0x00000040);
  ASSERT(puint32_val == (u32_t) 0x40000000);
  ASSERT(PUINT32_SWAP_BYTES(puint32_val) == (u32_t) 0x00000040);
  puint64_val = PUINT64_SWAP_BYTES (0x0000000000000080ULL);
  ASSERT(puint64_val == (u64_t) 0x8000000000000000ULL);
  ASSERT(PUINT64_SWAP_BYTES(puint64_val) == (u64_t) 0x0000000000000080ULL);
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
