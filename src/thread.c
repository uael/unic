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

#include <time.h>

#include "unic/atomic.h"

#ifndef U_OS_WIN
# include "unic/err.h"
#endif

#include "unic/mem.h"
#include "unic/spinlock.h"
#include "unic/thread.h"
#include "unic/string.h"
#include "thread-private.h"

#ifdef U_OS_OS2
# define INCL_DOSPROCESS
# define INCL_DOSERRORS
# define INCL_DOSMISC
# include <os2.h>
#endif

#ifndef U_OS_WIN
# include <unistd.h>
#endif

#ifdef U_OS_WIN
typedef void (WINAPI *SystemInfoFunc)(LPSYSTEM_INFO);
#endif

#ifdef U_OS_HPUX
# include <sys/pstat.h>
#endif

#ifdef U_OS_BSD4
# include <sys/param.h>
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#ifdef U_OS_VMS
# define __NEW_STARLET 1
# include <starlet.h>
# include <ssdef.h>
# include <stsdef.h>
# include <efndef.h>
# include <iledef.h>
# include <iosbdef.h>
# include <syidef.h>
# include <tis.h>
# include <lib$routines.h>
#endif

#ifdef U_OS_QNX6
# include <sys/syspage.h>
#endif

#ifdef U_OS_BEOS
# include <kernel/OS.h>
#endif

#ifdef U_OS_SYLLABLE
# include <atheos/sysinfo.h>
#endif

#if defined (U_OS_SCO) && !defined (_SC_NPROCESSORS_ONLN)
# include <sys/utsname.h>
#endif

extern void
u_thread_init_internal(void);

extern void
u_thread_shutdown_internal(void);

extern void
u_thread_exit_internal(void);

extern void
u_thread_wait_internal(thread_t *thread);

extern void
u_thread_free_internal(thread_t *thread);

extern thread_t *
u_thread_create_internal(thread_fn_t func,
  bool joinable,
  thread_prio_t prio,
  size_t stack_size);

static void
pp_thread_cleanup(ptr_t data);

static ptr_t
pp_thread_proxy(ptr_t data);

#ifndef U_OS_WIN
# if !defined (UNIC_HAS_CLOCKNANOSLEEP) && !defined (UNIC_HAS_NANOSLEEP)
static int pp_thread_nanosleep (u32_t msec);
# endif
#endif

static thread_key_t *pp_thread_specific_data = NULL;

static spinlock_t *pp_thread_new_spin = NULL;

static void
pp_thread_cleanup(ptr_t data) {
  u_thread_unref(data);
}

static ptr_t
pp_thread_proxy(ptr_t data) {
  thread_base_t *base_thread = data;
  u_thread_set_local(pp_thread_specific_data, data);
  u_spinlock_lock(pp_thread_new_spin);
  u_spinlock_unlock(pp_thread_new_spin);
  base_thread->func(base_thread->data);
  return NULL;
}

void
u_thread_init(void) {
  if (U_LIKELY (pp_thread_specific_data == NULL)) {
    pp_thread_specific_data =
      u_thread_local_new((destroy_fn_t) pp_thread_cleanup);
  }
  if (U_LIKELY (pp_thread_new_spin == NULL)) {
    pp_thread_new_spin = u_spinlock_new();
  }
  u_thread_init_internal();
}

void
u_thread_shutdown(void) {
  thread_t *cur_thread;
  if (U_LIKELY (pp_thread_specific_data != NULL)) {
    cur_thread = u_thread_get_local(pp_thread_specific_data);
    if (U_UNLIKELY (cur_thread != NULL)) {
      u_thread_unref(cur_thread);
      u_thread_set_local(pp_thread_specific_data, NULL);
    }
    u_thread_local_free(pp_thread_specific_data);
    pp_thread_specific_data = NULL;
  }
  if (U_LIKELY (pp_thread_new_spin != NULL)) {
    u_spinlock_free(pp_thread_new_spin);
    pp_thread_new_spin = NULL;
  }
  u_thread_shutdown_internal();
}

thread_t *
u_thread_create_full(thread_fn_t func,
  ptr_t data,
  bool joinable,
  thread_prio_t prio,
  size_t stack_size) {
  thread_base_t *base_thread;
  if (U_UNLIKELY (func == NULL)) {
    return NULL;
  }
  u_spinlock_lock(pp_thread_new_spin);
  base_thread = (thread_base_t *) u_thread_create_internal(
    pp_thread_proxy,
    joinable,
    prio,
    stack_size
  );
  if (U_LIKELY (base_thread != NULL)) {
    base_thread->ref_count = 2;
    base_thread->ours = true;
    base_thread->joinable = joinable;
    base_thread->func = func;
    base_thread->data = data;
  }
  u_spinlock_unlock(pp_thread_new_spin);
  return (thread_t *) base_thread;
}

