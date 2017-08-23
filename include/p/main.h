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

/*!@file p/main.h
 * @brief Library initialization
 * @author Alexander Saprykin
 *
 * Before using the library you must to initialize it properly. Use
 * p_libsys_init() to initialize the library. Please note that you need to call
 * it only once, not in every thread. This call is not MT-safe (because it also
 * initializes the threading subsystem itself), so it is best to place it in the
 * program's main thread, when the program starts.
 *
 * The only difference between p_libsys_init() and p_libsys_init_full() is that
 * the latter one allows to setup memory management routines before doing any
 * internal library call. This way you can ensure to use provided memory
 * management everywhere (even for library initialization).
 *
 * When you do not need the library anymore release used resourses with the
 * p_libsys_shutdown() routine. You should only call it once, too. This call is
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
 * p/main.h Library initialization
 * @endlink
 * - @link
 * p/types.h Data types
 * @endlink
 * - @link
 * p/arch.h CPU detection macros
 * @endlink
 * - @link
 * p/os.h OS detection macros
 * @endlink
 * - @link
 * p/cc.h Compiler detection macros
 * @endlink
 * - @link
 * p/macros.h Miscellaneous macros
 * @endlink
 * - @link
 * p/string.h Strings
 * @endlink
 *
 * System
 * - @link
 * p/mem.h Memory management
 * @endlink
 * - @link
 * p/process.h Process
 * @endlink
 * - @link
 * p/dl.h Shared library loader
 * @endlink
 * - @link
 * p/profiler.h Time profiler
 * @endlink
 * - @link
 * p/err.h Errors
 * @endlink
 *
 * Data structures
 * - @link
 * p/list.h Singly linked list
 * @endlink
 * - @link
 * p/htable.h Hash table
 * @endlink
 * - @link
 * p/hash.h Cryptographic hash
 * @endlink
 * - @link
 * p/tree.h Binary search tree
 * @endlink
 *
 * Multithreading
 * - @link
 * p/uthread.h Thread
 * @endlink
 * - @link
 * p/mutex.h Mutex
 * @endlink
 * - @link
 * p/condvar.h Condition variable
 * @endlink
 * - @link
 * p/rwlock.h Read-write lock
 * @endlink
 * - @link
 * p/spinlock.h Spinlock
 * @endlink
 * - @link
 * p/atomic.h Atomic operations
 * @endlink
 *
 * Interprocess communication
 * - @link
 * p/sem.h Semaphore
 * @endlink
 * - @link
 * p/shm.h Shared memory
 * @endlink
 * - @link
 * p/shmbuf.h Shared memory buffer
 * @endlink
 *
 * Networking
 * - @link
 * p/socketaddr.h Socket address
 * @endlink
 * - @link
 * p/socket.h Socket
 * @endlink
 *
 * File and directories
 * - @link
 * p/file.h Files
 * @endlink
 * - @link
 * p/dir.h Directories
 * @endlink
 * - @link
 * p/inifile.h INI file parser
 * @endlink
 */
#ifndef P_MAIN_H__
# define P_MAIN_H__

#include "p/macros.h"
#include "p/mem.h"

/*!@brief Initializes library resources.
 * @since 0.0.1
 */
P_API void
p_libsys_init(void);

/*!@brief Initializes library resources along with the memory table.
 * @param vtable Memory management table.
 * @since 0.0.1
 */
P_API void
p_libsys_init_full(const mem_vtable_t *vtable);

/*!@brief Frees library resources. You should stop using any of the library
 * routines after calling this one.
 * @since 0.0.1
 */
P_API void
p_libsys_shutdown(void);

/*!@brief Gets the library version against which it was compiled at run-time.
 * @return Library version.
 * @since 0.0.1
 * @note This version may differ from the version the application was compiled
 * against.
 * @sa #PLIBSYS_VERSION, #PLIBSYS_VERSION_STR, #PLIBSYS_VERSION_CHECK
 */
P_API const byte_t *
p_libsys_version(void);

#endif /* !P_MAIN_H__ */
