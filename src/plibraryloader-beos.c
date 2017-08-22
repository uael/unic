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

#include "p/err.h"
#include "p/file.h"
#include "p/dl.h"
#include "p/mem.h"
#include "p/string.h"

typedef image_id plibrary_handle;

struct dl {
  plibrary_handle handle;
  status_t last_status;
};

static void
pp_dl_clean_handle(plibrary_handle handle);

static void
pp_dl_clean_handle(plibrary_handle handle) {
  if (P_UNLIKELY (unload_add_on(handle) != B_OK))
    P_ERROR (
      "dl_t::pp_dl_clean_handle: unload_add_on() failed");
}

dl_t *
p_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  plibrary_handle handle;
  if (!p_file_is_exists(path)) {
    return NULL;
  }
  if (P_UNLIKELY ((handle = load_add_on(path)) == B_ERROR)) {
    P_ERROR ("dl_t::p_dl_new: load_add_on() failed");
    return NULL;
  }
  if (P_UNLIKELY ((loader = p_malloc0(sizeof(dl_t))) == NULL)) {
    P_ERROR ("dl_t::p_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  loader->last_status = B_OK;
  return loader;
}

PFuncAddr
p_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  ptr_t location = NULL;
  status_t status;
  if (P_UNLIKELY (loader == NULL || sym == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((
    status = get_image_symbol(
      loader->handle,
      (byte_t *) sym,
      B_SYMBOL_TYPE_ANY,
      &location
    )) != B_OK)) {
    P_ERROR (
      "dl_t::p_dl_get_symbol: get_image_symbol() failed");
    loader->last_status = status;
    return NULL;
  }
  loader->last_status = B_OK;
  return (PFuncAddr) location;
}

void
p_dl_free(dl_t *loader) {
  if (P_UNLIKELY (loader == NULL)) {
    return;
  }
  pp_dl_clean_handle(loader->handle);
  p_free(loader);
}

byte_t *
p_dl_get_last_error(dl_t *loader) {
  if (loader == NULL) {
    return NULL;
  }
  switch (loader->last_status) {
    case B_OK:
      return NULL;
    case B_BAD_IMAGE_ID:
      return p_strdup("Image handler doesn't identify an existing image");
    case B_BAD_INDEX:
      return p_strdup("Invalid symbol index");
    default:
      return p_strdup("Unknown error");
  }
}

bool
p_dl_is_ref_counted(void) {
  return true;
}
