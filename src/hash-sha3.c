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
#include "hash-sha3.h"

struct PHashSHA3_ {
  union buf_ {
    ubyte_t buf[200];
    u64_t buf_w[25];
  } buf;
  u64_t hash[25];

  u32_t len;
  u32_t block_size;
};

static const u64_t pp_crypto_hash_sha3_K[] = {
  0x0000000000000001ULL, 0x0000000000008082ULL,
  0x800000000000808AULL, 0x8000000080008000ULL,
  0x000000000000808BULL, 0x0000000080000001ULL,
  0x8000000080008081ULL, 0x8000000000008009ULL,
  0x000000000000008AULL, 0x0000000000000088ULL,
  0x0000000080008009ULL, 0x000000008000000AULL,
  0x000000008000808BULL, 0x800000000000008BULL,
  0x8000000000008089ULL, 0x8000000000008003ULL,
  0x8000000000008002ULL, 0x8000000000000080ULL,
  0x000000000000800AULL, 0x800000008000000AULL,
  0x8000000080008081ULL, 0x8000000000008080ULL,
  0x0000000080000001ULL, 0x8000000080008008ULL
};

static void
pp_crypto_hash_sha3_swap_bytes(u64_t *data, uint_t words);

static void
pp_crypto_hash_sha3_keccak_theta(PHashSHA3 *ctx);

static void
pp_crypto_hash_sha3_keccak_rho_pi(PHashSHA3 *ctx);

static void
pp_crypto_hash_sha3_keccak_chi(PHashSHA3 *ctx);

static void
pp_crypto_hash_sha3_keccak_permutate(PHashSHA3 *ctx);

static void
pp_crypto_hash_sha3_process(PHashSHA3 *ctx, const u64_t *data);

static PHashSHA3 *
pp_crypto_hash_sha3_new_internal(uint_t bits);

#define U_SHA3_SHL(val, shift) ((val) << (shift))
#define U_SHA3_ROTL(val, shift) (U_SHA3_SHL(val, shift) | ((val) >> (64 - (shift))))

static void
pp_crypto_hash_sha3_swap_bytes(u64_t *data,
  uint_t words) {
#ifndef UNIC_IS_BIGENDIAN
  U_UNUSED (data);
  U_UNUSED (words);
#else
  while (words-- > 0) {
    *data = PUINT64_TO_LE (*data);
    ++data;
  }
#endif
}

/* Theta step (see [Keccak Reference, Section 2.3.2]) */
static void
pp_crypto_hash_sha3_keccak_theta(PHashSHA3 *ctx) {
  uint_t i;
  u64_t C[5], D[5];

  /* Compute the parity of the columns */
  for (i = 0; i < 5; ++i) {
    C[i] =
      ctx->hash[i] ^ ctx->hash[i + 5] ^ ctx->hash[i + 10] ^ ctx->hash[i + 15]
        ^ ctx->hash[i + 20];
  }

  /* Compute the theta effect for a given column */
  D[0] = U_SHA3_ROTL (C[1], 1) ^ C[4];
  D[1] = U_SHA3_ROTL (C[2], 1) ^ C[0];
  D[2] = U_SHA3_ROTL (C[3], 1) ^ C[1];
  D[3] = U_SHA3_ROTL (C[4], 1) ^ C[2];
  D[4] = U_SHA3_ROTL (C[0], 1) ^ C[3];

  /* Add the theta effect to the whole column */
  for (i = 0; i < 5; ++i) {
    ctx->hash[i] ^= D[i];
    ctx->hash[i + 5] ^= D[i];
    ctx->hash[i + 10] ^= D[i];
    ctx->hash[i + 15] ^= D[i];
    ctx->hash[i + 20] ^= D[i];
  }
}

/* Rho and pi steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) */
static void
pp_crypto_hash_sha3_keccak_rho_pi(PHashSHA3 *ctx) {
  u64_t tmp_A;

  /* Unroll the loop over ((0 1)(2 3))^t * (1 0) for 0 ≤ t ≤ 23 */
  tmp_A = ctx->hash[1];
  ctx->hash[1] = U_SHA3_ROTL (ctx->hash[6], 44);
  ctx->hash[6] = U_SHA3_ROTL (ctx->hash[9], 20);
  ctx->hash[9] = U_SHA3_ROTL (ctx->hash[22], 61);
  ctx->hash[22] = U_SHA3_ROTL (ctx->hash[14], 39);
  ctx->hash[14] = U_SHA3_ROTL (ctx->hash[20], 18);
  ctx->hash[20] = U_SHA3_ROTL (ctx->hash[2], 62);
  ctx->hash[2] = U_SHA3_ROTL (ctx->hash[12], 43);
  ctx->hash[12] = U_SHA3_ROTL (ctx->hash[13], 25);
  ctx->hash[13] = U_SHA3_ROTL (ctx->hash[19], 8);
  ctx->hash[19] = U_SHA3_ROTL (ctx->hash[23], 56);
  ctx->hash[23] = U_SHA3_ROTL (ctx->hash[15], 41);
  ctx->hash[15] = U_SHA3_ROTL (ctx->hash[4], 27);
  ctx->hash[4] = U_SHA3_ROTL (ctx->hash[24], 14);
  ctx->hash[24] = U_SHA3_ROTL (ctx->hash[21], 2);
  ctx->hash[21] = U_SHA3_ROTL (ctx->hash[8], 55);
  ctx->hash[8] = U_SHA3_ROTL (ctx->hash[16], 45);
  ctx->hash[16] = U_SHA3_ROTL (ctx->hash[5], 36);
  ctx->hash[5] = U_SHA3_ROTL (ctx->hash[3], 28);
  ctx->hash[3] = U_SHA3_ROTL (ctx->hash[18], 21);
  ctx->hash[18] = U_SHA3_ROTL (ctx->hash[17], 15);
  ctx->hash[17] = U_SHA3_ROTL (ctx->hash[11], 10);
  ctx->hash[11] = U_SHA3_ROTL (ctx->hash[7], 6);
  ctx->hash[7] = U_SHA3_ROTL (ctx->hash[10], 3);
  ctx->hash[10] = U_SHA3_ROTL (tmp_A, 1);
}

