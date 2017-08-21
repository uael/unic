/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

/**
 * @file p/dir.h
 * @brief Filesystem interface
 * @author Alexander Saprykin
 *
 * A traditional filesystem can be presented as a combination of directories and
 * files within a defined hierarchy. A directory contains the so called entries:
 * files and other directories. #PDir allows to iterate through these entries
 * without reading their contents, thus building a filesystem hierarchy tree.
 *
 * Think of this module as an interface to the well-known `dirent` API.
 *
 * First you need to open a directory for iterating through its content entries
 * using p_dir_new(). After that every next entry inside the directory can be
 * read with the p_dir_get_next_entry() call until it returns NULL (though it's
 * better to check an error code to be sure no error occurred).
 *
 * Also some directory manipulation routines are provided to create, remove and
 * check existance.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef P_DIR_H__
#define P_DIR_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/error.h"

/** Directory opaque data structure. */
typedef struct p_dir p_dir_t;

/** Directory entry types. */
typedef enum p_dirent_kind {
  P_DIR_ENTRY_TYPE_DIR = 1,  /**< Directory.	*/
  P_DIR_ENTRY_TYPE_FILE = 2,  /**< File.	*/
  P_DIR_ENTRY_TYPE_OTHER = 3  /**< Other.	*/
} p_dirent_kind_t;

/** Structure with directory entry information. */
typedef struct p_dirent {
  char *name;  /**< Name.	*/
  p_dirent_kind_t type;  /**< Type.	*/
} p_dirent_t;

/**
 * @brief Creates a new #PDir object.
 * @param path Directory path.
 * @return Pointer to a newly created #PDir object in case of success, NULL
 * otherwise.
 * @param[out] error Error report object, NULL to ignore.
 * @since 0.0.1
 * @note If you want to create a new directory on a filesystem, use
 * p_dir_create() instead.
 */
P_API p_dir_t *p_dir_new(const byte_t *path,
  p_err_t **error);

/**
 * @brief Creates a new directory on a filesystem.
 * @param path Directory path.
 * @param mode Directory permissions to use, ignored on Windows.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @note Call returns TRUE if the directory @a path is already exists.
 * @note On OpenVMS operating system it creates intermediate directories as
 * well.
 */
P_API bool p_dir_create(const byte_t *path,
  int_t mode,
  p_err_t **error);

/**
 * @brief Removes an empty directory.
 * @param path Directory path to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * The directory @a path should be empty to be removed successfully.
 */
P_API bool p_dir_remove(const byte_t *path,
  p_err_t **error);

/**
 * @brief Checks whether a directory exists or not.
 * @param path Directory path.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_API bool p_dir_is_exists(const byte_t *path);

/**
 * @brief Gets the original directory path used to create a #PDir object.
 * @param dir #PDir object to retrieve the path from.
 * @return The directory path in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned string. Use p_free() to free memory
 * after usage.
 */
P_API byte_t *p_dir_get_path(const p_dir_t *dir);

/**
 * @brief Gets the next directory entry info.
 * @param dir Directory to get the next entry from.
 * @param[out] error Error report object, NULL to ignore.
 * @return Info for the next entry in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned object. Use p_dir_entry_free() to free
 * memory of the directory entry after usage.
 *
 * An error is set only if it is occurred. You should check the @a error object
 * for #P_ERR_IO_NO_MORE code.
 */
P_API p_dirent_t *p_dir_get_next_entry(p_dir_t *dir,
  p_err_t **error);

/**
 * @brief Resets a directory entry pointer.
 * @param dir Directory to reset the entry pointer.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_API bool p_dir_rewind(p_dir_t *dir,
  p_err_t **error);

/**
 * @brief Frees #PDirEntry object.
 * @param entry #PDirEntry to free.
 * @since 0.0.1
 */
P_API void p_dir_entry_free(p_dirent_t *entry);

/**
 * @brief Frees #PDir object.
 * @param dir #PDir to free.
 * @since 0.0.1
 */
P_API void p_dir_free(p_dir_t *dir);

#endif /* P_DIR_H__ */
