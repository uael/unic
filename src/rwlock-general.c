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

#include "unic/mem.h"
#include "unic/mutex.h"
#include "unic/condvar.h"
#include "unic/rwlock.h"

#define U_RWLOCK_SET_READERS(lock, readers) (((lock) & (~0x00007FFF)) | (readers))
#define U_RWLOCK_READER_COUNT(lock) ((lock) & 0x00007FFF)
#define U_RWLOCK_SET_WRITERS(lock, writers) (((lock) & (~0x3FFF8000)) | ((writers) << 15))
#define U_RWLOCK_WRITER_COUNT(lock) (((lock) & 0x3FFF8000) >> 15)
struct rwlock {
  mutex_t *mutex;
  condvar_t *read_cv;
  condvar_t *write_cv;
  u32_t active_threads;
  u32_t waiting_threads;
};

rwlock_t *
u_rwlock_new(void) {
  rwlock_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(rwlock_t))) == NULL)) {
    U_ERROR ("rwlock_t::u_rwlock_new: failed to allocate memory");
    return NULL;
  }
  if (U_UNLIKELY ((ret->mutex = u_mutex_new()) == NULL)) {
    U_ERROR ("rwlock_t::u_rwlock_new: failed to allocate mutex");
    u_free(ret);
  }
  if (U_UNLIKELY ((ret->read_cv = u_condvar_new()) == NULL)) {
    U_ERROR (
      "rwlock_t::u_rwlock_new: failed to allocate condition variable for read");
    u_mutex_free(ret->mutex);
    u_free(ret);
  }
  if (U_UNLIKELY ((ret->write_cv = u_condvar_new()) == NULL)) {
    U_ERROR (
      "rwlock_t::u_rwlock_new: failed to allocate condition variable for write");
    u_condvar_free(ret->read_cv);
    u_mutex_free(ret->mutex);
    u_free(ret);
  }
  return ret;
}

bool
u_rwlock_reader_lock(rwlock_t *lock) {
  bool wait_ok;
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_lock: u_mutex_lock() failed");
    return false;
  }
  wait_ok = true;
  if (U_RWLOCK_WRITER_COUNT (lock->active_threads)) {
    lock->waiting_threads = U_RWLOCK_SET_READERS (lock->waiting_threads,
      U_RWLOCK_READER_COUNT(lock->waiting_threads) + 1);
    while (U_RWLOCK_WRITER_COUNT (lock->active_threads)) {
      wait_ok = u_condvar_wait(lock->read_cv, lock->mutex);
      if (U_UNLIKELY (wait_ok == false)) {
        U_ERROR (
          "rwlock_t::u_rwlock_reader_lock: u_condvar_wait() failed");
        break;
      }
    }
    lock->waiting_threads = U_RWLOCK_SET_READERS (lock->waiting_threads,
      U_RWLOCK_READER_COUNT(lock->waiting_threads) - 1);
  }
  if (U_LIKELY (wait_ok == true)) {
    lock->active_threads = U_RWLOCK_SET_READERS (lock->active_threads,
      U_RWLOCK_READER_COUNT(lock->active_threads) + 1);
  }
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_lock: u_mutex_unlock() failed");
    return false;
  }
  return wait_ok;
}

bool
u_rwlock_reader_trylock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_trylock: u_mutex_lock() failed");
    return false;
  }
  if (U_RWLOCK_WRITER_COUNT (lock->active_threads)) {
    if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false))
      U_ERROR ("rwlock_t::u_rwlock_reader_trylock: u_mutex_unlock() failed(1)");
    return false;
  }
  lock->active_threads = U_RWLOCK_SET_READERS (lock->active_threads,
    U_RWLOCK_READER_COUNT(lock->active_threads) + 1);
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_trylock: u_mutex_unlock() failed(2)");
    return false;
  }
  return true;
}

