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

#include <string.h>

#include "unic/mem.h"
#include "unic/hash.h"
#include "hash-gost3411.h"
#include "hash-md5.h"
#include "hash-sha1.h"
#include "hash-sha2-256.h"
#include "hash-sha2-512.h"
#include "hash-sha3.h"

#define U_HASH_FUNCS(ctx, type) \
  do { \
    (ctx)->create = (void * (*) (void)) u_crypto_hash_##type##_new; \
    (ctx)->update = (void (*) (void *, const ubyte_t *, size_t)) \
      u_crypto_hash_##type##_update; \
    (ctx)->finish = (void (*) (void *)) u_crypto_hash_##type##_finish; \
    (ctx)->digest = (const ubyte_t * (*) (void *)) \
      u_crypto_hash_##type##_digest; \
    (ctx)->reset = (void (*) (void *)) u_crypto_hash_##type##_reset; \
    (ctx)->free = (void (*) (void *)) u_crypto_hash_##type##_free; \
  } while (0)

struct hash {
  hash_kind_t type;
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
pp_crypto_hash_digest_to_hex(const ubyte_t *digest, uint_t len, byte_t *out) {
  uint_t i;

  for (i = 0; i < len; ++i) {
    *(out + (i << 1)) = pp_crypto_hash_hex_str[(digest[i] >> 4) & 0x0F];
    *(out + (i << 1) + 1) = pp_crypto_hash_hex_str[(digest[i]) & 0x0F];
  }
}

hash_t *
u_crypto_hash_new(hash_kind_t type) {
  hash_t *ret;

  if (type < U_HASH_MD5 || type > U_HASH_GOST) {
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(hash_t))) == NULL)) {
    U_ERROR ("hash_t::u_crypto_hash_new: failed to allocate memory");
    return NULL;
  }
  switch (type) {
    case U_HASH_MD5:
    U_HASH_FUNCS (ret, md5);
      ret->hash_len = 16;
      break;
    case U_HASH_SHA1:
    U_HASH_FUNCS (ret, sha1);
      ret->hash_len = 20;
      break;
    case U_HASH_SHA2_224:
    U_HASH_FUNCS (ret, sha2_224);
      ret->hash_len = 28;
      break;
    case U_HASH_SHA2_256:
    U_HASH_FUNCS (ret, sha2_256);
      ret->hash_len = 32;
      break;
    case U_HASH_SHA2_384:
    U_HASH_FUNCS (ret, sha2_384);
      ret->hash_len = 48;
      break;
    case U_HASH_SHA2_512:
    U_HASH_FUNCS (ret, sha2_512);
      ret->hash_len = 64;
      break;
    case U_HASH_SHA3_224:
    U_HASH_FUNCS (ret, sha3_224);
      ret->hash_len = 28;
      break;
    case U_HASH_SHA3_256:
    U_HASH_FUNCS (ret, sha3_256);
      ret->hash_len = 32;
      break;
    case U_HASH_SHA3_384:
    U_HASH_FUNCS (ret, sha3_384);
      ret->hash_len = 48;
      break;
    case U_HASH_SHA3_512:
    U_HASH_FUNCS (ret, sha3_512);
      ret->hash_len = 64;
      break;
    case U_HASH_GOST:
    U_HASH_FUNCS (ret, gost3411);
      ret->hash_len = 32;
      break;
  }
  ret->type = type;
  ret->closed = false;
  if (U_UNLIKELY ((ret->context = ret->create()) == NULL)) {
    u_free(ret);
    return NULL;
  }
  return ret;
}

void
u_crypto_hash_update(hash_t *hash, const ubyte_t *data, size_t len) {
  if (U_UNLIKELY (hash == NULL || data == NULL || len == 0)) {
    return;
  }
  if (U_UNLIKELY (hash->closed)) {
    return;
  }
  hash->update(hash->context, data, len);
}

void
u_crypto_hash_reset(hash_t *hash) {
  if (U_UNLIKELY (hash == NULL)) {
    return;
  }
  hash->reset(hash->context);
  hash->closed = false;
}

byte_t *
u_crypto_hash_get_string(hash_t *hash) {
  byte_t *ret;
  const ubyte_t *digest;

  if (U_UNLIKELY (hash == NULL)) {
    return NULL;
  }
  if (!hash->closed) {
    hash->finish(hash->context);
    hash->closed = true;
  }
  if (U_UNLIKELY ((digest = hash->digest(hash->context)) == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((ret = u_malloc0(hash->hash_len * 2 + 1)) == NULL)) {
    return NULL;
  }
  pp_crypto_hash_digest_to_hex(digest, hash->hash_len, ret);
  return ret;
}

void
u_crypto_hash_get_digest(hash_t *hash, ubyte_t *buf, size_t *len) {
  const ubyte_t *digest;

  if (U_UNLIKELY (len == NULL)) {
    return;
  }
  if (U_UNLIKELY (hash == NULL || buf == NULL)) {
    *len = 0;
    return;
  }
  if (U_UNLIKELY (hash->hash_len > *len)) {
    *len = 0;
    return;
  }
  if (!hash->closed) {
    hash->finish(hash->context);
    hash->closed = true;
  }
  if (U_UNLIKELY ((digest = hash->digest(hash->context)) == NULL)) {
    *len = 0;
    return;
  }
  memcpy(buf, digest, hash->hash_len);
  *len = hash->hash_len;
}

ssize_t
u_crypto_hash_get_length(const hash_t *hash) {
  if (U_UNLIKELY (hash == NULL)) {
    return 0;
  }
  return hash->hash_len;
}

hash_kind_t
u_crypto_hash_get_type(const hash_t *hash) {
  if (U_UNLIKELY (hash == NULL)) {
    return (hash_kind_t) -1;
  }
  return hash->type;
}

void
u_crypto_hash_free(hash_t *hash) {
  if (U_UNLIKELY (hash == NULL)) {
    return;
  }
  hash->free(hash->context);
  u_free(hash);
}
