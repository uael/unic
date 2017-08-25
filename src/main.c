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

#include "unic/mem.h"
#include "unic/main.h"

extern void
u_mem_init(void);

extern void
u_mem_shutdown(void);

extern void
u_atomic_thread_init(void);

extern void
u_atomic_thread_shutdown(void);

extern void
u_socket_init_once(void);

extern void
u_socket_close_once(void);

extern void
u_thread_init(void);

extern void
u_thread_shutdown(void);

extern void
u_condvar_init(void);

extern void
u_condvar_shutdown(void);

extern void
u_rwlock_init(void);

extern void
u_rwlock_shutdown(void);

extern void
u_profiler_init(void);

extern void
u_profiler_shutdown(void);

static bool pp_unic_inited = false;

static byte_t pp_unic_version[] = UNIC_VERSION_STR;

void
u_libsys_init(void) {
  if (U_UNLIKELY (pp_unic_inited == true)) {
    return;
  }
  pp_unic_inited = true;
  u_mem_init();
  u_atomic_thread_init();
  u_socket_init_once();
  u_thread_init();
  u_condvar_init();
  u_rwlock_init();
  u_profiler_init();
}

void
u_libsys_init_full(const mem_vtable_t *vtable) {
  if (u_mem_set_vtable(vtable) == false)
    U_ERROR ("MAIN::u_libsys_init_full: failed to initialize memory table");
  u_libsys_init();
}

void
u_libsys_shutdown(void) {
  if (U_UNLIKELY (pp_unic_inited == false)) {
    return;
  }
  pp_unic_inited = false;
  u_profiler_shutdown();
  u_rwlock_shutdown();
  u_condvar_shutdown();
  u_thread_shutdown();
  u_socket_close_once();
  u_atomic_thread_shutdown();
  u_mem_shutdown();
}

const byte_t *
u_libsys_version(void) {
  return (const byte_t *) pp_unic_version;
}

#ifdef U_OS_WIN

extern void
u_thread_win32_thread_detach(void);

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  U_UNUSED (hinstDLL);
  U_UNUSED (lpvReserved);
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      u_thread_win32_thread_detach();
      break;
    case DLL_PROCESS_DETACH:
      break;
    default:;
  }
  return true;
}

#endif
