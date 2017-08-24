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

/* SHA2-512 interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHSHA2_512_H
# define UNIC_HEADER_PCRYPTOHASHSHA2_512_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashSHA2_512_ PHashSHA2_512;

PHashSHA2_512 *
u_crypto_hash_sha2_512_new(void);

void
u_crypto_hash_sha2_512_update(PHashSHA2_512 *ctx, const ubyte_t *data,
  size_t len);

void
u_crypto_hash_sha2_512_finish(PHashSHA2_512 *ctx);

const ubyte_t *
u_crypto_hash_sha2_512_digest(PHashSHA2_512 *ctx);

void
u_crypto_hash_sha2_512_reset(PHashSHA2_512 *ctx);

void
u_crypto_hash_sha2_512_free(PHashSHA2_512 *ctx);

PHashSHA2_512 *
u_crypto_hash_sha2_384_new(void);

#define u_crypto_hash_sha2_384_update u_crypto_hash_sha2_512_update
#define u_crypto_hash_sha2_384_finish u_crypto_hash_sha2_512_finish
#define u_crypto_hash_sha2_384_digest u_crypto_hash_sha2_512_digest
#define u_crypto_hash_sha2_384_reset  u_crypto_hash_sha2_512_reset
#define u_crypto_hash_sha2_384_free   u_crypto_hash_sha2_512_free
#endif /* UNIC_HEADER_PCRYPTOHASHSHA2_512_H */
