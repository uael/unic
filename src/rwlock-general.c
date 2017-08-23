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

#include "p/mem.h"
#include "p/mutex.h"
#include "p/condvar.h"
#include "p/rwlock.h"

#define P_RWLOCK_SET_READERS(lock, readers) (((lock) & (~0x00007FFF)) | (readers))
#define P_RWLOCK_READER_COUNT(lock) ((lock) & 0x00007FFF)
#define P_RWLOCK_SET_WRITERS(lock, writers) (((lock) & (~0x3FFF8000)) | ((writers) << 15))
#define P_RWLOCK_WRITER_COUNT(lock) (((lock) & 0x3FFF8000) >> 15)
struct rwlock {
  mutex_t *mutex;
  condvar_t *read_cv;
  condvar_t *write_cv;
  uint32_t active_threads;
  uint32_t waiting_threads;
};

rwlock_t *
p_rwlock_new(void) {
  rwlock_t *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(rwlock_t))) == NULL)) {
    P_ERROR ("rwlock_t::p_rwlock_new: failed to allocate memory");
    return NULL;
  }
  if (P_UNLIKELY ((ret->mutex = p_mutex_new()) == NULL)) {
    P_ERROR ("rwlock_t::p_rwlock_new: failed to allocate mutex");
    p_free(ret);
  }
  if (P_UNLIKELY ((ret->read_cv = p_condvar_new()) == NULL)) {
    P_ERROR (
      "rwlock_t::p_rwlock_new: failed to allocate condition variable for read");
    p_mutex_free(ret->mutex);
    p_free(ret);
  }
  if (P_UNLIKELY ((ret->write_cv = p_condvar_new()) == NULL)) {
    P_ERROR (
      "rwlock_t::p_rwlock_new: failed to allocate condition variable for write");
    p_condvar_free(ret->read_cv);
    p_mutex_free(ret->mutex);
    p_free(ret);
  }
  return ret;
}

bool
p_rwlock_reader_lock(rwlock_t *lock) {
  bool wait_ok;
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_lock: p_mutex_lock() failed");
    return false;
  }
  wait_ok = true;
  if (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
    lock->waiting_threads = P_RWLOCK_SET_READERS (lock->waiting_threads,
      P_RWLOCK_READER_COUNT(lock->waiting_threads) + 1);
    while (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
      wait_ok = p_condvar_wait(lock->read_cv, lock->mutex);
      if (P_UNLIKELY (wait_ok == false)) {
        P_ERROR (
          "rwlock_t::p_rwlock_reader_lock: p_condvar_wait() failed");
        break;
      }
    }
    lock->waiting_threads = P_RWLOCK_SET_READERS (lock->waiting_threads,
      P_RWLOCK_READER_COUNT(lock->waiting_threads) - 1);
  }
  if (P_LIKELY (wait_ok == true)) {
    lock->active_threads = P_RWLOCK_SET_READERS (lock->active_threads,
      P_RWLOCK_READER_COUNT(lock->active_threads) + 1);
  }
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_lock: p_mutex_unlock() failed");
    return false;
  }
  return wait_ok;
}

bool
p_rwlock_reader_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_trylock: p_mutex_lock() failed");
    return false;
  }
  if (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
    if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false))
      P_ERROR ("rwlock_t::p_rwlock_reader_trylock: p_mutex_unlock() failed(1)");
    return false;
  }
  lock->active_threads = P_RWLOCK_SET_READERS (lock->active_threads,
    P_RWLOCK_READER_COUNT(lock->active_threads) + 1);
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_trylock: p_mutex_unlock() failed(2)");
    return false;
  }
  return true;
}

