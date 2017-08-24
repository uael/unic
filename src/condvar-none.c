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

#include "unic/condvar.h"

struct condvar {
  int hdl;
};

condvar_t *
u_condvar_new(void) {
  return NULL;
}

void
u_condvar_free(condvar_t *cond) {
  U_UNUSED (cond);
}

bool
u_condvar_wait(condvar_t *cond,
  mutex_t *mutex) {
  U_UNUSED (cond);
  U_UNUSED (mutex);
  return false;
}

bool
u_condvar_signal(condvar_t *cond) {
  U_UNUSED (cond);
  return false;
}

bool
u_condvar_broadcast(condvar_t *cond) {
  U_UNUSED (cond);
  return false;
}

void
u_condvar_init(void) {
}

void
u_condvar_shutdown(void) {
}
