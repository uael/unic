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

#include <dlfcn.h>

#include "unic/err.h"
#include "unic/file.h"
#include "unic/dl.h"
#include "unic/mem.h"
#include "unic/string.h"

/* FreeBSD may cause a segfault: https://reviews.freebsd.org/D5112,
 * DragonFlyBSD as well, so we need to check a file size before calling dlopen()
 */
#if defined (U_OS_FREEBSD) || defined (U_OS_DRAGONFLY)
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

typedef ptr_t unic_handle;

struct dl {
  unic_handle handle;
};

static void
pp_dl_clean_handle(unic_handle handle);

static void
pp_dl_clean_handle(unic_handle handle) {
  if (U_UNLIKELY (dlclose(handle) != 0))
    U_ERROR (
      "dl_t::pp_dl_clean_handle: dlclose() failed");
}

dl_t *
u_dl_new(const byte_t *path) {
  dl_t *loader = NULL;
  unic_handle handle;
#if defined (U_OS_FREEBSD) || defined (U_OS_DRAGONFLY)
  struct stat stat_buf;
#endif
  if (!u_file_is_exists(path)) {
    return NULL;
  }
#if defined (U_OS_FREEBSD) || defined (U_OS_DRAGONFLY)
  if (U_UNLIKELY (stat (path, &stat_buf) != 0)) {
    U_ERROR ("dl_t::u_dl_new: stat() failed");
    return NULL;
  }

  if (U_UNLIKELY (stat_buf.st_size == 0)) {
    U_ERROR ("dl_t::u_dl_new: unable to handle zero-size file");
    return NULL;
  }
#endif
  if (U_UNLIKELY ((handle = dlopen(path, RTLD_NOW)) == NULL)) {
    U_ERROR ("dl_t::u_dl_new: dlopen() failed");
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
  return (fn_addr_t) dlsym(loader->handle, sym);
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
  byte_t *res = NULL;
  byte_t *msg;
  U_UNUSED (loader);
  msg = dlerror();
  if (msg != NULL) {
    res = u_strdup(msg);
  }
  return res;
}

bool
u_dl_is_ref_counted(void) {
  return true;
}
