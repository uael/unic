/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file unic/main.h
 * @brief Library initialization
 * @author Alexander Saprykin
 *
 * Before using the library you must to initialize it properly. Use
 * u_libsys_init() to initialize the library. Please note that you need to call
 * it only once, not in every thread. This call is not MT-safe (because it also
 * initializes the threading subsystem itself), so it is best to place it in the
 * program's main thread, when the program starts.
 *
 * The only difference between u_libsys_init() and u_libsys_init_full() is that
 * the latter one allows to setup memory management routines before doing any
 * internal library call. This way you can ensure to use provided memory
 * management everywhere (even for library initialization).
 *
 * When you do not need the library anymore release used resourses with the
 * u_libsys_shutdown() routine. You should only call it once, too. This call is
 * not MT-safe (because it also deinitializes the threading subsystem itself),
 * so it is best to place it in the program's main thread, when the program
 * finishes.
 *
 * It is not recommended to call the initialization and deinitialization
 * routines on Windows in the DllMain() call because it may require libraries
 * other than kernel32.dll.
 */

/*!@mainpage
 * Basic
 * - @link
 * unic/main.h Library initialization
 * @endlink
 * - @link
 * unic/types.h Data types
 * @endlink
 * - @link
 * unic/arch.h CPU detection macros
 * @endlink
 * - @link
 * unic/os.h OS detection macros
 * @endlink
 * - @link
 * unic/cc.h Compiler detection macros
 * @endlink
 * - @link
 * unic/macros.h Miscellaneous macros
 * @endlink
 * - @link
 * unic/string.h Strings
 * @endlink
 *
 * System
 * - @link
 * unic/mem.h Memory management
 * @endlink
 * - @link
 * unic/process.h Process
 * @endlink
 * - @link
 * unic/dl.h Shared library loader
 * @endlink
 * - @link
 * unic/profiler.h Time profiler
 * @endlink
 * - @link
 * unic/err.h Errors
 * @endlink
 *
 * Data structures
 * - @link
 * unic/list.h Singly linked list
 * @endlink
 * - @link
 * unic/htable.h Hash table
 * @endlink
 * - @link
 * unic/hash.h Cryptographic hash
 * @endlink
 * - @link
 * unic/tree.h Binary search tree
 * @endlink
 *
 * Multithreading
 * - @link
 * unic/thread.h Thread
 * @endlink
 * - @link
 * unic/mutex.h Mutex
 * @endlink
 * - @link
 * unic/condvar.h Condition variable
 * @endlink
 * - @link
 * unic/rwlock.h Read-write lock
 * @endlink
 * - @link
 * unic/spinlock.h Spinlock
 * @endlink
 * - @link
 * unic/atomic.h Atomic operations
 * @endlink
 *
 * Interprocess communication
 * - @link
 * unic/sem.h Semaphore
 * @endlink
 * - @link
 * unic/shm.h Shared memory
 * @endlink
 * - @link
 * unic/shmbuf.h Shared memory buffer
 * @endlink
 *
 * Networking
 * - @link
 * unic/socketaddr.h Socket address
 * @endlink
 * - @link
 * unic/socket.h Socket
 * @endlink
 *
 * File and directories
 * - @link
 * unic/file.h Files
 * @endlink
 * - @link
 * unic/dir.h Directories
 * @endlink
 * - @link
 * unic/inifile.h INI file parser
 * @endlink
 */
#ifndef U_MAIN_H__
# define U_MAIN_H__

#include "unic/macros.h"
#include "unic/mem.h"

/*!@brief Initializes library resources.
 * @since 0.0.1
 */
U_API void
u_libsys_init(void);

/*!@brief Initializes library resources along with the memory table.
 * @param vtable Memory management table.
 * @since 0.0.1
 */
U_API void
u_libsys_init_full(const mem_vtable_t *vtable);

/*!@brief Frees library resources. You should stop using any of the library
 * routines after calling this one.
 * @since 0.0.1
 */
U_API void
u_libsys_shutdown(void);

/*!@brief Gets the library version against which it was compiled at run-time.
 * @return Library version.
 * @since 0.0.1
 * @note This version may differ from the version the application was compiled
 * against.
 * @sa #UNIC_VERSION, #UNIC_VERSION_STR, #UNIC_VERSION_CHECK
 */
U_API const byte_t *
u_libsys_version(void);

#endif /* !U_MAIN_H__ */
