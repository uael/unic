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

/* SHA2-256 interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHSHA2_256_H
# define UNIC_HEADER_PCRYPTOHASHSHA2_256_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashSHA2_256_ PHashSHA2_256;

PHashSHA2_256 *
u_crypto_hash_sha2_256_new(void);

void
u_crypto_hash_sha2_256_update(PHashSHA2_256 *ctx, const ubyte_t *data,
  size_t len);

void
u_crypto_hash_sha2_256_finish(PHashSHA2_256 *ctx);

const ubyte_t *
u_crypto_hash_sha2_256_digest(PHashSHA2_256 *ctx);

void
u_crypto_hash_sha2_256_reset(PHashSHA2_256 *ctx);

void
u_crypto_hash_sha2_256_free(PHashSHA2_256 *ctx);

PHashSHA2_256 *
u_crypto_hash_sha2_224_new(void);

#define u_crypto_hash_sha2_224_update u_crypto_hash_sha2_256_update
#define u_crypto_hash_sha2_224_finish u_crypto_hash_sha2_256_finish
#define u_crypto_hash_sha2_224_digest u_crypto_hash_sha2_256_digest
#define u_crypto_hash_sha2_224_reset  u_crypto_hash_sha2_256_reset
#define u_crypto_hash_sha2_224_free   u_crypto_hash_sha2_256_free
#endif /* UNIC_HEADER_PCRYPTOHASHSHA2_256_H */
