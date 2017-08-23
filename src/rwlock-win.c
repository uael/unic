/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

/* More emulation variants: https://github.com/neosmart/RWLock */

#include "p/mem.h"
#include "p/atomic.h"
#include "p/uthread.h"
#include "p/rwlock.h"

#define P_RWLOCK_XP_MAX_SPIN 4000
#define P_RWLOCK_XP_IS_CLEAR(lock) (((lock) & 0x40007FFF) == 0)
#define P_RWLOCK_XP_IS_WRITER(lock) (((lock) & 0x40000000) != 0)
#define P_RWLOCK_XP_SET_WRITER(lock) ((lock) | 0x40000000)
#define P_RWLOCK_XP_UNSET_WRITER(lock) ((lock) & (~0x40000000))
#define P_RWLOCK_XP_SET_READERS(lock, readers) (((lock) & (~0x00007FFF)) | (readers))
#define P_RWLOCK_XP_READER_COUNT(lock) ((lock) & 0x00007FFF)
#define P_RWLOCK_XP_SET_WAITING(lock, waiting) (((lock) & (~0x3FFF8000)) | ((waiting) << 15))
#define P_RWLOCK_XP_WAITING_COUNT(lock) (((lock) & 0x3FFF8000) >> 15)

typedef VOID    (WINAPI
  *InitializeSRWLockFunc)(
  ptr_t lock
);

typedef VOID    (WINAPI
  *AcquireSRWLockExclusiveFunc)(
  ptr_t lock
);

typedef BOOLEAN (WINAPI
  *TryAcquireSRWLockExclusiveFunc)(
  ptr_t lock
);

typedef VOID    (WINAPI
  *ReleaseSRWLockExclusiveFunc)(
  ptr_t lock
);

typedef VOID    (WINAPI
  *AcquireSRWLockSharedFunc)(
  ptr_t lock
);

typedef BOOLEAN (WINAPI
  *TryAcquireSRWLockSharedFunc)(
  ptr_t lock
);

typedef VOID    (WINAPI
  *ReleaseSRWLockSharedFunc)(
  ptr_t lock
);

typedef bool (*PWin32LockInit)(rwlock_t *lock);

typedef void     (*PWin32LockClose)(rwlock_t *lock);

typedef bool (*PWin32LockStartRead)(rwlock_t *lock);

typedef bool (*PWin32LockStartReadTry)(rwlock_t *lock);

typedef bool (*PWin32LockEndRead)(rwlock_t *lock);

typedef bool (*PWin32LockStartWrite)(rwlock_t *lock);

typedef bool (*PWin32LockStartWriteTry)(rwlock_t *lock);

typedef bool (*PWin32LockEndWrite)(rwlock_t *lock);

static PWin32LockInit pp_rwlock_init_func = NULL;

static PWin32LockClose pp_rwlock_close_func = NULL;

static PWin32LockStartRead pp_rwlock_start_read_func = NULL;

static PWin32LockStartReadTry pp_rwlock_start_read_try_func = NULL;

static PWin32LockEndRead pp_rwlock_end_read_func = NULL;

static PWin32LockStartWrite pp_rwlock_start_write_func = NULL;

static PWin32LockStartWriteTry pp_rwlock_start_write_try_func = NULL;

static PWin32LockEndWrite pp_rwlock_end_write_func = NULL;

typedef struct PRWLockVistaTable_ {
  InitializeSRWLockFunc rwl_init;
  AcquireSRWLockExclusiveFunc rwl_excl_lock;
  TryAcquireSRWLockExclusiveFunc rwl_excl_lock_try;
  ReleaseSRWLockExclusiveFunc rwl_excl_rel;
  AcquireSRWLockSharedFunc rwl_shr_lock;
  TryAcquireSRWLockSharedFunc rwl_shr_lock_try;
  ReleaseSRWLockSharedFunc rwl_shr_rel;
} PRWLockVistaTable;

typedef struct PRWLockXP_ {
  volatile u32_t lock;
  HANDLE event;
} PRWLockXP;

struct rwlock {
  ptr_t lock;
};

static PRWLockVistaTable pp_rwlock_vista_table = {
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL
};

/* SRWLock routines */
static bool
pp_rwlock_init_vista(rwlock_t *lock);

static void
pp_rwlock_close_vista(rwlock_t *lock);

static bool
pp_rwlock_start_read_vista(rwlock_t *lock);

static bool
pp_rwlock_start_read_try_vista(rwlock_t *lock);

static bool
pp_rwlock_end_read_vista(rwlock_t *lock);

