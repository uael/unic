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

/*!@file unic/file.h
 * @brief File operations
 * @author Alexander Saprykin
 *
 * To check file existence use u_file_is_exists(). To remove an exisiting file
 * use u_file_remove().
 *
 * #U_DIR_SEP provides a platform independent directory separator symbol
 * which you can use to form file or directory path.
 */
#ifndef U_FILE_H__
# define U_FILE_H__

#include "unic/macros.h"
#include "unic/types.h"
#include "unic/err.h"

/*!@def U_DIR_SEP
 * @brief A directory separator.
 */
#if defined (U_OS_WIN) || defined (U_OS_OS2)
# define U_DIR_SEP "\\"
#else
# define U_DIR_SEP "/"
#endif

/*!@brief Checks whether a file exists or not.
 * @param file File name to check.
 * @return true if the file exists, false otherwise.
 * @since 0.0.1
 *
 * On Windows this call doesn't resolve symbolic links, while on UNIX systems
 * does.
 */
U_API bool
u_file_is_exists(const byte_t *file);

/*!@brief Removes a file from the disk.
 * @param file File name to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return true if the file was successfully removed, false otherwise.
 * @since 0.0.1
 *
 * This call doesn't resolve symbolic links and remove a symbolic link if the
 * given path points to it.
 */
U_API bool
u_file_remove(const byte_t *file, err_t **error);

#endif /* !U_FILE_H__ */
