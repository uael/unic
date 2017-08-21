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

#include "pmem.h"
#include "pspinlock.h"

struct PSpinLock_ {
	volatile pint spin;
};

P_LIB_API PSpinLock *
p_spinlock_new (void)
{
	PSpinLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSpinLock))) == NULL)) {
		P_ERROR ("PSpinLock::p_spinlock_new: failed to allocate memory");
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_spinlock_lock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	while ((pboolean) __sync_bool_compare_and_swap (&(spinlock->spin), 0, 1) == FALSE);

	return TRUE;
}

P_LIB_API pboolean
p_spinlock_trylock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	return (pboolean) __sync_bool_compare_and_swap (&(spinlock->spin), 0, 1);
}

P_LIB_API pboolean
p_spinlock_unlock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	spinlock->spin = 0;
	__sync_synchronize ();

	return TRUE;
}

P_LIB_API void
p_spinlock_free (PSpinLock *spinlock)
{
	p_free (spinlock);
}