static bool
pp_rwlock_start_write_vista(rwlock_t *lock);

static bool
pp_rwlock_start_write_try_vista(rwlock_t *lock);

static bool
pp_rwlock_end_write_vista(rwlock_t *lock);

/* Windows XP emulation routines */
static bool
pp_rwlock_init_xp(rwlock_t *lock);

static void
pp_rwlock_close_xp(rwlock_t *lock);

static bool
pp_rwlock_start_read_xp(rwlock_t *lock);

static bool
pp_rwlock_start_read_try_xp(rwlock_t *lock);

static bool
pp_rwlock_end_read_xp(rwlock_t *lock);

static bool
pp_rwlock_start_write_xp(rwlock_t *lock);

static bool
pp_rwlock_start_write_try_xp(rwlock_t *lock);

static bool
pp_rwlock_end_write_xp(rwlock_t *lock);

/* SRWLock routines */

static bool
pp_rwlock_init_vista(rwlock_t *lock) {
  pp_rwlock_vista_table.rwl_init(lock);
  return true;
}

static void
pp_rwlock_close_vista(rwlock_t *lock) {
  P_UNUSED (lock);
}

static bool
pp_rwlock_start_read_vista(rwlock_t *lock) {
  pp_rwlock_vista_table.rwl_shr_lock(lock);
  return true;
}

static bool
pp_rwlock_start_read_try_vista(rwlock_t *lock) {
  return pp_rwlock_vista_table.rwl_shr_lock_try(lock) != 0 ? true : false;
}

static bool
pp_rwlock_end_read_vista(rwlock_t *lock) {
  pp_rwlock_vista_table.rwl_shr_rel(lock);
  return true;
}

static bool
pp_rwlock_start_write_vista(rwlock_t *lock) {
  pp_rwlock_vista_table.rwl_excl_lock(lock);
  return true;
}

static bool
pp_rwlock_start_write_try_vista(rwlock_t *lock) {
  return pp_rwlock_vista_table.rwl_excl_lock_try(lock) != 0 ? true : false;
}

static bool
pp_rwlock_end_write_vista(rwlock_t *lock) {
  pp_rwlock_vista_table.rwl_excl_rel(lock);
  return true;
}

/* Windows XP emulation routines */

static bool
pp_rwlock_init_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp;
  if ((lock->lock = p_malloc0(sizeof(PRWLockXP))) == NULL) {
    P_ERROR ("rwlock_t::pp_rwlock_init_xp: failed to allocate memory");
    return false;
  }
  rwl_xp = ((PRWLockXP *) lock->lock);
  rwl_xp->lock = 0;
  rwl_xp->event = CreateEventA(NULL, false, false, NULL);
  if (P_UNLIKELY (rwl_xp->event == NULL)) {
    P_ERROR ("rwlock_t::pp_rwlock_init_xp: CreateEventA() failed");
    p_free(lock->lock);
    lock->lock = NULL;
    return false;
  }
  return true;
}

static void
pp_rwlock_close_xp(rwlock_t *lock) {
  CloseHandle(((PRWLockXP *) lock->lock)->event);
  p_free(lock->lock);
}

static bool
pp_rwlock_start_read_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  int i;
  u32_t tmp_lock;
  u32_t counter;
  for (i = 0;; ++i) {
    tmp_lock =
      (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock);
    if (!P_RWLOCK_XP_IS_WRITER (tmp_lock)) {
      counter = P_RWLOCK_XP_SET_READERS (tmp_lock,
        P_RWLOCK_XP_READER_COUNT(tmp_lock) + 1);
      if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
        (int) tmp_lock,
        (int) counter
      ) == true) {
        return true;
      } else {
        continue;
      }
    } else {
      if (P_LIKELY (i < P_RWLOCK_XP_MAX_SPIN)) {
        p_uthread_yield();
        continue;
      }
      counter = P_RWLOCK_XP_SET_WAITING (tmp_lock,
        P_RWLOCK_XP_WAITING_COUNT(tmp_lock) + 1);
      if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
        (int) tmp_lock,
        (int) counter
      ) != true) {
        continue;
      }
      i = 0;
      if (P_UNLIKELY (
        WaitForSingleObject(rwl_xp->event, INFINITE) != WAIT_OBJECT_0))
        P_WARNING (
          "rwlock_t::pp_rwlock_start_read_xp: WaitForSingleObject() failed, go ahead");
      do {
        tmp_lock = p_atomic_int_get((const volatile int *) &rwl_xp->lock);
        counter = P_RWLOCK_XP_SET_WAITING (tmp_lock,
          P_RWLOCK_XP_WAITING_COUNT(tmp_lock) - 1);
      } while (
        p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
          (int) tmp_lock,
          (int) counter
        ) != true);
    }
  }
  return true;
}

