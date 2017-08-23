/*
 * Copyright (C) 2016-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "p/dl.h"

dl_t *
p_dl_new(const byte_t *path) {
  P_ERROR ("dl_t::p_dl_new: not implemented");
  return NULL;
}

PFuncAddr
p_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  P_UNUSED (loader);
  P_UNUSED (sym);
  P_ERROR ("dl_t::p_dl_get_symbol: not implemented");
  return NULL;
}

void
p_dl_free(dl_t *loader) {
  P_UNUSED (loader);
  P_ERROR ("dl_t::p_dl_free: not implemented");
}

byte_t *
p_dl_get_last_error(dl_t *loader) {
  P_UNUSED (loader);
  P_ERROR ("dl_t::p_dl_get_last_error: not implemented");
  return NULL;
}

bool
p_dl_is_ref_counted(void) {
  return false;
}
