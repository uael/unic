/*
 * Copyright (C) 2011-2016 Alexander Saprykin <xelfium@gmail.com>
 * Copyright (C) 2009 Tom Van Baak (tvb) www.LeapSecond.com
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

#include "p/mem.h"
#include "p/string.h"
#include <ctype.h>

#define P_STR_MAX_EXPON    308

byte_t *
p_strdup(const byte_t *str) {
  byte_t *ret;
  size_t len;
  if (P_UNLIKELY (str == NULL)) {
    return NULL;
  }
  len = strlen(str) + 1;
  if (P_UNLIKELY ((ret = p_malloc(len)) == NULL)) {
    return NULL;
  }
  memcpy(ret, str, len);
  return ret;
}

byte_t *
p_strchomp(const byte_t *str) {
  ssize_t pos_start, pos_end;
  size_t str_len;
  byte_t *ret;
  const byte_t *ptr;
  if (P_UNLIKELY (str == NULL)) {
    return NULL;
  }
  ptr = str;
  pos_start = 0;
  pos_end = ((ssize_t) strlen(str)) - 1;
  while (pos_start < pos_end && isspace(*((const ubyte_t *) ptr++))) {
    ++pos_start;
  }
  ptr = str + pos_end;
  while (pos_end > 0 && isspace(*((const ubyte_t *) ptr--))) {
    --pos_end;
  }
  if (pos_end < pos_start) {
    return p_strdup("\0");
  }
  if (pos_end == pos_start && isspace(*((const ubyte_t *) (str + pos_end)))) {
    return p_strdup("\0");
  }
  str_len = (size_t) (pos_end - pos_start + 2);
  if (P_UNLIKELY ((ret = p_malloc0(str_len)) == NULL)) {
    return NULL;
  }
  memcpy(ret, str + pos_start, str_len - 1);
  *(ret + str_len - 1) = '\0';
  return ret;
}

byte_t *
p_strtok(byte_t *str, const byte_t *delim, byte_t **buf) {
  if (P_UNLIKELY (delim == NULL)) {
    return str;
  }
#ifdef P_OS_WIN
# ifdef P_CC_MSVC
    if (P_UNLIKELY (buf == NULL))
      return str;
#   if _MSC_VER < 1400
    P_UNUSED (buf);
    return strtok (str, delim);
#   else
    return strtok_s (str, delim, buf);
#   endif
# else
  P_UNUSED (buf);
  return strtok(str, delim);
# endif
#else
  if (P_UNLIKELY (buf == NULL))
    return str;

  return strtok_r(str, delim, buf);
#endif
}

double
p_strtod(const byte_t *str) {
  double sign;
  double value;
  double scale;
  double pow10;
  uint_t expon;
  int_t frac;
  byte_t *orig_str, *strp;
  orig_str = p_strchomp(str);
  if (P_UNLIKELY (orig_str == NULL)) {
    return 0.0;
  }
  strp = orig_str;
  sign = 1.0;
  if (*strp == '-') {
    sign = -1.0;
    strp += 1;
  } else if (*strp == '+') {
    strp += 1;
  }

  /* Get digits before decimal point or exponent, if any */
  for (value = 0.0; isdigit((int_t) *strp); strp += 1) {
    value = value * 10.0 + (*strp - '0');
  }

  /* Get digits after decimal point, if any */
  if (*strp == '.') {
    pow10 = 10.0;
    strp += 1;
    while (isdigit((int_t) *strp)) {
      value += (*strp - '0') / pow10;
      pow10 *= 10.0;
      strp += 1;
    }
  }

  /* Handle exponent, if any */
  frac = 0;
  scale = 1.0;
  if ((*strp == 'e') || (*strp == 'E')) {
    /* Get sign of exponent, if any */
    strp += 1;
    if (*strp == '-') {
      frac = 1;
      strp += 1;
    } else if (*strp == '+') {
      strp += 1;
    }

    /* Get digits of exponent, if any */
    for (expon = 0; isdigit((int_t) *strp); strp += 1) {
      expon = expon * 10 + (uint_t) ((*strp - '0'));
    }
    if (P_UNLIKELY (expon > P_STR_MAX_EXPON)) {
      expon = P_STR_MAX_EXPON;
    }

    /* Calculate scaling factor */
    while (expon >= 50) {
      scale *= 1e50;
      expon -= 50;
    }
    while (expon >= 8) {
      scale *= 1e8;
      expon -= 8;
    }
    while (expon > 0) {
      scale *= 10.0;
      expon -= 1;
    }
  }
  p_free(orig_str);

  /* Return signed and scaled floating point result */
  return sign * (frac ? (value / scale) : (value * scale));
}
