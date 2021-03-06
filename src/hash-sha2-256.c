/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "unic/string.h"
#include "unic/mem.h"
#include "hash-sha2-256.h"

struct PHashSHA2_256_ {
  union buf_ {
    ubyte_t buf[64];
    u32_t buf_w[16];
  } buf;
  u32_t hash[8];

  u32_t len_high;
  u32_t len_low;

  bool is224;
};

static const ubyte_t pp_crypto_hash_sha2_256_pad[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const u32_t pp_crypto_hash_sha2_256_K[] = {
  0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
  0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
  0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
  0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
  0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
  0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
  0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
  0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
  0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
  0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
  0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
  0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
  0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
  0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
  0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
  0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static void
pp_crypto_hash_sha2_256_swap_bytes(u32_t *data, uint_t words);

static void
pp_crypto_hash_sha2_256_process(PHashSHA2_256 *ctx,
  const u32_t data[16]);

static PHashSHA2_256 *
pp_crypto_hash_sha2_256_new_internal(bool is224);

#define U_SHA2_256_SHR(val, shift) (((val) & 0xFFFFFFFF) >> (shift))
#define U_SHA2_256_ROTR(val, shift) (U_SHA2_256_SHR(val, shift) | ((val) << (32 - (shift))))
#define U_SHA2_256_S0(x) (U_SHA2_256_ROTR (x, 7)  ^ U_SHA2_256_ROTR (x, 18) ^ U_SHA2_256_SHR  (x, 3))
#define U_SHA2_256_S1(x) (U_SHA2_256_ROTR (x, 17) ^ U_SHA2_256_ROTR (x, 19) ^ U_SHA2_256_SHR  (x, 10))
#define U_SHA2_256_S2(x) (U_SHA2_256_ROTR (x, 2)  ^ U_SHA2_256_ROTR (x, 13) ^ U_SHA2_256_ROTR (x, 22))
#define U_SHA2_256_S3(x) (U_SHA2_256_ROTR (x, 6)  ^ U_SHA2_256_ROTR (x, 11) ^ U_SHA2_256_ROTR (x, 25))
#define U_SHA2_256_F0(x, y, z) ((x & y) | (z & (x | y)))
#define U_SHA2_256_F1(x, y, z) (z ^ (x & (y ^ z)))
#define U_SHA2_256_R(t) \
( \
  W[t] = U_SHA2_256_S1 (W[t -  2]) + W[t -  7] + \
         U_SHA2_256_S0 (W[t - 15]) + W[t - 16] \
)
#define U_SHA2_256_P(a, b, c, d, e, f, g, h, x, K) \
{ \
  tmp_sum1 = h + U_SHA2_256_S3 (e) + U_SHA2_256_F1 (e, f, g) + K + x; \
  tmp_sum2 = U_SHA2_256_S2 (a) + U_SHA2_256_F0 (a, b, c); \
  d += tmp_sum1; \
  h = tmp_sum1 + tmp_sum2; \
}

static void
pp_crypto_hash_sha2_256_swap_bytes(u32_t *data,
  uint_t words) {
#ifdef UNIC_IS_BIGENDIAN
  U_UNUSED (data);
  U_UNUSED (words);
#else
  while (words-- > 0) {
    *data = PUINT32_TO_BE (*data);
    ++data;
  }
#endif
}

static void
pp_crypto_hash_sha2_256_process(PHashSHA2_256 *ctx,
  const u32_t data[16]) {
  u32_t tmp_sum1, tmp_sum2;
  u32_t W[64];
  u32_t A[8];
  uint_t i;
  for (i = 0; i < 8; i++) {
    A[i] = ctx->hash[i];
  }
  memcpy(W, data, 64);
  for (i = 0; i < 16; i += 8) {
    U_SHA2_256_P (A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i + 0],
      pp_crypto_hash_sha2_256_K[i + 0]);
    U_SHA2_256_P (A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6], W[i + 1],
      pp_crypto_hash_sha2_256_K[i + 1]);
    U_SHA2_256_P (A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5], W[i + 2],
      pp_crypto_hash_sha2_256_K[i + 2]);
    U_SHA2_256_P (A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4], W[i + 3],
      pp_crypto_hash_sha2_256_K[i + 3]);
    U_SHA2_256_P (A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3], W[i + 4],
      pp_crypto_hash_sha2_256_K[i + 4]);
    U_SHA2_256_P (A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2], W[i + 5],
      pp_crypto_hash_sha2_256_K[i + 5]);
    U_SHA2_256_P (A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1], W[i + 6],
      pp_crypto_hash_sha2_256_K[i + 6]);
    U_SHA2_256_P (A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0], W[i + 7],
      pp_crypto_hash_sha2_256_K[i + 7]);
  }
  for (i = 16; i < 64; i += 8) {
    U_SHA2_256_P (A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7],
      U_SHA2_256_R(i + 0), pp_crypto_hash_sha2_256_K[i + 0]);
    U_SHA2_256_P (A[7], A[0], A[1], A[2], A[3], A[4], A[5], A[6],
      U_SHA2_256_R(i + 1), pp_crypto_hash_sha2_256_K[i + 1]);
    U_SHA2_256_P (A[6], A[7], A[0], A[1], A[2], A[3], A[4], A[5],
      U_SHA2_256_R(i + 2), pp_crypto_hash_sha2_256_K[i + 2]);
    U_SHA2_256_P (A[5], A[6], A[7], A[0], A[1], A[2], A[3], A[4],
      U_SHA2_256_R(i + 3), pp_crypto_hash_sha2_256_K[i + 3]);
    U_SHA2_256_P (A[4], A[5], A[6], A[7], A[0], A[1], A[2], A[3],
      U_SHA2_256_R(i + 4), pp_crypto_hash_sha2_256_K[i + 4]);
    U_SHA2_256_P (A[3], A[4], A[5], A[6], A[7], A[0], A[1], A[2],
      U_SHA2_256_R(i + 5), pp_crypto_hash_sha2_256_K[i + 5]);
    U_SHA2_256_P (A[2], A[3], A[4], A[5], A[6], A[7], A[0], A[1],
      U_SHA2_256_R(i + 6), pp_crypto_hash_sha2_256_K[i + 6]);
    U_SHA2_256_P (A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[0],
      U_SHA2_256_R(i + 7), pp_crypto_hash_sha2_256_K[i + 7]);
  }
  for (i = 0; i < 8; i++) {
    ctx->hash[i] += A[i];
  }
}

