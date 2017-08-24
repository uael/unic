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

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include "p/err.h"
#include "p/file.h"
#include "p/dl.h"
#include "p/mem.h"
#include "p/string.h"

typedef HMODULE plibrary_handle;

struct dl {
  plibrary_handle handle;
  APIRET last_error;
};

static void
pp_dl_clean_handle(plibrary_handle handle);

static void
pp_dl_clean_handle(plibrary_handle handle) {
  APIRET ulrc;
  while ((ulrc = DosFreeModule(handle)) == ERROR_INTERRUPT);
  if (P_UNLIKELY (ulrc != NO_ERROR))
    P_ERROR (
      "dl_t::pp_dl_clean_handle: DosFreeModule() failed");
}

dl_t *
p_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  plibrary_handle handle = NULLHANDLE;
  UCHAR load_err[256];
  APIRET ulrc;
  if (!p_file_is_exists(path)) {
    return NULL;
  }
  while ((
    ulrc = DosLoadModule((PSZ) load_err,
      sizeof(load_err),
      (PSZ) path,
      (PHMODULE) & handle
    )) == ERROR_INTERRUPT) {}
  if (P_UNLIKELY (ulrc != NO_ERROR)) {
    P_ERROR ("dl_t::p_dl_new: DosLoadModule() failed");
    return NULL;
  }
  if (P_UNLIKELY ((loader = p_malloc0(sizeof(dl_t))) == NULL)) {
    P_ERROR ("dl_t::p_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  loader->last_error = NO_ERROR;
  return loader;
}

fn_addr_t
p_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  PFN func_addr = NULL;
  APIRET ulrc;
  if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY (
    (ulrc = DosQueryProcAddr(loader->handle, 0, (PSZ) sym, &func_addr))
      != NO_ERROR)) {
    P_ERROR (
      "dl_t::p_dl_get_symbol: DosQueryProcAddr() failed");
    loader->last_error = ulrc;
    return NULL;
  }
  loader->last_error = NO_ERROR;
  return (fn_addr_t) func_addr;
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
  switch (loader->last_error) {
    case NO_ERROR:
      return NULL;
    case ERROR_INVALID_HANDLE:
      return p_strdup("Invalid resource handler");
    case ERROR_INVALID_NAME:
      return p_strdup("Invalid procedure name");
    default:
      return p_strdup("Unknown error");
  }
}

bool
p_dl_is_ref_counted(void) {
  return true;
}
