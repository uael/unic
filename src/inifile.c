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

#include "p/err.h"
#include "p/inifile.h"
#include "p/mem.h"
#include "p/string.h"
#include "err-private.h"

#define  P_INI_FILE_MAX_LINE  1024

typedef struct PIniParameter_ {
  byte_t *name;
  byte_t *value;
} PIniParameter;

typedef struct PIniSection_ {
  byte_t *name;
  list_t *keys;
} PIniSection;

struct ini_file {
  byte_t *path;
  list_t *sections;
  bool is_parsed;
};

static PIniParameter *
pp_inifile_parameter_new(const byte_t *name,
  const byte_t *val);

static void
pp_inifile_parameter_free(PIniParameter *param);

static PIniSection *
pp_inifile_section_new(const byte_t *name);

static void
pp_inifile_section_free(PIniSection *section);

static byte_t *
pp_inifile_find_parameter(const ini_file_t *file,
  const byte_t *section, const byte_t *key);

static PIniParameter *
pp_inifile_parameter_new(const byte_t *name,
  const byte_t *val) {
  PIniParameter *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PIniParameter))) == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((ret->name = p_strdup(name)) == NULL)) {
    p_free(ret);
    return NULL;
  }
  if (P_UNLIKELY ((ret->value = p_strdup(val)) == NULL)) {
    p_free(ret->name);
    p_free(ret);
    return NULL;
  }
  return ret;
}

static void
pp_inifile_parameter_free(PIniParameter *param) {
  p_free(param->name);
  p_free(param->value);
  p_free(param);
}

static PIniSection *
pp_inifile_section_new(const byte_t *name) {
  PIniSection *ret;
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(PIniSection))) == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((ret->name = p_strdup(name)) == NULL)) {
    p_free(ret);
    return NULL;
  }
  return ret;
}

static void
pp_inifile_section_free(PIniSection *section) {
  p_list_foreach(section->keys, (fn_t) pp_inifile_parameter_free, NULL);
  p_list_free(section->keys);
  p_free(section->name);
  p_free(section);
}

static byte_t *
pp_inifile_find_parameter(const ini_file_t *file, const byte_t *section,
  const byte_t *key) {
  list_t *item;
  if (P_UNLIKELY (
    file == NULL || file->is_parsed == false || section == NULL ||
      key == NULL)) {
        return NULL;
  }
  for (item = file->sections; item != NULL; item = item->next) {
    if (strcmp(((PIniSection *) item->data)->name, section) == 0) {
      break;
    }
  }
  if (item == NULL) {
    return NULL;
  }
  for (item = ((PIniSection *) item->data)->keys; item != NULL;
    item = item->next) {
      if (strcmp(((PIniParameter *) item->data)->name, key) == 0) {
        return p_strdup(((PIniParameter *) item->data)->value);
      }
  }
  return NULL;
}

