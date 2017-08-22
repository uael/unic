/*
 * Copyright (C) 2015-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "p/err.h"
#include "p/file.h"
#include "p/dl.h"
#include "p/mem.h"
#include "p/string.h"

typedef HINSTANCE plibrary_handle;

struct dl {
  plibrary_handle handle;
};

static void
pp_dl_clean_handle(plibrary_handle handle);

static void
pp_dl_clean_handle(plibrary_handle handle) {
  if (P_UNLIKELY (!FreeLibrary(handle)))
    P_ERROR (
      "dl_t::pp_dl_clean_handle: FreeLibrary() failed");
}

dl_t *
p_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  plibrary_handle handle;
  if (!p_file_is_exists(path)) {
    return NULL;
  }
  if (P_UNLIKELY ((handle = LoadLibraryA(path)) == NULL)) {
    P_ERROR ("dl_t::p_dl_new: LoadLibraryA() failed");
    return NULL;
  }
  if (P_UNLIKELY ((loader = p_malloc0(sizeof(dl_t))) == NULL)) {
    P_ERROR ("dl_t::p_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  return loader;
}

PFuncAddr
p_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  PFuncAddr ret_sym = NULL;
  if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL)) {
    return NULL;
  }
  ret_sym = (PFuncAddr) GetProcAddress(loader->handle, sym);
  return ret_sym;
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
  byte_t *res = NULL;
  DWORD err_code;
  LPVOID msg_buf;
  P_UNUSED (loader);
  err_code = p_error_get_last_system();
  if (err_code == 0) {
    return NULL;
  }
  if (P_LIKELY (FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    err_code,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR) &msg_buf,
    0,
    NULL
  ) != 0)) {
    res = p_strdup((byte_t *) msg_buf);
    LocalFree(msg_buf);
  }
  return res;
}

bool
p_dl_is_ref_counted(void) {
  return true;
}
