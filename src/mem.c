/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include <string.h>
#include <stdlib.h>

#include "p/err.h"
#include "p/mem.h"
#include "err-private.h"

#ifndef P_OS_WIN
# if defined (P_OS_BEOS)
#   include <be/kernel/OS.h>
# elif defined (P_OS_OS2)
# define INCL_DOSMEMMGR
# define INCL_DOSERRORS
# include <os2.h>
# else
#   include <sys/mman.h>
# endif
#endif

static bool p_mem_table_inited = false;

static mem_vtable_t p_mem_table;

void
p_mem_init(void) {
  if (P_UNLIKELY (p_mem_table_inited == true)) {
    return;
  }
  p_mem_restore_vtable();
}

void
p_mem_shutdown(void) {
  if (P_UNLIKELY (!p_mem_table_inited)) {
    return;
  }
  p_mem_table.malloc = NULL;
  p_mem_table.realloc = NULL;
  p_mem_table.free = NULL;
  p_mem_table_inited = false;
}

ptr_t
p_malloc(size_t n_bytes) {
  if (P_LIKELY (n_bytes > 0)) {
    return p_mem_table.malloc(n_bytes);
  } else {
    return NULL;
  }
}

ptr_t
p_malloc0(size_t n_bytes) {
  ptr_t ret;

  if (P_LIKELY (n_bytes > 0)) {
    if (P_UNLIKELY ((ret = p_mem_table.malloc(n_bytes)) == NULL)) {
      return NULL;
    }
    memset(ret, 0, n_bytes);
    return ret;
  } else {
    return NULL;
  }
}

ptr_t
p_realloc(ptr_t mem, size_t n_bytes) {
  if (P_UNLIKELY (n_bytes == 0)) {
    return NULL;
  }
  if (P_UNLIKELY (mem == NULL)) {
    return p_mem_table.malloc(n_bytes);
  } else {
    return p_mem_table.realloc(mem, n_bytes);
  }
}

void
p_free(ptr_t mem) {
  if (P_LIKELY (mem != NULL)) {
    p_mem_table.free(mem);
  }
}

bool
p_mem_set_vtable(const mem_vtable_t *table) {
  if (P_UNLIKELY (table == NULL)) {
    return false;
  }
  if (P_UNLIKELY (
    table->free == NULL || table->malloc == NULL || table->realloc == NULL)) {
      return false;
  }
  p_mem_table.malloc = table->malloc;
  p_mem_table.realloc = table->realloc;
  p_mem_table.free = table->free;
  p_mem_table_inited = true;
  return true;
}

void
p_mem_restore_vtable(void) {
  p_mem_table.malloc = (ptr_t (*)(size_t)) malloc;
  p_mem_table.realloc = (ptr_t (*)(ptr_t, size_t)) realloc;
  p_mem_table.free = (void (*)(ptr_t)) free;
  p_mem_table_inited = true;
}

ptr_t
p_mem_mmap(size_t n_bytes, err_t **error) {
  ptr_t addr;
#if defined (P_OS_WIN)
  HANDLE hdl;
#elif defined (P_OS_BEOS)
  area_id  area;
#elif defined (P_OS_OS2)
  APIRET  ulrc;
#else
  int fd;
  int map_flags = MAP_PRIVATE;
#endif

  if (P_UNLIKELY (n_bytes == 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return NULL;
  }
#if defined (P_OS_WIN)
  if (P_UNLIKELY ((
    hdl = CreateFileMappingA(
      INVALID_HANDLE_VALUE,
      NULL,
      PAGE_READWRITE,
      0,
      (DWORD) n_bytes,
      NULL
    )) == NULL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call CreateFileMapping() to create file mapping"
    );
    return NULL;
  }
  if (P_UNLIKELY ((
    addr = MapViewOfFile(
      hdl,
      FILE_MAP_READ | FILE_MAP_WRITE,
      0,
      0,
      n_bytes
    )) == NULL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call MapViewOfFile() to map file view"
    );
    CloseHandle(hdl);
    return NULL;
  }
  if (P_UNLIKELY (!CloseHandle(hdl))) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call CloseHandle() to close file mapping"
    );
    UnmapViewOfFile(addr);
    return NULL;
  }
