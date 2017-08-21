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

/**
 * @file p/file.h
 * @brief File operations
 * @author Alexander Saprykin
 *
 * To check file existance use p_file_is_exists(). To remove an exisiting file
 * use p_file_remove().
 *
 * #P_DIR_SEPARATOR provides a platform independent directory separator symbol
 * which you can use to form file or directory path.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef P_FILE_H__
#define P_FILE_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/error.h"

/**
 * @def P_DIR_SEPARATOR
 * @brief A directory separator.
 */
#if defined (P_OS_WIN) || defined (P_OS_OS2)
#  define P_DIR_SEPARATOR "\\"
#else
#  define P_DIR_SEPARATOR "/"
#endif

/**
 * @brief Checks whether a file exists or not.
 * @param file File name to check.
 * @return TRUE if the file exists, FALSE otherwise.
 * @since 0.0.1
 *
 * On Windows this call doesn't resolve symbolic links, while on UNIX systems
 * does.
 */
P_API bool p_file_is_exists(const byte_t *file);

/**
 * @brief Removes a file from the disk.
 * @param file File name to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if the file was successully removed, FALSE otherwise.
 * @since 0.0.1
 *
 * This call doesn't resolve symbolic links and remove a symbolic link if the
 * given path points to it.
 */
P_API bool p_file_remove(const byte_t *file,
  p_err_t **error);

#endif /* P_FILE_H__ */