thread_t *
u_thread_create(thread_fn_t func,
  ptr_t data,
  bool joinable) {
  /* All checks will be inside */
  return u_thread_create_full(
    func, data, joinable, U_THREAD_PRIORITY_INHERIT,
    0
  );
}

void
u_thread_exit(int code) {
  thread_base_t *base_thread = (thread_base_t *) u_thread_current();
  if (U_UNLIKELY (base_thread == NULL)) {
    return;
  }
  if (U_UNLIKELY (base_thread->ours == false)) {
    U_WARNING (
      "thread_t::u_thread_exit: u_thread_exit() cannot be called from an unknown thread");
    return;
  }
  base_thread->ret_code = code;
  u_thread_exit_internal();
}

int
u_thread_join(thread_t *thread) {
  thread_base_t *base_thread;
  if (U_UNLIKELY (thread == NULL)) {
    return -1;
  }
  base_thread = (thread_base_t *) thread;
  if (base_thread->joinable == false) {
    return -1;
  }
  u_thread_wait_internal(thread);
  return base_thread->ret_code;
}

thread_t *
u_thread_current(void) {
  thread_base_t *base_thread = u_thread_get_local(pp_thread_specific_data);
  if (U_UNLIKELY (base_thread == NULL)) {
    if (U_UNLIKELY ((base_thread = u_malloc0(sizeof(thread_base_t))) == NULL)) {
      U_ERROR ("thread_t::u_thread_current: failed to allocate memory");
      return NULL;
    }
    base_thread->ref_count = 1;
    u_thread_set_local(pp_thread_specific_data, base_thread);
  }
  return (thread_t *) base_thread;
}

int
u_thread_ideal_count(void) {
#if defined (U_OS_WIN)
  SYSTEM_INFO sys_info;
  SystemInfoFunc sys_info_func;
  sys_info_func = (SystemInfoFunc) GetProcAddress(
    GetModuleHandleA("kernel32.dll"),
    "GetNativeSystemInfo"
  );
  if (U_UNLIKELY (sys_info_func == NULL)) {
    sys_info_func = (SystemInfoFunc) GetProcAddress(
      GetModuleHandleA("kernel32.dll"),
      "GetSystemInfo"
    );
  }
  if (U_UNLIKELY (sys_info_func == NULL)) {
    U_ERROR (
      "thread_t::u_thread_ideal_count: failed to get address of system info procedure");
    return 1;
  }
  sys_info_func(&sys_info);
  return (int) sys_info.dwNumberOfProcessors;
#elif defined (U_OS_HPUX)
  struct pst_dynamic psd;

  if (U_LIKELY (pstat_getdynamic (&psd, sizeof (psd), 1, 0) != -1))
    return (int) psd.psd_proc_cnt;
  else {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call pstat_getdynamic()");
    return 1;
  }
#elif defined (U_OS_IRIX)
  int cores;

  cores = sysconf (_SC_NPROC_ONLN);

  if (U_UNLIKELY (cores < 0)) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call sysconf(_SC_NPROC_ONLN)");
    cores = 1;
  }

  return cores;
#elif defined (U_OS_BSD4)
  int cores;
  int mib[2];
  size_t len = sizeof (cores);

  mib[0] = CTL_HW;
  mib[1] = HW_NCPU;

  if (U_UNLIKELY (sysctl (mib, 2, &cores, &len, NULL, 0) == -1)) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call sysctl()");
    return 1;
  }

  return (int) cores;
#elif defined (U_OS_VMS)
  int cores;
  int status;
  uint_t efn;
  IOSB iosb;
# if (UNIC_SIZEOF_VOID_P == 4)
  ILE3 itmlst[] = { { sizeof (cores), SYI$_AVAILCPU_CNT, &cores, NULL},
           { 0, 0, NULL, NULL}
         };
# else
  ILEB_64 itmlst[] = { { 1, SYI$_AVAILCPU_CNT, -1, sizeof (cores), &cores, NULL},
           { 0, 0, 0, 0, NULL, NULL}
         };
# endif

  status = lib$get_ef (&efn);

  if (U_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call lib$get_ef()");
    return 1;
  }

  status = sys$getsyi (efn, NULL, NULL, itmlst, &iosb, tis_io_complete, 0);

  if (U_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call sys$getsyiw()");
    lib$free_ef (&efn);
    return 1;
  }

  status = tis_synch (efn, &iosb);

  if (U_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call tis_synch()");
    lib$free_ef (&efn);
    return 1;
  }

  if (U_UNLIKELY (iosb.iosb$l_getxxi_status != SS$_NORMAL)) {
    U_WARNING ("thread_t::u_thread_ideal_count: l_getxxi_status is not normal");
    lib$free_ef (&efn);
    return 1;
  }

  lib$free_ef (&efn);

  return cores;
