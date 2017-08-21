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

#include "p/condvar.h"

#include <stdlib.h>

struct p_condvar {
  int_t hdl;
};

P_API p_condvar_t *
p_condvar_new(void) {
  return NULL;
}

P_API void
p_condvar_free(p_condvar_t *cond) {
  P_UNUSED (cond);
}

P_API bool
p_condvar_wait(p_condvar_t *cond,
  PMutex *mutex) {
  P_UNUSED (cond);
  P_UNUSED (mutex);

  return false;
}

P_API bool
p_condvar_signal(p_condvar_t *cond) {
  P_UNUSED (cond);

  return false;
}

P_API bool
p_condvar_broadcast(p_condvar_t *cond) {
  P_UNUSED (cond);

  return false;
}

void
p_condvar_init(void) {
}

void
p_condvar_shutdown(void) {
}
