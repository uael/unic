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

#include "p/mutex.h"

struct mutex {
  int hdl;
};

mutex_t *
p_mutex_new(void) {
  return NULL;
}

bool
p_mutex_lock(mutex_t *mutex) {
  return false;
}

bool
p_mutex_trylock(mutex_t *mutex) {
  P_UNUSED (mutex);
  return false;
}

bool
p_mutex_unlock(mutex_t *mutex) {
  P_UNUSED (mutex);
  return false;
}

void
p_mutex_free(mutex_t *mutex) {
  P_UNUSED (mutex);
}
