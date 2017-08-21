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

#include "p/process.h"

#ifndef P_OS_WIN
#  include <sys/types.h>
#  include <signal.h>
#  include <unistd.h>
#endif

P_API uint32_t
p_process_get_current_pid(void) {
#ifdef P_OS_WIN
  return (uint32_t) GetCurrentProcessId ();
#else
  return (uint32_t) getpid();
#endif
}

P_API bool
p_process_is_running(uint32_t pid) {
#ifdef P_OS_WIN
  HANDLE proc;
  DWORD ret;

  if ((proc = OpenProcess (SYNCHRONIZE, false, pid)) == NULL)
    return false;

  ret = WaitForSingleObject (proc, 0);
  CloseHandle (proc);

  return ret == WAIT_TIMEOUT;
#else
  return kill((pid_t) pid, 0) == 0;
#endif
}