#elif defined (U_OS_OS2)
  APIRET ulrc;
  ULONG cores;

  if (U_UNLIKELY (DosQuerySysInfo (QSV_NUMPROCESSORS,
           QSV_NUMPROCESSORS,
           &cores,
           sizeof (cores)) != NO_ERROR)) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call DosQuerySysInfo()");
    return 1;
  }

  return (int) cores;
#elif defined (U_OS_QNX6)
  return (int) _syspage_ptr->num_cpu;
#elif defined (U_OS_BEOS)
  system_info sys_info;

  get_system_info (&sys_info);

  return (int) sys_info.cpu_count;
#elif defined (U_OS_SYLLABLE)
  system_info sys_info;

  if (U_UNLIKELY (get_system_info_v (&sys_info, SYS_INFO_VERSION) != 0)) {
    U_WARNING ("thread_t::u_thread_ideal_count: failed to call get_system_info_v()");
    return 1;
  }

  return (int) sys_info.nCPUCount;
#elif defined (U_OS_SCO) && !defined (_SC_NPROCESSORS_ONLN)
  struct scoutsname utsn;

  if (U_UNLIKELY (__scoinfo (&utsn, sizeof (utsn)) == -1)) {
    U_ERROR ("thread_t::u_thread_ideal_count: failed to call __scoinfo()");
    return 1;
  }

  return (int) utsn.numcpu;
#elif defined (_SC_NPROCESSORS_ONLN)
  int cores;

  cores = (int) sysconf(_SC_NPROCESSORS_ONLN);

  if (U_UNLIKELY (cores == -1)) {
    U_WARNING (
      "thread_t::u_thread_ideal_count: failed to call sysconf(_SC_NPROCESSORS_ONLN)");
    return 1;
  }

  return cores;
#else
  return 1;
#endif
}

void
u_thread_ref(thread_t *thread) {
  if (U_UNLIKELY (thread == NULL)) {
    return;
  }
  u_atomic_int_inc(&((thread_base_t *) thread)->ref_count);
}

void
u_thread_unref(thread_t *thread) {
  thread_base_t *base_thread;
  if (U_UNLIKELY (thread == NULL)) {
    return;
  }
  base_thread = (thread_base_t *) thread;
  if (u_atomic_int_dec_and_test(&base_thread->ref_count) == true) {
    if (base_thread->ours == true) {
      u_thread_free_internal(thread);
    } else {
      u_free(thread);
    }
  }
}
#ifndef U_OS_WIN
# include <errno.h>
# if !defined (UNIC_HAS_CLOCKNANOSLEEP) && !defined (UNIC_HAS_NANOSLEEP)
#   include <sys/select.h>
#   include <sys/time.h>
static int pp_thread_nanosleep (u32_t msec)
{
  int  rc;
  struct timeval tstart, tstop, tremain, time2wait;

  time2wait.tv_sec  = msec / 1000;
  time2wait.tv_usec = (msec % 1000) * 1000;

  if (U_UNLIKELY (gettimeofday (&tstart, NULL) != 0))
    return -1;

  rc = -1;

  while (rc != 0) {
    if (U_UNLIKELY ((rc = select (0, NULL, NULL, NULL, &time2wait)) != 0)) {
      if (u_err_get_last_system () == EINTR) {
        if (gettimeofday (&tstop, NULL) != 0)
          return -1;

        tremain.tv_sec = time2wait.tv_sec -
             (tstop.tv_sec - tstart.tv_sec);
        tremain.tv_usec = time2wait.tv_usec -
              (tstop.tv_usec - tstart.tv_usec);
        tremain.tv_sec += tremain.tv_usec / 1000000L;
        tremain.tv_usec %= 1000000L;
      } else
        return -1;
    }
  }

  return 0;
}
# endif
#endif

int
u_thread_sleep(u32_t msec) {
#if defined (U_OS_WIN)
  Sleep(msec);
  return 0;
#elif defined (U_OS_OS2)
  return (DosSleep (msec) == NO_ERROR) ? 0 : -1;
#elif defined (UNIC_HAS_CLOCKNANOSLEEP) || defined (UNIC_HAS_NANOSLEEP)
  int result;
  struct timespec time_req;
  struct timespec time_rem;

  memset(&time_rem, 0, sizeof(struct timespec));

  time_req.tv_nsec = (msec % 1000) * 1000000L;
  time_req.tv_sec = (time_t) (msec / 1000);

  result = -1;
  while (result != 0) {
# ifdef UNIC_HAS_CLOCKNANOSLEEP
    if (U_UNLIKELY ((result = clock_nanosleep(CLOCK_MONOTONIC,
      0,
      &time_req,
      &time_rem)) != 0)) {
# else
      if (U_UNLIKELY ((result = nanosleep (&time_req, &time_rem)) != 0)) {
# endif
      if (u_err_get_last_system() == EINTR)
        time_req = time_rem;
      else
        return -1;
    }
  }

  return 0;
#else
  return pp_thread_nanosleep (msec);
#endif
}
