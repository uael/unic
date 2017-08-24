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

#include "unic/string.h"
#include "unic/mem.h"
#include "hash-sha1.h"

struct PHashSHA1_ {
  union buf_ {
    ubyte_t buf[64];
    u32_t buf_w[16];
  } buf;
  u32_t hash[5];

  u32_t len_high;
  u32_t len_low;
};

static const ubyte_t pp_crypto_hash_sha1_pad[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void
pp_crypto_hash_sha1_swap_bytes(u32_t *data, uint_t words);

static void
pp_crypto_hash_sha1_process(PHashSHA1 *ctx, const u32_t data[16]);

#define U_SHA1_ROTL(val, shift) ((val) << (shift) |  (val) >> (32 - (shift)))
#define U_SHA1_F1(x, y, z) ((x & y) | ((~x) & z))
#define U_SHA1_F2(x, y, z) (x ^ y ^ z)
#define U_SHA1_F3(x, y, z) ((x & y) | (x & z) | (y & z))
#define U_SHA1_W(W, i) \
( \
  (W)[i & 0x0F] = U_SHA1_ROTL ( \
        (W)[(i - 3)  & 0x0F] \
            ^ (W)[(i - 8)  & 0x0F] \
            ^ (W)[(i - 14) & 0x0F] \
            ^ (W)[(i - 16) & 0x0F], \
            1) \
)
#define U_SHA1_ROUND_0(a, b, c, d, e, w) \
{ \
  e += U_SHA1_ROTL (a, 5) + U_SHA1_F1 (b, c, d) \
     + 0x5A827999 + w; \
  b = U_SHA1_ROTL (b, 30); \
}
#define U_SHA1_ROUND_1(a, b, c, d, e, w) \
{ \
  e += U_SHA1_ROTL (a, 5) + U_SHA1_F2 (b, c, d) \
     + 0x6ED9EBA1 + w; \
  b = U_SHA1_ROTL (b, 30); \
}
#define U_SHA1_ROUND_2(a, b, c, d, e, w) \
{ \
  e += U_SHA1_ROTL (a, 5) + U_SHA1_F3 (b, c, d) \
     + 0x8F1BBCDC + w; \
  b = U_SHA1_ROTL (b, 30); \
}
#define U_SHA1_ROUND_3(a, b, c, d, e, w) \
{ \
  e += U_SHA1_ROTL (a, 5) + U_SHA1_F2 (b, c, d) \
     + 0xCA62C1D6 + w; \
  b = U_SHA1_ROTL (b, 30); \
}

static void
pp_crypto_hash_sha1_swap_bytes(u32_t *data,
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
pp_crypto_hash_sha1_process(PHashSHA1 *ctx,
  const u32_t data[16]) {
  u32_t W[16], A, B, C, D, E;
  if (U_UNLIKELY (ctx == NULL)) {
    return;
  }
  memcpy(W, data, 64);
  A = ctx->hash[0];
  B = ctx->hash[1];
  C = ctx->hash[2];
  D = ctx->hash[3];
  E = ctx->hash[4];
  U_SHA1_ROUND_0 (A, B, C, D, E, W[0]);
  U_SHA1_ROUND_0 (E, A, B, C, D, W[1]);
  U_SHA1_ROUND_0 (D, E, A, B, C, W[2]);
  U_SHA1_ROUND_0 (C, D, E, A, B, W[3]);
  U_SHA1_ROUND_0 (B, C, D, E, A, W[4]);
  U_SHA1_ROUND_0 (A, B, C, D, E, W[5]);
  U_SHA1_ROUND_0 (E, A, B, C, D, W[6]);
  U_SHA1_ROUND_0 (D, E, A, B, C, W[7]);
  U_SHA1_ROUND_0 (C, D, E, A, B, W[8]);
  U_SHA1_ROUND_0 (B, C, D, E, A, W[9]);
  U_SHA1_ROUND_0 (A, B, C, D, E, W[10]);
  U_SHA1_ROUND_0 (E, A, B, C, D, W[11]);
  U_SHA1_ROUND_0 (D, E, A, B, C, W[12]);
  U_SHA1_ROUND_0 (C, D, E, A, B, W[13]);
  U_SHA1_ROUND_0 (B, C, D, E, A, W[14]);
  U_SHA1_ROUND_0 (A, B, C, D, E, W[15]);
  U_SHA1_ROUND_0 (E, A, B, C, D, U_SHA1_W(W, 16));
  U_SHA1_ROUND_0 (D, E, A, B, C, U_SHA1_W(W, 17));
  U_SHA1_ROUND_0 (C, D, E, A, B, U_SHA1_W(W, 18));
  U_SHA1_ROUND_0 (B, C, D, E, A, U_SHA1_W(W, 19));
  U_SHA1_ROUND_1 (A, B, C, D, E, U_SHA1_W(W, 20));
  U_SHA1_ROUND_1 (E, A, B, C, D, U_SHA1_W(W, 21));
  U_SHA1_ROUND_1 (D, E, A, B, C, U_SHA1_W(W, 22));
  U_SHA1_ROUND_1 (C, D, E, A, B, U_SHA1_W(W, 23));
  U_SHA1_ROUND_1 (B, C, D, E, A, U_SHA1_W(W, 24));
  U_SHA1_ROUND_1 (A, B, C, D, E, U_SHA1_W(W, 25));
  U_SHA1_ROUND_1 (E, A, B, C, D, U_SHA1_W(W, 26));
  U_SHA1_ROUND_1 (D, E, A, B, C, U_SHA1_W(W, 27));
  U_SHA1_ROUND_1 (C, D, E, A, B, U_SHA1_W(W, 28));
  U_SHA1_ROUND_1 (B, C, D, E, A, U_SHA1_W(W, 29));
  U_SHA1_ROUND_1 (A, B, C, D, E, U_SHA1_W(W, 30));
  U_SHA1_ROUND_1 (E, A, B, C, D, U_SHA1_W(W, 31));
  U_SHA1_ROUND_1 (D, E, A, B, C, U_SHA1_W(W, 32));
  U_SHA1_ROUND_1 (C, D, E, A, B, U_SHA1_W(W, 33));
  U_SHA1_ROUND_1 (B, C, D, E, A, U_SHA1_W(W, 34));
  U_SHA1_ROUND_1 (A, B, C, D, E, U_SHA1_W(W, 35));
  U_SHA1_ROUND_1 (E, A, B, C, D, U_SHA1_W(W, 36));
  U_SHA1_ROUND_1 (D, E, A, B, C, U_SHA1_W(W, 37));
  U_SHA1_ROUND_1 (C, D, E, A, B, U_SHA1_W(W, 38));
  U_SHA1_ROUND_1 (B, C, D, E, A, U_SHA1_W(W, 39));
  U_SHA1_ROUND_2 (A, B, C, D, E, U_SHA1_W(W, 40));
  U_SHA1_ROUND_2 (E, A, B, C, D, U_SHA1_W(W, 41));
  U_SHA1_ROUND_2 (D, E, A, B, C, U_SHA1_W(W, 42));
  U_SHA1_ROUND_2 (C, D, E, A, B, U_SHA1_W(W, 43));
  U_SHA1_ROUND_2 (B, C, D, E, A, U_SHA1_W(W, 44));
  U_SHA1_ROUND_2 (A, B, C, D, E, U_SHA1_W(W, 45));
  U_SHA1_ROUND_2 (E, A, B, C, D, U_SHA1_W(W, 46));
  U_SHA1_ROUND_2 (D, E, A, B, C, U_SHA1_W(W, 47));
  U_SHA1_ROUND_2 (C, D, E, A, B, U_SHA1_W(W, 48));
  U_SHA1_ROUND_2 (B, C, D, E, A, U_SHA1_W(W, 49));
  U_SHA1_ROUND_2 (A, B, C, D, E, U_SHA1_W(W, 50));
  U_SHA1_ROUND_2 (E, A, B, C, D, U_SHA1_W(W, 51));
  U_SHA1_ROUND_2 (D, E, A, B, C, U_SHA1_W(W, 52));
  U_SHA1_ROUND_2 (C, D, E, A, B, U_SHA1_W(W, 53));
  U_SHA1_ROUND_2 (B, C, D, E, A, U_SHA1_W(W, 54));
  U_SHA1_ROUND_2 (A, B, C, D, E, U_SHA1_W(W, 55));
  U_SHA1_ROUND_2 (E, A, B, C, D, U_SHA1_W(W, 56));
  U_SHA1_ROUND_2 (D, E, A, B, C, U_SHA1_W(W, 57));
  U_SHA1_ROUND_2 (C, D, E, A, B, U_SHA1_W(W, 58));
  U_SHA1_ROUND_2 (B, C, D, E, A, U_SHA1_W(W, 59));
  U_SHA1_ROUND_3 (A, B, C, D, E, U_SHA1_W(W, 60));
  U_SHA1_ROUND_3 (E, A, B, C, D, U_SHA1_W(W, 61));
  U_SHA1_ROUND_3 (D, E, A, B, C, U_SHA1_W(W, 62));
  U_SHA1_ROUND_3 (C, D, E, A, B, U_SHA1_W(W, 63));
  U_SHA1_ROUND_3 (B, C, D, E, A, U_SHA1_W(W, 64));
  U_SHA1_ROUND_3 (A, B, C, D, E, U_SHA1_W(W, 65));
  U_SHA1_ROUND_3 (E, A, B, C, D, U_SHA1_W(W, 66));
  U_SHA1_ROUND_3 (D, E, A, B, C, U_SHA1_W(W, 67));
  U_SHA1_ROUND_3 (C, D, E, A, B, U_SHA1_W(W, 68));
  U_SHA1_ROUND_3 (B, C, D, E, A, U_SHA1_W(W, 69));
  U_SHA1_ROUND_3 (A, B, C, D, E, U_SHA1_W(W, 70));
  U_SHA1_ROUND_3 (E, A, B, C, D, U_SHA1_W(W, 71));
  U_SHA1_ROUND_3 (D, E, A, B, C, U_SHA1_W(W, 72));
  U_SHA1_ROUND_3 (C, D, E, A, B, U_SHA1_W(W, 73));
  U_SHA1_ROUND_3 (B, C, D, E, A, U_SHA1_W(W, 74));
  U_SHA1_ROUND_3 (A, B, C, D, E, U_SHA1_W(W, 75));
  U_SHA1_ROUND_3 (E, A, B, C, D, U_SHA1_W(W, 76));
  U_SHA1_ROUND_3 (D, E, A, B, C, U_SHA1_W(W, 77));
  U_SHA1_ROUND_3 (C, D, E, A, B, U_SHA1_W(W, 78));
  U_SHA1_ROUND_3 (B, C, D, E, A, U_SHA1_W(W, 79));
  ctx->hash[0] += A;
  ctx->hash[1] += B;
  ctx->hash[2] += C;
  ctx->hash[3] += D;
  ctx->hash[4] += E;
}

void
u_crypto_hash_sha1_reset(PHashSHA1 *ctx) {
  memset(ctx->buf.buf, 0, 64);
  ctx->len_low = 0;
  ctx->len_high = 0;
  ctx->hash[0] = 0x67452301;
  ctx->hash[1] = 0xEFCDAB89;
  ctx->hash[2] = 0x98BADCFE;
  ctx->hash[3] = 0x10325476;
  ctx->hash[4] = 0xC3D2E1F0;
}

PHashSHA1 *
u_crypto_hash_sha1_new(void) {
  PHashSHA1 *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(PHashSHA1))) == NULL)) {
    return NULL;
  }
  u_crypto_hash_sha1_reset(ret);
  return ret;
}

