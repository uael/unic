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

/* GOST R 34.11-94 interface implementation for #hash_t */

#ifndef UNIC_HEADER_PCRYPTOHASHGOST3411_H
# define UNIC_HEADER_PCRYPTOHASHGOST3411_H

#include "unic/types.h"
#include "unic/macros.h"

typedef struct PHashGOST3411_ PHashGOST3411;

PHashGOST3411 *
u_crypto_hash_gost3411_new(void);

void
u_crypto_hash_gost3411_update(PHashGOST3411 *ctx,
  const ubyte_t *data,
  size_t len);

void
u_crypto_hash_gost3411_finish(PHashGOST3411 *ctx);

const ubyte_t *
u_crypto_hash_gost3411_digest(PHashGOST3411 *ctx);

void
u_crypto_hash_gost3411_reset(PHashGOST3411 *ctx);

void
u_crypto_hash_gost3411_free(PHashGOST3411 *ctx);

#endif /* UNIC_HEADER_PCRYPTOHASHGOST3411_H */
