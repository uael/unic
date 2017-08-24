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

/*
 * Copyright 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * http://crbug.com/269623
 * http://openradar.appspot.com/14999594
 *
 * When the default version of close used on macOS fails with EINTR, the
 * file descriptor is not in a deterministic state. It may have been closed,
 * or it may not have been. This makes it impossible to gracefully recover
 * from the error. If the close is retried after the FD has been closed, the
 * subsequent close can report EBADF, or worse, it can close an unrelated FD
 * opened by another thread. If the close is not retried after the FD has been
 * left open, the FD is leaked. Neither of these are good options.
 *
 * macOS provides an alternate version of close, close$NOCANCEL. This
 * version will never fail with EINTR before the FD is actually closed. With
 * this version, it is thus safe to call close without checking for EINTR (as
 * the HANDLE_EINTR macro does) and not risk leaking the FD. In fact, mixing
 * this verison of close with HANDLE_EINTR is hazardous.
 *
 * The $NOCANCEL variants of various system calls are activated by compiling
 * with __DARWIN_NON_CANCELABLE, which prevents them from being pthread
 * cancellation points. Rather than taking such a heavy-handed approach, this
 * file implements an alternative: to use the $NOCANCEL variant of close (thus
 * preventing it from being a pthread cancellation point) without affecting
 * any other system calls.
 *
 * This file operates by providing a close function with the non-$NOCANCEL
 * symbol name expected for the compilation environment as set by <unistd.h>
 * and <sys/cdefs.h> (the DARWIN_ALIAS_C macro). That function calls the
 * $NOCANCEL variant, which is resolved from libsyscall. By linking with this
 * version of close prior to the libsyscall version, close's implementation is
 * overridden.
 */

#include "sysclose-private.h"

#ifndef ___close_nocancel
# if !__DARWIN_ONLY_UNIX_CONFORMANCE
/*
 * When there's a choice between UNIX2003 and pre-UNIX2003. There's no
 * close$NOCANCEL symbol in this case, so use close$NOCANCEL$UNIX2003 as the
 * implementation. It does the same thing that close$NOCANCEL would do.
 */
#   define ___close_nocancel close$NOCANCEL$UNIX2003
# else
/* When only UNIX2003 is supported: */
#   define ___close_nocancel close$NOCANCEL
# endif
#endif

extern int
___close_nocancel(int fd);

int
u_sys_close(int fd) {
  return ___close_nocancel(fd);
}
#undef close_implementation
