/*
 * Copyright (C) 2012-2016 Alexander Saprykin <xelfium@gmail.com>
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

/*!@file p/inifile.h
 * @brief INI file parser
 * @author Alexander Saprykin
 *
 * An INI file is usually used for storing configuration information. It
 * consists of sections: every section starts with a line containing the name in
 * square brackets (i.e. [section_name]). After that line all the following
 * parameters will belong to that section until another section begins.
 *
 * Each section has a list of key-value pairs. Empty sections are not permitted
 * (they will be skipped). Every key-value pair is represented with a line in
 * the `key = value` format. If a section has several values with the same key
 * the last one will be used. A value is parsed by the first in-order '='
 * symbol. All the following '=' occurrences belong to the value.
 *
 * All symbols after '#' and ';' (even at the line ending) are the comments and
 * wouldn't be read. If you want to use them in values take the value inside the
 * "" or '' symbols. A section name line is not allowed to use the comment
 * symbols after the section name in the square brackets itself.
 *
 * Integer values can be written in the usual form.
 *
 * Floating point values can be written in any commonly used notation (i.e. with
 * the decimal point, in the exponential form using the 'e' character). The only
 * valid decimal point symbol is the '.'. There is no locale dependency on the
 * decimal point.
 *
 * Boolean values can be written in the form of 'true/false' or 'true/false', or
 * simply '0/1'.
 *
 * Any value can be interpreted as a string at any moment. Actually all the
 * values are stored internally as strings.
 *
 * A list of values can be stored between the '{}' symbols separated with
 * spaces. The list only supports string values, so you should convert them to
 * numbers manually. The list doesn't support strings with spaces - such strings
 * will be splitted.
 *
 * To parse a file, create #inifile_t with p_inifile_new() and then parse it
 * with the p_inifile_parse() routine.
 *
 * #inifile_t handles (skips) UTF-8/16/32 BOM characters (marks).
 *
 * Example of the INI file contents:
 * @code
 * [numeric_section]
 * numeric_value_1 = 1234 # One type of the comment
 * numeric_value_2 = 123  ; Comment is allowed here
 *
 * [floating_section]
 * float_value_1 = 123.3e10
 * float_value_2 = 123.19
 *
 * [boolean_section]
 * boolean_value_1 = true
 * boolean_value_2 = 0
 * boolean_value_3 = false
 *
 * [string_section]
 * string_value_1 = "Test string"
 * string_value_2 = 'Another test string'
 *
 * [list_section]
 * list_value = {123 val 7654}
 * @endcode
 */
#ifndef P_INIFILE_H__
# define P_INIFILE_H__

#include "p/macros.h"
#include "p/types.h"
#include "p/list.h"
#include "p/err.h"

/*!@brief INI file opaque data structure. */
typedef struct inifile inifile_t;

/*!@brief Creates a new #inifile_t for parsing.
 * @param path Path to a file to parse.
 * @return Newly allocated #inifile_t in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_API inifile_t *
p_inifile_new(const byte_t *path);

/*!@brief Frees memory and allocated resources of #inifile_t.
 * @param file #inifile_t to free.
 * @since 0.0.1
 */
P_API void
p_inifile_free(inifile_t *file);

/*!@brief Parses given #inifile_t.
 * @param file #inifile_t file to parse.
 * @param[out] error Error report object, NULL to ignore.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
P_API bool
p_inifile_parse(inifile_t *file, err_t **error);

/*!@brief Checks whether #inifile_t was already parsed or not.
 * @param file #inifile_t to check.
 * @return true if the file was already parsed, false otherwise.
 * @since 0.0.1
 */
P_API bool
p_inifile_is_parsed(const inifile_t *file);

/*!@brief Gets all the sections from a given file.
 * @param file #inifile_t to get the sections from. The @a file should be parsed
 * before.
 * @return #list_t of section names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_API list_t *
p_inifile_sections(const inifile_t *file);

/*!@brief Gets all the keys from a given section.
 * @param file #inifile_t to get the keys from. The @a file should be parsed
 * before.
 * @param section Section name to get the keys from.
 * @return #list_t of key names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_API list_t *
p_inifile_keys(const inifile_t *file, const byte_t *section);

/*!@brief Checks whether a key exists.
 * @param file #inifile_t to check in. The @a file should be parsed before.
 * @param section Section to check the key in.
 * @param key Key to check.
 * @return true if @a key exists, false otherwise.
 * @since 0.0.1
 */
P_API bool
p_inifile_is_key_exists(const inifile_t *file, const byte_t *section,
  const byte_t *key);

/*!@brief Gets specified parameter's value as a string.
 * @param file #inifile_t to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() the returned string after
 * usage.
 */
P_API byte_t *
p_inifile_parameter_string(const inifile_t *file, const byte_t *section,
  const byte_t *key, const byte_t *default_val);

/*!@brief Gets specified parameter's value as an integer.
 * @param file #inifile_t to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_API int
p_inifile_parameter_int(const inifile_t *file, const byte_t *section,
  const byte_t *key, int default_val);

/*!@brief Gets specified parameter's value as a floating point.
 * @param file #inifile_t to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_API double
p_inifile_parameter_double(const inifile_t *file, const byte_t *section,
  const byte_t *key, double default_val);

/*!@brief Gets specified parameter's value as a boolean.
 * @param file #inifile_t to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */

P_API bool
p_inifile_parameter_boolean(const inifile_t *file, const byte_t *section,
  const byte_t *key, bool default_val);

/*!@brief Gets specified parameter's value as a list of strings separated with
 * the spaces or tabs.
 * @param file #inifile_t to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @return #list_t of strings. NULL will be returned if no parameter with the
 * given name exists.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_API list_t *
p_inifile_parameter_list(const inifile_t *file, const byte_t *section,
  const byte_t *key);

#endif /* !P_INIFILE_H__ */