#elif defined (P_OS_BEOS)
  if (P_LIKELY ((n_bytes % B_PAGE_SIZE)) != 0)
    n_bytes = (n_bytes / B_PAGE_SIZE + 1) * B_PAGE_SIZE;

  area = create_area ("", &addr, B_ANY_ADDRESS, n_bytes, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);

  if (P_UNLIKELY (area < B_NO_ERROR)) {
    p_err_set_err_p (error,
             (int) p_err_get_last_io (),
             p_err_get_last_system (),
             "Failed to call create_area() to create memory area");
    return NULL;
  }
#elif defined (P_OS_OS2)
  if (P_UNLIKELY ((ulrc = DosAllocMem ((PPVOID) &addr,
               (ULONG) n_bytes,
               PAG_READ | PAG_WRITE | PAG_COMMIT |
               OBJ_ANY)) != NO_ERROR)) {
    /* Try to remove OBJ_ANY */
    if (P_UNLIKELY ((ulrc = DosAllocMem ((PPVOID) &addr,
                 (ULONG) n_bytes,
                 PAG_READ | PAG_WRITE)) != NO_ERROR)) {
      p_err_set_err_p (error,
               (int) p_err_get_io_from_system ((int) ulrc),
               ulrc,
               "Failed to call DosAllocMemory() to alocate memory");
      return NULL;
    }
  }
#else
# if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
  if (P_UNLIKELY ((fd = open ("/dev/zero", O_RDWR | O_EXCL, 0754)) == -1)) {
    p_err_set_err_p (error,
             (int) p_err_get_last_io (),
             p_err_get_last_system (),
             "Failed to open /dev/zero for file mapping");
    return NULL;
  }
# else
  fd = -1;
# endif

# ifdef PLIBSYS_MMAP_HAS_MAP_ANONYMOUS
  map_flags |= MAP_ANONYMOUS;
# elif defined (PLIBSYS_MMAP_HAS_MAP_ANON)
  map_flags |= MAP_ANON;
# endif

  if (P_UNLIKELY ((addr = mmap(NULL,
    n_bytes,
    PROT_READ | PROT_WRITE,
    map_flags,
    fd,
    0)) == (void *) -1)) {
    p_err_set_err_p(error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call mmap() to create file mapping");
# if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
    if (P_UNLIKELY (p_sys_close (fd) != 0))
      P_WARNING ("PMem::p_mem_mmap: failed to close file descriptor to /dev/zero");
# endif
    return NULL;
  }

# if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
  if (P_UNLIKELY (p_sys_close (fd) != 0)) {
    p_err_set_err_p (error,
             (int) p_err_get_last_io (),
             p_err_get_last_system (),
             "Failed to close /dev/zero handle");
    munmap (addr, n_bytes);
    return NULL;
  }
# endif
#endif
  return addr;
}

bool
p_mem_munmap(ptr_t mem, size_t n_bytes, err_t **error) {
#if defined (P_OS_BEOS)
  area_id area;
#elif defined (P_OS_OS2)
  APIRET ulrc;
#endif

  if (P_UNLIKELY (mem == NULL || n_bytes == 0)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
#if defined (P_OS_WIN)
  if (P_UNLIKELY (UnmapViewOfFile(mem) == 0)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to call UnmapViewOfFile() to remove file mapping"
    );
#elif defined (P_OS_BEOS)
    if (P_UNLIKELY ((area = area_for (mem)) == B_ERROR)) {
      p_err_set_err_p (error,
               (int) p_err_get_last_io (),
               p_err_get_last_system (),
               "Failed to call area_for() to find allocated memory area");
      return false;
    }

    if (P_UNLIKELY ((delete_area (area)) != B_OK)) {
      p_err_set_err_p (error,
               (int) p_err_get_last_io (),
               p_err_get_last_system (),
               "Failed to call delete_area() to remove memory area");
#elif defined (P_OS_OS2)
    if (P_UNLIKELY ((ulrc = DosFreeMem ((PVOID) mem)) != NO_ERROR)) {
      p_err_set_err_p (error,
               (int) p_err_get_io_from_system ((int) ulrc),
               ulrc,
               "Failed to call DosFreeMem() to free memory");
#else
    if (P_UNLIKELY (munmap(mem, n_bytes) != 0)) {
      p_err_set_err_p(error,
        (int) p_err_get_last_io(),
        p_err_get_last_system(),
        "Failed to call munmap() to remove file mapping");
#endif
    return false;
  } else {
    return true;
  }
}
