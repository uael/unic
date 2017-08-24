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

/* SHA1 interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHSHA1_H
# define UNIC_HEADER_PCRYPTOHASHSHA1_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashSHA1_ PHashSHA1;

PHashSHA1 *
u_crypto_hash_sha1_new(void);

void
u_crypto_hash_sha1_update(PHashSHA1 *ctx, const ubyte_t *data, size_t len);

void
u_crypto_hash_sha1_finish(PHashSHA1 *ctx);

const ubyte_t *
u_crypto_hash_sha1_digest(PHashSHA1 *ctx);

void
u_crypto_hash_sha1_reset(PHashSHA1 *ctx);

void
u_crypto_hash_sha1_free(PHashSHA1 *ctx);

#endif /* UNIC_HEADER_PCRYPTOHASHSHA1_H */
