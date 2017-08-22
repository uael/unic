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

#ifndef PLIBSYS_HEADER_PCRYPTOHASHSHA2_512_H
# define PLIBSYS_HEADER_PCRYPTOHASHSHA2_512_H

#include "p/types.h"
#include "p/macros.h"

typedef struct PHashSHA2_512_ PHashSHA2_512;

PHashSHA2_512 *
p_crypto_hash_sha2_512_new(void);

void
p_crypto_hash_sha2_512_update(PHashSHA2_512 *ctx, const ubyte_t *data,
  size_t len);

void
p_crypto_hash_sha2_512_finish(PHashSHA2_512 *ctx);

const ubyte_t *
p_crypto_hash_sha2_512_digest(PHashSHA2_512 *ctx);

void
p_crypto_hash_sha2_512_reset(PHashSHA2_512 *ctx);

void
p_crypto_hash_sha2_512_free(PHashSHA2_512 *ctx);

PHashSHA2_512 *
p_crypto_hash_sha2_384_new(void);

#define p_crypto_hash_sha2_384_update p_crypto_hash_sha2_512_update
#define p_crypto_hash_sha2_384_finish p_crypto_hash_sha2_512_finish
#define p_crypto_hash_sha2_384_digest p_crypto_hash_sha2_512_digest
#define p_crypto_hash_sha2_384_reset  p_crypto_hash_sha2_512_reset
#define p_crypto_hash_sha2_384_free   p_crypto_hash_sha2_512_free
#endif /* PLIBSYS_HEADER_PCRYPTOHASHSHA2_512_H */
