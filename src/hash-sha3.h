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

/* SHA-3 (Keccak) interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHSHA3_H
# define UNIC_HEADER_PCRYPTOHASHSHA3_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashSHA3_ PHashSHA3;

void
u_crypto_hash_sha3_update(PHashSHA3 *ctx, const ubyte_t *data, size_t len);

void
u_crypto_hash_sha3_finish(PHashSHA3 *ctx);

const ubyte_t *
u_crypto_hash_sha3_digest(PHashSHA3 *ctx);

void
u_crypto_hash_sha3_reset(PHashSHA3 *ctx);

void
u_crypto_hash_sha3_free(PHashSHA3 *ctx);

PHashSHA3 *
u_crypto_hash_sha3_224_new(void);

PHashSHA3 *
u_crypto_hash_sha3_256_new(void);

PHashSHA3 *
u_crypto_hash_sha3_384_new(void);

PHashSHA3 *
u_crypto_hash_sha3_512_new(void);

#define u_crypto_hash_sha3_224_update u_crypto_hash_sha3_update
#define u_crypto_hash_sha3_224_finish u_crypto_hash_sha3_finish
#define u_crypto_hash_sha3_224_digest u_crypto_hash_sha3_digest
#define u_crypto_hash_sha3_224_reset  u_crypto_hash_sha3_reset
#define u_crypto_hash_sha3_224_free   u_crypto_hash_sha3_free
#define u_crypto_hash_sha3_256_update u_crypto_hash_sha3_update
#define u_crypto_hash_sha3_256_finish u_crypto_hash_sha3_finish
#define u_crypto_hash_sha3_256_digest u_crypto_hash_sha3_digest
#define u_crypto_hash_sha3_256_reset  u_crypto_hash_sha3_reset
#define u_crypto_hash_sha3_256_free   u_crypto_hash_sha3_free
#define u_crypto_hash_sha3_384_update u_crypto_hash_sha3_update
#define u_crypto_hash_sha3_384_finish u_crypto_hash_sha3_finish
#define u_crypto_hash_sha3_384_digest u_crypto_hash_sha3_digest
#define u_crypto_hash_sha3_384_reset  u_crypto_hash_sha3_reset
#define u_crypto_hash_sha3_384_free   u_crypto_hash_sha3_free
#define u_crypto_hash_sha3_512_update u_crypto_hash_sha3_update
#define u_crypto_hash_sha3_512_finish u_crypto_hash_sha3_finish
#define u_crypto_hash_sha3_512_digest u_crypto_hash_sha3_digest
#define u_crypto_hash_sha3_512_reset  u_crypto_hash_sha3_reset
#define u_crypto_hash_sha3_512_free   u_crypto_hash_sha3_free
#endif /* UNIC_HEADER_PCRYPTOHASHSHA3_H */
