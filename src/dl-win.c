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

#include "unic/err.h"
#include "unic/file.h"
#include "unic/dl.h"
#include "unic/mem.h"
#include "unic/string.h"

typedef HINSTANCE unic_handle;

struct dl {
  unic_handle handle;
};

static void
pp_dl_clean_handle(unic_handle handle);

static void
pp_dl_clean_handle(unic_handle handle) {
  if (U_UNLIKELY (!FreeLibrary(handle)))
    U_ERROR ("dl_t::pp_dl_clean_handle: FreeLibrary() failed");
}

dl_t *
u_dl_new(const byte_t *path) {
  dl_t *loader;
  unic_handle handle;

  if (!u_file_is_exists(path)) {
    return NULL;
  }
  if (U_UNLIKELY ((handle = LoadLibraryA(path)) == NULL)) {
    U_ERROR ("dl_t::u_dl_new: LoadLibraryA() failed");
    return NULL;
  }
  if (U_UNLIKELY ((loader = u_malloc0(sizeof(dl_t))) == NULL)) {
    U_ERROR ("dl_t::u_dl_new: failed to allocate memory");
    pp_dl_clean_handle(handle);
    return NULL;
  }
  loader->handle = handle;
  return loader;
}

fn_addr_t
u_dl_get_symbol(dl_t *loader, const byte_t *sym) {
  if (U_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL)) {
    return NULL;
  }
  return (fn_addr_t) GetProcAddress(loader->handle, sym);
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
  byte_t *res;
  DWORD err_code;
  LPVOID msg_buf;

  res = NULL;
  U_UNUSED (loader);
  err_code = (DWORD) u_err_get_last_system();
  if (err_code == 0) {
    return NULL;
  }
  if (U_LIKELY (FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    err_code,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR) & msg_buf,
    0,
    NULL
  ) != 0)) {
    res = u_strdup((byte_t *) msg_buf);
    LocalFree(msg_buf);
  }
  return res;
}

bool
u_dl_is_ref_counted(void) {
  return true;
}
