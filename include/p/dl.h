/*
 * Copyright (C) 2015-2017 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file p/dl.h
 * @brief Shared library loader
 * @author Alexander Saprykin
 *
 * All modern operating systems support dynamic loadable objects. Such objects
 * are compiled with special flags and can be loaded by other programs and
 * libraries later at the runtime. These loadable objects often called as the
 * shared libraries, though some platforms even allow to treat the program
 * binary as a loadable object, too.
 *
 * When the program is linked with a shared library its dependency would be
 * resolved by the operating system automatically before starting the program.
 * But in some circumstances you may need to load a shared library object
 * explicitly (i.e. implementing a plugin subsystem, checking for API
 * availability).
 *
 * All functions and variables which a shared library is exporting are called
 * symbols. Usually only the exported symbols are available from outside the
 * shared library. Actually all those symbols represent a library API.
 *
 * Use p_dl_new() to load a shared library and
 * p_dl_get_symbol() to retrieve a pointer to a symbol within it.
 * Close the library after usage with p_dl_free().
 *
 * Please note the following platform specific differences:
 *
 * - HP-UX doesn't support loading libraries containing TLS and built with
 * static TLS model. The same rule applies to any library used as dependency.
 * HP-UX on 32-bit PA-RISC systems doesn't support reference counting for loaded
 * libraries when using shl_* family of functions (always removes all library
 * references on unload).
 *
 * - On OpenVMS only shareable images (linked with /SHAREABLE) can be used for
 * dynamic symbol resolving. Usually they have .EXE extension.
 *
 * - BeOS supports dynamic loading for add-ons only. It is also possible to
 * load the same library several times independently (not like a traditional
 * shared library).
 */
#ifndef P_DL_H__
# define P_DL_H__

#include "p/macros.h"
#include "p/types.h"

/*!@brief Opaque data structure to handle a shared library. */
typedef struct dl dl_t;

/*!@brief Pointer to a function address. */
typedef void (*PFuncAddr)(void);

/*!@brief Loads a shared library.
 * @param path Path to the shared library file.
 * @return Pointer to #PLibraryLoader in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * If you are loading the already loaded shared library, an operating system
 * increments corresponding reference count and decrements it after freeing
 * #PLibraryLoader, thus the shared library would be unloaded from the address
 * space only when the counter becomes zero.
 */
P_API dl_t *
p_dl_new(const byte_t *path);

/*!@brief Gets a pointer to a symbol in the loaded shared library.
 * @param loader Pointer to the loaded shared library handle.
 * @param sym Name of the symbol.
 * @return Pointer to the symbol in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Since the symbol may have a NULL value, the returned NULL value from this
 * call actually doesn't mean the failed result. You can additionally check the
 * error result using p_dl_get_last_error().
 */
P_API PFuncAddr
p_dl_get_symbol(dl_t *loader, const byte_t *sym);

/*!@brief Frees memory and allocated resources of #PLibraryLoader.
 * @param loader #PLibraryLoader object to free.
 * @since 0.0.1
 */
P_API void
p_dl_free(dl_t *loader);

/*!@brief Gets the last occurred error.
 * @param loader #PLibraryLoader object to get error for.
 * @return Human readable error string in case of success, NULL otherwise.
 * @since 0.0.1
 * @version 0.0.3 @p loader parameter was added.
 * @note Caller takes ownership of the returned string.
 *
 * The NULL result may indicate that no error was occurred since the last call.
 *
 * Different operating systems have different behavior on error indicating.
 * Some systems reset an error status before the call, some do not. Some
 * systems write the successful call result (usually zero) to the error status,
 * thus resetting an error from the previous call.
 *
 * Some operating systems may return last error even if library handler was not
 * created. In that case try to pass NULL value as a parameter.
 */
P_API byte_t *
p_dl_get_last_error(dl_t *loader);

/*!@brief Checks whether library loading subsystem uses reference counting.
 * @return true in case of success, false otherwise.
 * @since 0.0.3
 *
 * When reference counting is supported, the same shared library can be opened
 * several times, but it would be completely unloaded from the memory only when
 * the last reference to it is removed.
 *
 * @note For now, only HP-UX on 32-bit PA-RISC systems with shl_* model doesn't
 * support reference counting.
 */
P_API bool
p_dl_is_ref_counted(void);

#endif /* !P_DL_H__ */