static bool
pp_rwlock_start_read_try_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  u32_t tmp_lock;
  u32_t counter;
  tmp_lock = (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock
  );
  if (P_RWLOCK_XP_IS_WRITER (tmp_lock)) {
    return false;
  }
  counter =
    P_RWLOCK_XP_SET_READERS (tmp_lock, P_RWLOCK_XP_READER_COUNT(tmp_lock) + 1);
  return p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
    (int) tmp_lock,
    (int) counter
  );
}

static bool
pp_rwlock_end_read_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  u32_t tmp_lock;
  u32_t counter;
  while (true) {
    tmp_lock =
      (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock);
    counter = P_RWLOCK_XP_READER_COUNT (tmp_lock);
    if (P_UNLIKELY (counter == 0)) {
      return true;
    }
    if (counter == 1 && P_RWLOCK_XP_WAITING_COUNT (tmp_lock) != 0) {
      /* A duplicate wake up notification is possible */
      if (P_UNLIKELY (SetEvent(rwl_xp->event) == 0))
        P_WARNING ("rwlock_t::pp_rwlock_end_read_xp: SetEvent() failed");
    }
    counter = P_RWLOCK_XP_SET_READERS (tmp_lock, counter - 1);
    if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
      (int) tmp_lock,
      (int) counter
    ) == true) {
      break;
    }
  }
  return true;
}

static bool
pp_rwlock_start_write_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  int i;
  u32_t tmp_lock;
  u32_t counter;
  for (i = 0;; ++i) {
    tmp_lock =
      (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock);
    if (P_RWLOCK_XP_IS_CLEAR (tmp_lock)) {
      counter = P_RWLOCK_XP_SET_WRITER (tmp_lock);
      if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
        (int) tmp_lock,
        (int) counter
      ) == true) {
        return true;
      } else {
        continue;
      }
    } else {
      if (P_LIKELY (i < P_RWLOCK_XP_MAX_SPIN)) {
        p_uthread_yield();
        continue;
      }
      counter = P_RWLOCK_XP_SET_WAITING (tmp_lock,
        P_RWLOCK_XP_WAITING_COUNT(tmp_lock) + 1);
      if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
        (int) tmp_lock,
        (int) counter
      ) != true) {
        continue;
      }
      i = 0;
      if (P_UNLIKELY (
        WaitForSingleObject(rwl_xp->event, INFINITE) != WAIT_OBJECT_0))
        P_WARNING (
          "rwlock_t::pp_rwlock_start_write_xp: WaitForSingleObject() failed, go ahead");
      do {
        tmp_lock = p_atomic_int_get((const volatile int *) &rwl_xp->lock);
        counter = P_RWLOCK_XP_SET_WAITING (tmp_lock,
          P_RWLOCK_XP_WAITING_COUNT(tmp_lock) - 1);
      } while (
        p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
          (int) tmp_lock,
          (int) counter
        ) != true);
    }
  }
  return true;
}

static bool
pp_rwlock_start_write_try_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  u32_t tmp_lock;
  tmp_lock = (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock
  );
  if (P_RWLOCK_XP_IS_CLEAR (tmp_lock)) {
    return p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
      (int) tmp_lock,
      (int) P_RWLOCK_XP_SET_WRITER (tmp_lock));
  }
  return false;
}

static bool
pp_rwlock_end_write_xp(rwlock_t *lock) {
  PRWLockXP *rwl_xp = ((PRWLockXP *) lock->lock);
  u32_t tmp_lock;
  while (true) {
    while (true) {
      tmp_lock =
        (u32_t) p_atomic_int_get((const volatile int *) &rwl_xp->lock);
      if (P_UNLIKELY (!P_RWLOCK_XP_IS_WRITER(tmp_lock))) {
        return true;
      }
      if (P_RWLOCK_XP_WAITING_COUNT (tmp_lock) == 0) {
        break;
      }

      /* Only the one end-of-write call can be */
      if (P_UNLIKELY (SetEvent(rwl_xp->event) == 0))
        P_WARNING ("rwlock_t::pp_rwlock_end_write_xp: SetEvent() failed");
    }
    if (p_atomic_int_compare_and_exchange((volatile int *) &rwl_xp->lock,
      (int) tmp_lock,
      (int) P_RWLOCK_XP_UNSET_WRITER (tmp_lock)) == true) {
        break;
    }
  }
  return true;
}

