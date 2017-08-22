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

/*!@file p/dir.h
 * @brief Filesystem interface
 * @author Alexander Saprykin
 *
 * A traditional filesystem can be presented as a combination of directories and
 * files within a defined hierarchy. A directory contains the so called entries:
 * files and other directories. #dir_t allows to iterate through these entries
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
#ifndef P_DIR_H__
# define P_DIR_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/err.h"

/*!@brief Directory opaque data structure. */
typedef struct dir dir_t;
typedef struct p_dirent dirent_t;

/*!@brief Directory entry types. */
enum dirent_kind {

  /*!@brief Directory. */
  P_DIRENT_DIR = 1,

  /*!@brief File. */
  P_DIRENT_FILE = 2,

  /*!@brief Other. */
  P_DIRENT_OTHER = 3
};

typedef enum dirent_kind dirent_kind_t;

/*!@brief Structure with directory entry information. */
struct p_dirent {

  /*!@brief Name. */
  char *name;

  /*!@brief Type. */
  dirent_kind_t type;
};

/*!@brief Creates a new #dir_t object.
 * @param path Directory path.
 * @return Pointer to a newly created #dir_t object in case of success, NULL
 * otherwise.
 * @param[out] error Error report object, NULL to ignore.
 * @since 0.0.1
 * @note If you want to create a new directory on a filesystem, use
 * p_dir_create() instead.
 */
P_API dir_t *
p_dir_new(const byte_t *path, err_t **error);

/*!@brief Creates a new directory on a filesystem.
 * @param path Directory path.
 * @param mode Directory permissions to use, ignored on Windows.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 * @note Call returns true if the directory @a path is already exists.
 * @note On OpenVMS operating system it creates intermediate directories as
 * well.
 */
P_API bool
p_dir_create(const byte_t *path, int_t mode, err_t **error);

/*!@brief Removes an empty directory.
 * @param path Directory path to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 *
 * The directory @a path should be empty to be removed successfully.
 */
P_API bool
p_dir_remove(const byte_t *path, err_t **error);

/*!@brief Checks whether a directory exists or not.
 * @param path Directory path.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
P_API bool
p_dir_is_exists(const byte_t *path);

/*!@brief Gets the original directory path used to create a #dir_t object.
 * @param dir #dir_t object to retrieve the path from.
 * @return The directory path in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned string. Use p_free() to free memory
 * after usage.
 */
P_API byte_t *
p_dir_get_path(const dir_t *dir);

/*!@brief Gets the next directory entry info.
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
P_API dirent_t *
p_dir_get_next_entry(dir_t *dir, err_t **error);

/*!@brief Resets a directory entry pointer.
 * @param dir Directory to reset the entry pointer.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
P_API bool
p_dir_rewind(dir_t *dir, err_t **error);

/*!@brief Frees #dirent_t object.
 * @param entry #dirent_t to free.
 * @since 0.0.1
 */
P_API void
p_dir_entry_free(dirent_t *entry);

/*!@brief Frees #dir_t object.
 * @param dir #dir_t to free.
 * @since 0.0.1
 */
P_API void
p_dir_free(dir_t *dir);

#endif /* !P_DIR_H__ */
