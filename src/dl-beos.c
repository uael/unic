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

#include <be/kernel/image.h>

#include "unic/err.h"
#include "unic/file.h"
#include "unic/dl.h"
#include "unic/mem.h"
#include "unic/string.h"

typedef image_id unic_handle;

struct dl {
  unic_handle handle;
  status_t last_status;
};

static void
pp_dl_clean_handle(unic_handle handle);

static void
pp_dl_clean_handle(unic_handle handle) {
  if (U_UNLIKELY (unload_add_on(handle) != B_OK))
    U_ERROR (
      "dl_t::pp_dl_clean_handle: unload_add_on() failed");
}

dl_t *
u_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  unic_handle handle;
  if (!u_file_is_exists(path)) {
    return NULL;
  }
  if (U_UNLIKELY ((handle = load_add_on(path)) == B_ERROR)) {
    U_ERROR ("dl_t::u_dl_new: load_add_on() failed");
    return NULL;
  }
  if (U_UNLIKELY ((loader = u_malloc0(sizeof(dl_t))) == NULL)) {
    U_ERROR ("dl_t::u_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  loader->last_status = B_OK;
  return loader;
}

fn_addr_t
u_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  ptr_t location = NULL;
  status_t status;
  if (U_UNLIKELY (loader == NULL || sym == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY ((
    status = get_image_symbol(
      loader->handle,
      (byte_t *) sym,
      B_SYMBOL_TYPE_ANY,
      &location
    )) != B_OK)) {
    U_ERROR (
      "dl_t::u_dl_get_symbol: get_image_symbol() failed");
    loader->last_status = status;
    return NULL;
  }
  loader->last_status = B_OK;
  return (fn_addr_t) location;
}

void
u_dl_free(dl_t *loader) {
  if (U_UNLIKELY (loader == NULL)) {
    return;
  }
  pp_dl_clean_handle(loader->handle);
  u_free(loader);
}

byte_t *
u_dl_get_last_error(dl_t *loader) {
  if (loader == NULL) {
    return NULL;
  }
  switch (loader->last_status) {
    case B_OK:
      return NULL;
    case B_BAD_IMAGE_ID:
      return u_strdup("Image handler doesn't identify an existing image");
    case B_BAD_INDEX:
      return u_strdup("Invalid symbol index");
    default:
      return u_strdup("Unknown error");
  }
}

bool
u_dl_is_ref_counted(void) {
  return true;
}