rwlock_t *
p_rwlock_new(void) {
  rwlock_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(rwlock_t))) == NULL)) {
    P_ERROR ("rwlock_t::p_rwlock_new: failed to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY (pp_rwlock_init_func(ret) != true)) {
    P_ERROR ("rwlock_t::p_rwlock_new: failed to initialize");
    p_free(ret);
    return NULL;
  }
  return ret;
}

bool
p_rwlock_reader_lock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_start_read_func(lock);
}

bool
p_rwlock_reader_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_start_read_try_func(lock);
}

bool
p_rwlock_reader_unlock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_end_read_func(lock);
}

bool
p_rwlock_writer_lock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_start_write_func(lock);
}

bool
p_rwlock_writer_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_start_write_try_func(lock);
}

bool
p_rwlock_writer_unlock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  return pp_rwlock_end_write_func(lock);
}

void
p_rwlock_free(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return;
  }
  pp_rwlock_close_func(lock);
  p_free(lock);
}

void
p_rwlock_init(void) {
  HMODULE hmodule;
  hmodule = GetModuleHandleA("kernel32.dll");
  if (P_UNLIKELY (hmodule == NULL)) {
    P_ERROR ("rwlock_t::p_rwlock_init: failed to load kernel32.dll module");
    return;
  }
  pp_rwlock_vista_table.rwl_init =
    (InitializeSRWLockFunc) GetProcAddress(
      hmodule,
      "InitializeSRWLock"
    );
  if (P_LIKELY (pp_rwlock_vista_table.rwl_init != NULL)) {
    pp_rwlock_vista_table.rwl_excl_lock =
      (AcquireSRWLockExclusiveFunc) GetProcAddress(
        hmodule,
        "AcquireSRWLockExclusive"
      );
    pp_rwlock_vista_table.rwl_excl_lock_try =
      (TryAcquireSRWLockExclusiveFunc) GetProcAddress(
        hmodule,
        "TryAcquireSRWLockExclusive"
      );
    pp_rwlock_vista_table.rwl_excl_rel =
      (ReleaseSRWLockExclusiveFunc) GetProcAddress(
        hmodule,
        "ReleaseSRWLockExclusive"
      );
    pp_rwlock_vista_table.rwl_shr_lock =
      (AcquireSRWLockSharedFunc) GetProcAddress(
        hmodule,
        "AcquireSRWLockShared"
      );
    pp_rwlock_vista_table.rwl_shr_lock_try =
      (TryAcquireSRWLockSharedFunc) GetProcAddress(
        hmodule,
        "TryAcquireSRWLockShared"
      );
    pp_rwlock_vista_table.rwl_shr_rel =
      (ReleaseSRWLockSharedFunc) GetProcAddress(
        hmodule,
        "ReleaseSRWLockShared"
      );
    pp_rwlock_init_func = pp_rwlock_init_vista;
    pp_rwlock_close_func = pp_rwlock_close_vista;
    pp_rwlock_start_read_func = pp_rwlock_start_read_vista;
    pp_rwlock_start_read_try_func = pp_rwlock_start_read_try_vista;
    pp_rwlock_end_read_func = pp_rwlock_end_read_vista;
    pp_rwlock_start_write_func = pp_rwlock_start_write_vista;
    pp_rwlock_start_write_try_func = pp_rwlock_start_write_try_vista;
    pp_rwlock_end_write_func = pp_rwlock_end_write_vista;
  } else {
    pp_rwlock_init_func = pp_rwlock_init_xp;
    pp_rwlock_close_func = pp_rwlock_close_xp;
    pp_rwlock_start_read_func = pp_rwlock_start_read_xp;
    pp_rwlock_start_read_try_func = pp_rwlock_start_read_try_xp;
    pp_rwlock_end_read_func = pp_rwlock_end_read_xp;
    pp_rwlock_start_write_func = pp_rwlock_start_write_xp;
    pp_rwlock_start_write_try_func = pp_rwlock_start_write_try_xp;
    pp_rwlock_end_write_func = pp_rwlock_end_write_xp;
  }
}

void
p_rwlock_shutdown(void) {
  memset(&pp_rwlock_vista_table, 0, sizeof(pp_rwlock_vista_table));
  pp_rwlock_init_func = NULL;
  pp_rwlock_close_func = NULL;
  pp_rwlock_start_read_func = NULL;
  pp_rwlock_start_read_try_func = NULL;
  pp_rwlock_end_read_func = NULL;
  pp_rwlock_start_write_func = NULL;
  pp_rwlock_start_write_try_func = NULL;
  pp_rwlock_end_write_func = NULL;
}