/* Chi step (see [Keccak Reference, Section 2.3.1]) */
static void
pp_crypto_hash_sha3_keccak_chi(PHashSHA3 *ctx) {
  uint_t i;
  u64_t tmp_A1, tmp_A2;
  for (i = 0; i < 25; i += 5) {
    tmp_A1 = ctx->hash[i + 0];
    tmp_A2 = ctx->hash[i + 1];
    ctx->hash[i + 0] ^= ~tmp_A2 & ctx->hash[i + 2];
    ctx->hash[i + 1] ^= ~ctx->hash[i + 2] & ctx->hash[i + 3];
    ctx->hash[i + 2] ^= ~ctx->hash[i + 3] & ctx->hash[i + 4];
    ctx->hash[i + 3] ^= ~ctx->hash[i + 4] & tmp_A1;
    ctx->hash[i + 4] ^= ~tmp_A1 & tmp_A2;
  }
}

static void
pp_crypto_hash_sha3_keccak_permutate(PHashSHA3 *ctx) {
  uint_t i;
  for (i = 0; i < 24; ++i) {
    pp_crypto_hash_sha3_keccak_theta(ctx);
    pp_crypto_hash_sha3_keccak_rho_pi(ctx);
    pp_crypto_hash_sha3_keccak_chi(ctx);

    /* Iota step (see [Keccak Reference, Section 2.3.5]) */
    ctx->hash[0] ^= pp_crypto_hash_sha3_K[i];
  }
}

static void
pp_crypto_hash_sha3_process(PHashSHA3 *ctx,
  const u64_t *data) {
  uint_t i;
  uint_t qwords = ctx->block_size / 8;
  for (i = 0; i < qwords; ++i) {
    ctx->hash[i] ^= data[i];
  }

  /* Make the Keccak permutation */
  pp_crypto_hash_sha3_keccak_permutate(ctx);
}

static PHashSHA3 *
pp_crypto_hash_sha3_new_internal(uint_t bits) {
  PHashSHA3 *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(PHashSHA3))) == NULL)) {
    return NULL;
  }
  ret->block_size = (1600 - bits * 2) / 8;
  return ret;
}

void
u_crypto_hash_sha3_reset(PHashSHA3 *ctx) {
  memset(ctx->buf.buf, 0, 200);
  memset(ctx->hash, 0, sizeof(ctx->hash));
  ctx->len = 0;
}

PHashSHA3 *
u_crypto_hash_sha3_224_new(void) {
  return pp_crypto_hash_sha3_new_internal(224);
}

PHashSHA3 *
u_crypto_hash_sha3_256_new(void) {
  return pp_crypto_hash_sha3_new_internal(256);
}

PHashSHA3 *
u_crypto_hash_sha3_384_new(void) {
  return pp_crypto_hash_sha3_new_internal(384);
}

PHashSHA3 *
u_crypto_hash_sha3_512_new(void) {
  return pp_crypto_hash_sha3_new_internal(512);
}

void
u_crypto_hash_sha3_update(PHashSHA3 *ctx,
  const ubyte_t *data,
  size_t len) {
  u32_t left, to_fill;
  left = ctx->len;
  to_fill = ctx->block_size - left;
  ctx->len = (u32_t) (((size_t) ctx->len + len) % (size_t) ctx->block_size);
  if (left && (u64_t) len >= to_fill) {
    memcpy(ctx->buf.buf + left, data, to_fill);
    pp_crypto_hash_sha3_swap_bytes(ctx->buf.buf_w, ctx->block_size >> 3);
    pp_crypto_hash_sha3_process(ctx, ctx->buf.buf_w);
    data += to_fill;
    len -= to_fill;
    left = 0;
  }
  while (len >= ctx->block_size) {
    memcpy(ctx->buf.buf, data, ctx->block_size);
    pp_crypto_hash_sha3_swap_bytes(ctx->buf.buf_w, ctx->block_size >> 3);
    pp_crypto_hash_sha3_process(ctx, ctx->buf.buf_w);
    data += ctx->block_size;
    len -= ctx->block_size;
  }
  if (len > 0) {
    memcpy(ctx->buf.buf + left, data, len);
  }
}

void
u_crypto_hash_sha3_finish(PHashSHA3 *ctx) {
  memset(ctx->buf.buf + ctx->len, 0, ctx->block_size - ctx->len);
  ctx->buf.buf[ctx->len] |= 0x06;
  ctx->buf.buf[ctx->block_size - 1] |= 0x80;
  pp_crypto_hash_sha3_swap_bytes(ctx->buf.buf_w, ctx->block_size >> 3);
  pp_crypto_hash_sha3_process(ctx, ctx->buf.buf_w);
  pp_crypto_hash_sha3_swap_bytes(
    ctx->hash,
    (100 - (ctx->block_size >> 2)) >> 3
  );
}

const ubyte_t *
u_crypto_hash_sha3_digest(PHashSHA3 *ctx) {
  return (const ubyte_t *) ctx->hash;
}

void
u_crypto_hash_sha3_free(PHashSHA3 *ctx) {
  u_free(ctx);
}
