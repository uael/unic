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

#include "p/rwlock.h"

struct rwlock {
  int hdl;
};

rwlock_t *
p_rwlock_new(void) {
  return NULL;
}

bool
p_rwlock_reader_lock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

bool
p_rwlock_reader_trylock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

bool
p_rwlock_reader_unlock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

bool
p_rwlock_writer_lock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

bool
p_rwlock_writer_trylock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

bool
p_rwlock_writer_unlock(rwlock_t *lock) {
  P_UNUSED (lock);
  return false;
}

void
p_rwlock_free(rwlock_t *lock) {
  P_UNUSED (lock);
}

void
p_rwlock_init(void) {
}

void
p_rwlock_shutdown(void) {
}

