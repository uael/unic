/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

#include <dl.h>
#include <errno.h>

#include "unic/err.h"
#include "unic/file.h"
#include "unic/dl.h"
#include "unic/mem.h"
#include "unic/string.h"

typedef shl_t unic_handle;

struct dl {
  unic_handle handle;
  int last_error;
};

static void
pp_dl_clean_handle(unic_handle handle);

static void
pp_dl_clean_handle(unic_handle handle) {
  if (U_UNLIKELY (shl_unload(handle) != 0))
    U_ERROR (
      "dl_t::pp_dl_clean_handle: shl_unload() failed");
}

dl_t *
u_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  unic_handle handle;
  if (!u_file_is_exists(path)) {
    return NULL;
  }
  if (U_UNLIKELY (
    (handle = shl_load(path, BIND_IMMEDIATE | BIND_NONFATAL | DYNAMIC_PATH, 0))
      == NULL)) {
    U_ERROR ("dl_t::u_dl_new: shl_load() failed");
    return NULL;
  }
  if (U_UNLIKELY ((loader = u_malloc0(sizeof(dl_t))) == NULL)) {
    U_ERROR ("dl_t::u_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  loader->last_error = 0;
  return loader;
}

fn_addr_t
u_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  fn_addr_t func_addr = NULL;
  if (U_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL)) {
    return NULL;
  }
  if (U_UNLIKELY (
    shl_findsym(&loader->handle, sym, TYPE_UNDEFINED, (ptr_t) &func_addr)
      != 0)) {
    U_ERROR (
      "dl_t::u_dl_get_symbol: shl_findsym() failed");
    loader->last_error = (errno == 0 ? -1 : errno);
    return NULL;
  }
  loader->last_error = 0;
  return func_addr;
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
  if (loader->last_error == 0) {
    return NULL;
  } else if (loader->last_error == -1) {
    return u_strdup("Failed to find a symbol");
  } else {
    return u_strdup(strerror(loader->last_error));
  }
}

bool
u_dl_is_ref_counted(void) {
#if defined (U_OS_HPUX) && defined (U_ARCH_HPPA) && (UNIC_SIZEOF_VOID_P == 4)
  return false;
#else
  return true;
#endif
}
