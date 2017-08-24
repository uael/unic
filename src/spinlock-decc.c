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
#include "unic/spinlock.h"

#ifdef U_OS_VMS
# include <builtins.h>
#else

# include <machine/builtins.h>

#endif
struct spinlock {
  volatile int spin;
};

spinlock_t *
u_spinlock_new(void) {
  spinlock_t *ret;
  if (U_UNLIKELY ((ret = u_malloc0(sizeof(spinlock_t))) == NULL)) {
    U_ERROR ("spinlock_t::u_spinlock_new: failed to allocate memory");
    return NULL;
  }
  return ret;
}

bool
u_spinlock_lock(spinlock_t *spinlock) {
  if (U_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  (void) __LOCK_LONG((volatile void *) &(spinlock->spin));
  return true;
}

bool
u_spinlock_trylock(spinlock_t *spinlock) {
  if (U_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  return __LOCK_LONG_RETRY((volatile void *) &(spinlock->spin), 1) == 1 ? true
    : false;
}

bool
u_spinlock_unlock(spinlock_t *spinlock) {
  if (U_UNLIKELY (spinlock == NULL)) {
    return false;
  }
  (void) __UNLOCK_LONG((volatile void *) &(spinlock->spin));
  return true;
}

void
u_spinlock_free(spinlock_t *spinlock) {
  u_free(spinlock);
}