void
u_crypto_hash_sha1_update(PHashSHA1 *ctx,
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
    pp_crypto_hash_sha1_swap_bytes(ctx->buf.buf_w, 16);
    pp_crypto_hash_sha1_process(ctx, ctx->buf.buf_w);
    data += to_fill;
    len -= to_fill;
    left = 0;
  }
  while (len >= 64) {
    memcpy(ctx->buf.buf, data, 64);
    pp_crypto_hash_sha1_swap_bytes(ctx->buf.buf_w, 16);
    pp_crypto_hash_sha1_process(ctx, ctx->buf.buf_w);
    data += 64;
    len -= 64;
  }
  if (len > 0) {
    memcpy(ctx->buf.buf + left, data, len);
  }
}

void
u_crypto_hash_sha1_finish(PHashSHA1 *ctx) {
  u32_t high, low;
  int left, last;
  left = ctx->len_low & 0x3F;
  last = (left < 56) ? (56 - left) : (120 - left);
  low = ctx->len_low << 3;
  high = ctx->len_high << 3
    | ctx->len_low >> 29;
  if (last > 0) {
    u_crypto_hash_sha1_update(ctx, pp_crypto_hash_sha1_pad, (size_t) last);
  }
  ctx->buf.buf_w[14] = high;
  ctx->buf.buf_w[15] = low;
  pp_crypto_hash_sha1_swap_bytes(ctx->buf.buf_w, 14);
  pp_crypto_hash_sha1_process(ctx, ctx->buf.buf_w);
  pp_crypto_hash_sha1_swap_bytes(ctx->hash, 5);
}

const ubyte_t *
u_crypto_hash_sha1_digest(PHashSHA1 *ctx) {
  return (const ubyte_t *) ctx->hash;
}

void
u_crypto_hash_sha1_free(PHashSHA1 *ctx) {
  u_free(ctx);
}
