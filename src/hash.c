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

#include "p/mem.h"
#include "p/hash.h"
#include "pcryptohash-gost3411.h"
#include "pcryptohash-md5.h"
#include "pcryptohash-sha1.h"
#include "pcryptohash-sha2-256.h"
#include "pcryptohash-sha2-512.h"
#include "pcryptohash-sha3.h"

#include <string.h>

#define P_HASH_FUNCS(ctx, type) \
  ctx->create = (void * (*) (void)) p_crypto_hash_##type##_new;        \
  ctx->update = (void (*) (void *, const ubyte_t *, size_t)) p_crypto_hash_##type##_update;  \
  ctx->finish = (void (*) (void *)) p_crypto_hash_##type##_finish;      \
  ctx->digest = (const ubyte_t * (*) (void *)) p_crypto_hash_##type##_digest;    \
  ctx->reset = (void (*) (void *)) p_crypto_hash_##type##_reset;        \
  ctx->free = (void (*) (void *)) p_crypto_hash_##type##_free;

struct PCryptoHash_ {
  PCryptoHashType type;
  ptr_t context;
  uint_t hash_len;
  bool closed;
  ptr_t (*create)(void);
  void (*update)(void *hash, const ubyte_t *data, size_t len);
  void (*finish)(void *hash);
  const ubyte_t *(*digest)(void *hash);
  void (*reset)(void *hash);
  void (*free)(void *hash);
};

static byte_t pp_crypto_hash_hex_str[] = "0123456789abcdef";

static void
pp_crypto_hash_digest_to_hex(const ubyte_t *digest, uint_t len, byte_t *out);

static void
pp_crypto_hash_digest_to_hex(const ubyte_t *digest, uint_t len, byte_t *out) {
  uint_t i;

  for (i = 0; i < len; ++i) {
    *(out + (i << 1)) = pp_crypto_hash_hex_str[(digest[i] >> 4) & 0x0F];
    *(out + (i << 1) + 1) = pp_crypto_hash_hex_str[(digest[i]) & 0x0F];
  }
}

P_API PCryptoHash *
p_crypto_hash_new(PCryptoHashType type) {
  PCryptoHash *ret;

  if (type < P_HASH_TYPE_MD5 || type > P_HASH_TYPE_GOST)
    return NULL;

  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PCryptoHash))) == NULL)) {
    P_ERROR ("PCryptoHash::p_crypto_hash_new: failed to allocate memory");
    return NULL;
  }

  switch (type) {
    case P_HASH_TYPE_MD5:
    P_HASH_FUNCS (ret, md5);
      ret->hash_len = 16;
      break;
    case P_HASH_TYPE_SHA1:
    P_HASH_FUNCS (ret, sha1);
      ret->hash_len = 20;
      break;
    case P_HASH_TYPE_SHA2_224:
    P_HASH_FUNCS (ret, sha2_224);
      ret->hash_len = 28;
      break;
    case P_HASH_TYPE_SHA2_256:
    P_HASH_FUNCS (ret, sha2_256);
      ret->hash_len = 32;
      break;
    case P_HASH_TYPE_SHA2_384:
    P_HASH_FUNCS (ret, sha2_384);
      ret->hash_len = 48;
      break;
    case P_HASH_TYPE_SHA2_512:
    P_HASH_FUNCS (ret, sha2_512);
      ret->hash_len = 64;
      break;
    case P_HASH_TYPE_SHA3_224:
    P_HASH_FUNCS (ret, sha3_224);
      ret->hash_len = 28;
      break;
    case P_HASH_TYPE_SHA3_256:
    P_HASH_FUNCS (ret, sha3_256);
      ret->hash_len = 32;
      break;
    case P_HASH_TYPE_SHA3_384:
    P_HASH_FUNCS (ret, sha3_384);
      ret->hash_len = 48;
      break;
    case P_HASH_TYPE_SHA3_512:
    P_HASH_FUNCS (ret, sha3_512);
      ret->hash_len = 64;
      break;
    case P_HASH_TYPE_GOST:
    P_HASH_FUNCS (ret, gost3411);
      ret->hash_len = 32;
      break;
  }

  ret->type = type;
  ret->closed = false;

  if (P_UNLIKELY ((ret->context = ret->create()) == NULL)) {
    p_free(ret);
    return NULL;
  }

  return ret;
}

P_API void
p_crypto_hash_update(PCryptoHash *hash, const ubyte_t *data, size_t len) {
  if (P_UNLIKELY (hash == NULL || data == NULL || len == 0))
    return;

  if (P_UNLIKELY (hash->closed))
    return;

  hash->update(hash->context, data, len);
}

P_API void
p_crypto_hash_reset(PCryptoHash *hash) {
  if (P_UNLIKELY (hash == NULL))
    return;

  hash->reset(hash->context);
  hash->closed = false;
}

P_API byte_t *
p_crypto_hash_get_string(PCryptoHash *hash) {
  byte_t *ret;
  const ubyte_t *digest;

  if (P_UNLIKELY (hash == NULL))
    return NULL;

  if (!hash->closed) {
    hash->finish(hash->context);
    hash->closed = true;
  }

  if (P_UNLIKELY ((digest = hash->digest(hash->context)) == NULL))
    return NULL;

  if (P_UNLIKELY ((ret = p_malloc0(hash->hash_len * 2 + 1)) == NULL))
    return NULL;

  pp_crypto_hash_digest_to_hex(digest, hash->hash_len, ret);

  return ret;
}

P_API void
p_crypto_hash_get_digest(PCryptoHash *hash, ubyte_t *buf, size_t *len) {
  const ubyte_t *digest;

  if (P_UNLIKELY (len == NULL))
    return;

  if (P_UNLIKELY (hash == NULL || buf == NULL)) {
    *len = 0;
    return;
  }

  if (P_UNLIKELY (hash->hash_len > *len)) {
    *len = 0;
    return;
  }

  if (!hash->closed) {
    hash->finish(hash->context);
    hash->closed = true;
  }

  if (P_UNLIKELY ((digest = hash->digest(hash->context)) == NULL)) {
    *len = 0;
    return;
  }

  memcpy(buf, digest, hash->hash_len);
  *len = hash->hash_len;
}

P_API ssize_t
p_crypto_hash_get_length(const PCryptoHash *hash) {
  if (P_UNLIKELY (hash == NULL))
    return 0;

  return hash->hash_len;
}

P_API PCryptoHashType
p_crypto_hash_get_type(const PCryptoHash *hash) {
  if (P_UNLIKELY (hash == NULL))
    return (PCryptoHashType) -1;

  return hash->type;
}

P_API void
p_crypto_hash_free(PCryptoHash *hash) {
  if (P_UNLIKELY (hash == NULL))
    return;

  hash->free(hash->context);
  p_free(hash);
}