ini_file_t *
p_inifile_new(const byte_t *path) {
  ini_file_t *ret;
  if (P_UNLIKELY (path == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((ret = p_malloc0(sizeof(ini_file_t))) == NULL)) {
    return NULL;
  }
  if (P_UNLIKELY ((ret->path = p_strdup(path)) == NULL)) {
    p_free(ret);
    return NULL;
  }
  ret->is_parsed = false;
  return ret;
}

void
p_inifile_free(ini_file_t *file) {
  if (P_UNLIKELY (file == NULL)) {
    return;
  }
  p_list_foreach(file->sections, (fn_t) pp_inifile_section_free, NULL);
  p_list_free(file->sections);
  p_free(file->path);
  p_free(file);
}

bool
p_inifile_parse(ini_file_t *file,
  err_t **error) {
  PIniSection *section;
  PIniParameter *param;
  FILE *in_file;
  byte_t *dst_line, *tmp_str;
  byte_t src_line[P_INI_FILE_MAX_LINE + 1],
    key[P_INI_FILE_MAX_LINE + 1],
    value[P_INI_FILE_MAX_LINE + 1];
  int bom_shift;
  if (P_UNLIKELY (file == NULL)) {
    p_err_set_err_p(
      error,
      (int) P_ERR_IO_INVALID_ARGUMENT,
      0,
      "Invalid input argument"
    );
    return false;
  }
  if (file->is_parsed) {
    return true;
  }
  if (P_UNLIKELY ((in_file = fopen(file->path, "r")) == NULL)) {
    p_err_set_err_p(
      error,
      (int) p_err_get_last_io(),
      p_err_get_last_system(),
      "Failed to open file for reading"
    );
    return false;
  }
  dst_line = NULL;
  section = NULL;
  param = NULL;
  memset(src_line, 0, sizeof(src_line));
  while (fgets(src_line, sizeof(src_line), in_file) != NULL) {
    /* UTF-8, UTF-16 and UTF-32 BOM detection */
    if ((ubyte_t) src_line[0] == 0xEF && (ubyte_t) src_line[1] == 0xBB
      && (ubyte_t) src_line[2] == 0xBF) {
        bom_shift = 3;
    } else if (
      ((ubyte_t) src_line[0] == 0xFE && (ubyte_t) src_line[1] == 0xFF) ||
        ((ubyte_t) src_line[0] == 0xFF && (ubyte_t) src_line[1] == 0xFE)) {
          bom_shift = 2;
    } else if ((ubyte_t) src_line[0] == 0x00 && (ubyte_t) src_line[1] == 0x00 &&
      (ubyte_t) src_line[2] == 0xFE && (ubyte_t) src_line[3] == 0xFF) {
        bom_shift = 4;
    } else if ((ubyte_t) src_line[0] == 0xFF && (ubyte_t) src_line[1] == 0xFE &&
      (ubyte_t) src_line[2] == 0x00 && (ubyte_t) src_line[3] == 0x00) {
        bom_shift = 4;
    } else {
        bom_shift = 0;
    }
    dst_line = p_strchomp(src_line + bom_shift);
    if (dst_line == NULL) {
      continue;
    }

    /* This should not happen */
    if (P_UNLIKELY (strlen(dst_line) > P_INI_FILE_MAX_LINE)) {
      dst_line[P_INI_FILE_MAX_LINE] = '\0';
    }
    if (dst_line[0] == '[' && dst_line[strlen(dst_line) - 1] == ']' &&
      sscanf(dst_line, "[%[^]]", key) == 1) {
      /* New section found */
      if ((tmp_str = p_strchomp(key)) != NULL) {
        /* This should not happen */
        if (P_UNLIKELY (strlen(tmp_str) > P_INI_FILE_MAX_LINE)) {
          tmp_str[P_INI_FILE_MAX_LINE] = '\0';
        }
        strcpy(key, tmp_str);
        p_free(tmp_str);
        if (section != NULL) {
          if (section->keys == NULL) {
            pp_inifile_section_free(section);
          } else {
            file->sections = p_list_prepend(file->sections, section);
          }
        }
        section = pp_inifile_section_new(key);
      }
    } else if (sscanf(dst_line, "%[^=] = \"%[^\"]\"", key, value) == 2 ||
      sscanf(dst_line, "%[^=] = '%[^\']'", key, value) == 2 ||
      sscanf(dst_line, "%[^=] = %[^;#]", key, value) == 2) {
      /* New parameter found */
      if ((tmp_str = p_strchomp(key)) != NULL) {
        /* This should not happen */
        if (P_UNLIKELY (strlen(tmp_str) > P_INI_FILE_MAX_LINE)) {
          tmp_str[P_INI_FILE_MAX_LINE] = '\0';
        }
        strcpy(key, tmp_str);
        p_free(tmp_str);
        if ((tmp_str = p_strchomp(value)) != NULL) {
          /* This should not happen */
          if (P_UNLIKELY (strlen(tmp_str) > P_INI_FILE_MAX_LINE)) {
            tmp_str[P_INI_FILE_MAX_LINE] = '\0';
          }
          strcpy(value, tmp_str);
          p_free(tmp_str);
          if (strcmp(value, "\"\"") == 0 || (strcmp(value, "''") == 0)) {
            value[0] = '\0';
          }
          if (section != NULL
            && (param = pp_inifile_parameter_new(key, value)) != NULL) {
              section->keys = p_list_prepend(section->keys, param);
          }
        }
      }
    }
    p_free(dst_line);
    memset(src_line, 0, sizeof(src_line));
  }
  if (section != NULL) {
    if (section->keys == NULL) {
      pp_inifile_section_free(section);
    } else {
      file->sections = p_list_append(file->sections, section);
    }
  }
  if (P_UNLIKELY (fclose(in_file) != 0))
    P_WARNING ("ini_file_t::p_inifile_parse: fclose() failed");
  file->is_parsed = true;
  return true;
}

bool
p_inifile_is_parsed(const ini_file_t *file) {
  if (P_UNLIKELY (file == NULL)) {
    return false;
  }
  return file->is_parsed;
}

list_t *
p_inifile_sections(const ini_file_t *file) {
  list_t *ret;
  list_t *sec;
  if (P_UNLIKELY (file == NULL || file->is_parsed == false)) {
    return NULL;
  }
  ret = NULL;
  for (sec = file->sections; sec != NULL; sec = sec->next) {
    ret = p_list_prepend(ret, p_strdup(((PIniSection *) sec->data)->name));
  }
  return ret;
}

list_t *
p_inifile_keys(const ini_file_t *file,
  const byte_t *section) {
  list_t *ret;
  list_t *item;
  if (P_UNLIKELY (
    file == NULL || file->is_parsed == false || section == NULL)) {
      return NULL;
  }
  ret = NULL;
  for (item = file->sections; item != NULL; item = item->next) {
    if (strcmp(((PIniSection *) item->data)->name, section) == 0) {
      break;
    }
  }
  if (item == NULL) {
    return NULL;
  }
  for (item = ((PIniSection *) item->data)->keys; item != NULL;
    item = item->next) {
      ret = p_list_prepend(ret, p_strdup(((PIniParameter *) item->data)->name));
  }
  return ret;
}

bool
p_inifile_is_key_exists(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key) {
  list_t *item;
  if (P_UNLIKELY (
    file == NULL || file->is_parsed == false || section == NULL ||
      key == NULL)) {
        return false;
  }
  for (item = file->sections; item != NULL; item = item->next) {
    if (strcmp(((PIniSection *) item->data)->name, section) == 0) {
      break;
    }
  }
  if (item == NULL) {
    return false;
  }
  for (item = ((PIniSection *) item->data)->keys; item != NULL;
    item = item->next) {
      if (strcmp(((PIniParameter *) item->data)->name, key) == 0) {
        return true;
      }
  }
  return false;
}

byte_t *
p_inifile_parameter_string(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key,
  const byte_t *default_val) {
  byte_t *val;
  if ((val = pp_inifile_find_parameter(file, section, key)) == NULL) {
    return p_strdup(default_val);
  }
  return val;
}

int
p_inifile_parameter_int(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key,
  int default_val) {
  byte_t *val;
  int ret;
  if ((val = pp_inifile_find_parameter(file, section, key)) == NULL) {
    return default_val;
  }
  ret = atoi(val);
  p_free(val);
  return ret;
}

double
p_inifile_parameter_double(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key,
  double default_val) {
  byte_t *val;
  double ret;
  if ((val = pp_inifile_find_parameter(file, section, key)) == NULL) {
    return default_val;
  }
  ret = p_strtod(val);
  p_free(val);
  return ret;
}

bool
p_inifile_parameter_boolean(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key,
  bool default_val) {
  byte_t *val;
  bool ret;
  if ((val = pp_inifile_find_parameter(file, section, key)) == NULL) {
    return default_val;
  }
  if (strcmp(val, "true") == 0 || strcmp(val, "true") == 0) {
    ret = true;
  } else if (strcmp(val, "false") == 0 || strcmp(val, "false") == 0) {
    ret = false;
  } else if (atoi(val) > 0) {
    ret = true;
  } else {
    ret = false;
  }
  p_free(val);
  return ret;
}

list_t *
p_inifile_parameter_list(const ini_file_t *file,
  const byte_t *section,
  const byte_t *key) {
  list_t *ret = NULL;
  byte_t *val, *str;
  byte_t buf[P_INI_FILE_MAX_LINE + 1];
  size_t len, buf_cnt;
  if ((val = pp_inifile_find_parameter(file, section, key)) == NULL) {
    return NULL;
  }
  len = strlen(val);
  if (len < 3 || val[0] != '{' || val[len - 1] != '}') {
    p_free(val);
    return NULL;
  }

  /* Skip first brace '{' symbol */
  str = val + 1;
  buf[0] = '\0';
  buf_cnt = 0;
  while (*str && *str != '}') {
    if (!isspace(*((const ubyte_t *) str))) {
      buf[buf_cnt++] = *str;
    } else {
      buf[buf_cnt] = '\0';
      if (buf_cnt > 0) {
        ret = p_list_append(ret, p_strdup(buf));
      }
      buf_cnt = 0;
    }
    ++str;
  }
  if (buf_cnt > 0) {
    buf[buf_cnt] = '\0';
    ret = p_list_append(ret, p_strdup(buf));
  }
  p_free(val);
  return ret;
}