bool
p_rwlock_reader_unlock(rwlock_t *lock) {
  uint32_t reader_count;
  bool signal_ok;
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_unlock: p_mutex_lock() failed");
    return false;
  }
  reader_count = P_RWLOCK_READER_COUNT (lock->active_threads);
  if (P_UNLIKELY (reader_count == 0)) {
    if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false))
      P_ERROR ("rwlock_t::p_rwlock_reader_unlock: p_mutex_unlock() failed(1)");
    return true;
  }
  lock->active_threads =
    P_RWLOCK_SET_READERS (lock->active_threads, reader_count - 1);
  signal_ok = true;
  if (reader_count == 1 && P_RWLOCK_WRITER_COUNT (lock->waiting_threads)) {
    signal_ok = p_condvar_signal(lock->write_cv);
  }
  if (P_UNLIKELY (signal_ok == false))
    P_ERROR (
      "rwlock_t::p_rwlock_reader_unlock: p_condvar_signal() failed");
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_reader_unlock: p_mutex_unlock() failed(2)");
    return false;
  }
  return signal_ok;
}

bool
p_rwlock_writer_lock(rwlock_t *lock) {
  bool wait_ok;
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_lock: p_mutex_lock() failed");
    return false;
  }
  wait_ok = true;
  if (lock->active_threads) {
    lock->waiting_threads = P_RWLOCK_SET_WRITERS (lock->waiting_threads,
      P_RWLOCK_WRITER_COUNT(lock->waiting_threads) + 1);
    while (lock->active_threads) {
      wait_ok = p_condvar_wait(lock->write_cv, lock->mutex);
      if (P_UNLIKELY (wait_ok == false)) {
        P_ERROR (
          "rwlock_t::p_rwlock_writer_lock: p_condvar_wait() failed");
        break;
      }
    }
    lock->waiting_threads = P_RWLOCK_SET_WRITERS (lock->waiting_threads,
      P_RWLOCK_WRITER_COUNT(lock->waiting_threads) - 1);
  }
  if (P_LIKELY (wait_ok == true)) {
    lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 1);
  }
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_lock: p_mutex_unlock() failed");
    return false;
  }
  return wait_ok;
}

bool
p_rwlock_writer_trylock(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_trylock: p_mutex_lock() failed");
    return false;
  }
  if (lock->active_threads) {
    if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false))
      P_ERROR ("rwlock_t::p_rwlock_writer_trylock: p_mutex_unlock() failed(1)");
    return false;
  }
  lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 1);
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_trylock: p_mutex_unlock() failed(2)");
    return false;
  }
  return true;
}

bool
p_rwlock_writer_unlock(rwlock_t *lock) {
  bool signal_ok;
  if (P_UNLIKELY (lock == NULL)) {
    return false;
  }
  if (P_UNLIKELY (p_mutex_lock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_unlock: p_mutex_lock() failed");
    return false;
  }
  lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 0);
  signal_ok = true;
  if (P_RWLOCK_WRITER_COUNT (lock->waiting_threads)) {
    if (P_UNLIKELY (p_condvar_signal(lock->write_cv) == false)) {
      P_ERROR (
        "rwlock_t::p_rwlock_writer_unlock: p_condvar_signal() failed");
      signal_ok = false;
    }
  } else if (P_RWLOCK_READER_COUNT (lock->waiting_threads)) {
    if (P_UNLIKELY (p_condvar_broadcast(lock->read_cv) == false)) {
      P_ERROR (
        "rwlock_t::p_rwlock_writer_unlock: p_condvar_broadcast() failed");
      signal_ok = false;
    }
  }
  if (P_UNLIKELY (p_mutex_unlock(lock->mutex) == false)) {
    P_ERROR ("rwlock_t::p_rwlock_writer_unlock: p_mutex_unlock() failed");
    return false;
  }
  return signal_ok;
}

void
p_rwlock_free(rwlock_t *lock) {
  if (P_UNLIKELY (lock == NULL)) {
    return;
  }
  if (P_UNLIKELY (lock->active_threads))
    P_WARNING (
      "rwlock_t::p_rwlock_free: destroying while active threads are present");
  if (P_UNLIKELY (lock->waiting_threads))
    P_WARNING (
      "rwlock_t::p_rwlock_free: destroying while waiting threads are present");
  p_mutex_free(lock->mutex);
  p_condvar_free(lock->read_cv);
  p_condvar_free(lock->write_cv);
  p_free(lock);
}

void
p_rwlock_init(void) {
}

void
p_rwlock_shutdown(void) {
}