static PHashSHA2_256 *
pp_crypto_hash_sha2_256_new_internal(bool is224) {
  PHashSHA2_256 *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(PHashSHA2_256))) == NULL)) {
    return NULL;
  }
  ret->is224 = is224;
  u_crypto_hash_sha2_256_reset(ret);
  return ret;
}

void
u_crypto_hash_sha2_256_reset(PHashSHA2_256 *ctx) {
  memset(ctx->buf.buf, 0, 64);
  ctx->len_low = 0;
  ctx->len_high = 0;
  if (ctx->is224 == false) {
    /* SHA2-256 */
    ctx->hash[0] = 0x6A09E667;
    ctx->hash[1] = 0xBB67AE85;
    ctx->hash[2] = 0x3C6EF372;
    ctx->hash[3] = 0xA54FF53A;
    ctx->hash[4] = 0x510E527F;
    ctx->hash[5] = 0x9B05688C;
    ctx->hash[6] = 0x1F83D9AB;
    ctx->hash[7] = 0x5BE0CD19;
  } else {
    /* SHA2-224 */
    ctx->hash[0] = 0xC1059ED8;
    ctx->hash[1] = 0x367CD507;
    ctx->hash[2] = 0x3070DD17;
    ctx->hash[3] = 0xF70E5939;
    ctx->hash[4] = 0xFFC00B31;
    ctx->hash[5] = 0x68581511;
    ctx->hash[6] = 0x64F98FA7;
    ctx->hash[7] = 0xBEFA4FA4;
  }
}

PHashSHA2_256 *
u_crypto_hash_sha2_256_new(void) {
  return pp_crypto_hash_sha2_256_new_internal(false);
}

PHashSHA2_256 *
u_crypto_hash_sha2_224_new(void) {
  return pp_crypto_hash_sha2_256_new_internal(true);
}

void
u_crypto_hash_sha2_256_update(PHashSHA2_256 *ctx,
  const ubyte_t *data,
  size_t len) {
  u32_t left, to_fill;
  left = ctx->len_low & 0x3F;
  to_fill = 64 - left;
  ctx->len_low += (u32_t) len;
  if (ctx->len_low < (u32_t) len) {
    ++ctx->len_high;
  }
  if (left && (u32_t) len >= to_fill) {
    memcpy(ctx->buf.buf + left, data, to_fill);
    pp_crypto_hash_sha2_256_swap_bytes(ctx->buf.buf_w, 16);
    pp_crypto_hash_sha2_256_process(ctx, ctx->buf.buf_w);
    data += to_fill;
    len -= to_fill;
    left = 0;
  }
  while (len >= 64) {
    memcpy(ctx->buf.buf, data, 64);
    pp_crypto_hash_sha2_256_swap_bytes(ctx->buf.buf_w, 16);
    pp_crypto_hash_sha2_256_process(ctx, ctx->buf.buf_w);
    data += 64;
    len -= 64;
  }
  if (len > 0) {
    memcpy(ctx->buf.buf + left, data, len);
  }
}

void
u_crypto_hash_sha2_256_finish(PHashSHA2_256 *ctx) {
  u32_t high, low;
  int left, last;
  left = ctx->len_low & 0x3F;
  last = (left < 56) ? (56 - left) : (120 - left);
  low = ctx->len_low << 3;
  high = ctx->len_high << 3
    | ctx->len_low >> 29;
  if (last > 0) {
    u_crypto_hash_sha2_256_update(
      ctx, pp_crypto_hash_sha2_256_pad,
      (size_t) last
    );
  }
  ctx->buf.buf_w[14] = high;
  ctx->buf.buf_w[15] = low;
  pp_crypto_hash_sha2_256_swap_bytes(ctx->buf.buf_w, 14);
  pp_crypto_hash_sha2_256_process(ctx, ctx->buf.buf_w);
  pp_crypto_hash_sha2_256_swap_bytes(ctx->hash, ctx->is224 == false ? 8 : 7);
}

const ubyte_t *
u_crypto_hash_sha2_256_digest(PHashSHA2_256 *ctx) {
  return (const ubyte_t *) ctx->hash;
}

void
u_crypto_hash_sha2_256_free(PHashSHA2_256 *ctx) {
  u_free(ctx);
}