bool
u_rwlock_reader_unlock(rwlock_t *lock) {
  u32_t reader_count;
  bool signal_ok;
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_unlock: u_mutex_lock() failed");
    return false;
  }
  reader_count = U_RWLOCK_READER_COUNT (lock->active_threads);
  if (U_UNLIKELY (reader_count == 0)) {
    if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false))
      U_ERROR ("rwlock_t::u_rwlock_reader_unlock: u_mutex_unlock() failed(1)");
    return true;
  }
  lock->active_threads =
    U_RWLOCK_SET_READERS (lock->active_threads, reader_count - 1);
  signal_ok = true;
  if (reader_count == 1 && U_RWLOCK_WRITER_COUNT (lock->waiting_threads)) {
    signal_ok = u_condvar_signal(lock->write_cv);
  }
  if (U_UNLIKELY (signal_ok == false))
    U_ERROR (
      "rwlock_t::u_rwlock_reader_unlock: u_condvar_signal() failed");
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_reader_unlock: u_mutex_unlock() failed(2)");
    return false;
  }
  return signal_ok;
}

bool
u_rwlock_writer_lock(rwlock_t *lock) {
  bool wait_ok;
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_lock: u_mutex_lock() failed");
    return false;
  }
  wait_ok = true;
  if (lock->active_threads) {
    lock->waiting_threads = U_RWLOCK_SET_WRITERS (lock->waiting_threads,
      U_RWLOCK_WRITER_COUNT(lock->waiting_threads) + 1);
    while (lock->active_threads) {
      wait_ok = u_condvar_wait(lock->write_cv, lock->mutex);
      if (U_UNLIKELY (wait_ok == false)) {
        U_ERROR (
          "rwlock_t::u_rwlock_writer_lock: u_condvar_wait() failed");
        break;
      }
    }
    lock->waiting_threads = U_RWLOCK_SET_WRITERS (lock->waiting_threads,
      U_RWLOCK_WRITER_COUNT(lock->waiting_threads) - 1);
  }
  if (U_LIKELY (wait_ok == true)) {
    lock->active_threads = U_RWLOCK_SET_WRITERS (lock->active_threads, 1);
  }
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_lock: u_mutex_unlock() failed");
    return false;
  }
  return wait_ok;
}

bool
u_rwlock_writer_trylock(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_trylock: u_mutex_lock() failed");
    return false;
  }
  if (lock->active_threads) {
    if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false))
      U_ERROR ("rwlock_t::u_rwlock_writer_trylock: u_mutex_unlock() failed(1)");
    return false;
  }
  lock->active_threads = U_RWLOCK_SET_WRITERS (lock->active_threads, 1);
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_trylock: u_mutex_unlock() failed(2)");
    return false;
  }
  return true;
}

bool
u_rwlock_writer_unlock(rwlock_t *lock) {
  bool signal_ok;
  if (U_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (U_UNLIKELY (u_mutex_lock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_unlock: u_mutex_lock() failed");
    return false;
  }
  lock->active_threads = U_RWLOCK_SET_WRITERS (lock->active_threads, 0);
  signal_ok = true;
  if (U_RWLOCK_WRITER_COUNT (lock->waiting_threads)) {
    if (U_UNLIKELY (u_condvar_signal(lock->write_cv) == false)) {
      U_ERROR (
        "rwlock_t::u_rwlock_writer_unlock: u_condvar_signal() failed");
      signal_ok = false;
    }
  } else if (U_RWLOCK_READER_COUNT (lock->waiting_threads)) {
    if (U_UNLIKELY (u_condvar_broadcast(lock->read_cv) == false)) {
      U_ERROR (
        "rwlock_t::u_rwlock_writer_unlock: u_condvar_broadcast() failed");
      signal_ok = false;
    }
  }
  if (U_UNLIKELY (u_mutex_unlock(lock->mutex) == false)) {
    U_ERROR ("rwlock_t::u_rwlock_writer_unlock: u_mutex_unlock() failed");
    return false;
  }
  return signal_ok;
}

void
u_rwlock_free(rwlock_t *lock) {
  if (U_UNLIKELY (lock == NULL)) {
    return;
  }
  if (U_UNLIKELY (lock->active_threads))
    U_WARNING (
      "rwlock_t::u_rwlock_free: destroying while active threads are present");
  if (U_UNLIKELY (lock->waiting_threads))
    U_WARNING (
      "rwlock_t::u_rwlock_free: destroying while waiting threads are present");
  u_mutex_free(lock->mutex);
  u_condvar_free(lock->read_cv);
  u_condvar_free(lock->write_cv);
  u_free(lock);
}

void
u_rwlock_init(void) {
}

void
u_rwlock_shutdown(void) {
}
