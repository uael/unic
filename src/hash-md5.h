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

/* MD5 interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHMD5_H
# define UNIC_HEADER_PCRYPTOHASHMD5_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashMD5_ PHashMD5;

PHashMD5 *
u_crypto_hash_md5_new(void);

void
u_crypto_hash_md5_update(PHashMD5 *ctx, const ubyte_t *data, size_t len);

void
u_crypto_hash_md5_finish(PHashMD5 *ctx);

const ubyte_t *
u_crypto_hash_md5_digest(PHashMD5 *ctx);

void
u_crypto_hash_md5_reset(PHashMD5 *ctx);

void
u_crypto_hash_md5_free(PHashMD5 *ctx);

#endif /* UNIC_HEADER_PCRYPTOHASHMD5_H */
