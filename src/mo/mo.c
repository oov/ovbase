#include <ovmo.h>

#include <string.h>

#ifndef _WIN32
#  include <locale.h>
#  include <stdio.h>
#endif

#include <ovarray.h>

struct mo_msg {
  char const *id;
  size_t id_len;
  char const *str;
  size_t str_len;
};

enum mo_plural_form {
  mo_plural_form_unknown,
  mo_plural_form_n1, // nplurals=1; plural=0;
  mo_plural_form_n2, // nplurals=2; plural=(n != 1);
};

struct mo {
  struct mo_msg *msg;
  size_t msg_len;
  enum mo_plural_form plural_form;
};

static uint32_t read_le(void const *const p) {
  uint8_t const *const bytes = (uint8_t const *)p;
  return ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static uint32_t read_be(void const *const p) {
  uint8_t const *const bytes = (uint8_t const *)p;
  return ((uint32_t)bytes[3]) | ((uint32_t)bytes[2] << 8) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[0] << 24);
}

static bool read_header(char *const dest, size_t const destlen, char const *const name, char const *const header) {
  char const *cur = header;
  size_t const namelen = strlen(name);
  while (*cur != '\0') {
    char const *lineend = strchr(cur, '\n');
    if (!lineend) {
      lineend = cur + strlen(cur) - 1;
    }
    if (strncmp(cur, name, namelen) != 0 || cur[namelen] != ':') {
      cur = lineend + 1;
      continue;
    }
    char const *value = cur + namelen + 1;
    while (*value == ' ') {
      ++value;
    }
    size_t const valuelen = (size_t)(lineend - value);
    if (valuelen >= destlen) {
      return false;
    }
    strncpy(dest, value, valuelen);
    dest[valuelen] = '\0';
    return true;
  }
  return false;
}

bool mo_parse(struct mo **const mpp, void const *const ptr, size_t const ptrlen, struct ov_error *const err) {
  if (!mpp || *mpp || !ptr) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  if (ptrlen < 28) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    return false;
  }

  uint8_t const *p = (uint8_t const *)ptr;
  uint32_t (*read)(void const *const) = NULL;
  struct mo *mp = NULL;
  uint32_t num_strings = 0;
  uint32_t orig_offset = 0;
  uint32_t trans_offset = 0;

  {
    uint32_t const magic = read_le(p);
    bool const le = magic == 0x950412de;
    bool const be = magic == 0xde120495;
    if (!le && !be) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
      goto cleanup;
    }
    read = le ? read_le : read_be;
  }

  {
    uint32_t const version = read(p + 4);
    if (version != 0) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
      goto cleanup;
    }
  }

  num_strings = read(p + 8);
  orig_offset = read(p + 12);
  trans_offset = read(p + 16);
  if (ptrlen <= orig_offset || ptrlen <= trans_offset) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    goto cleanup;
  }

  if (!OV_REALLOC(mpp, 1, sizeof(struct mo), err)) {
    OV_ERROR_TRACE(err);
    goto cleanup;
  }
  mp = *mpp;
  *mp = (struct mo){0};

  if (!OV_REALLOC(&mp->msg, num_strings, sizeof(struct mo_msg), err)) {
    OV_ERROR_TRACE(err);
    goto cleanup;
  }
  memset(mp->msg, 0, sizeof(struct mo_msg) * num_strings);
  mp->msg_len = num_strings;

  for (uint32_t i = 0; i < num_strings; ++i) {
    struct mo_msg *msg = mp->msg + i;
    size_t const o_len = (size_t)(read(p + orig_offset + i * 8));
    size_t const o_offset = (size_t)(read(p + orig_offset + i * 8 + 4));
    size_t const t_len = (size_t)(read(p + trans_offset + i * 8));
    size_t const t_offset = (size_t)(read(p + trans_offset + i * 8 + 4));
    if (o_offset + o_len > ptrlen || t_offset + t_len > ptrlen) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
      goto cleanup;
    }
    msg->id = (char const *)p + o_offset;
    msg->id_len = o_len;
    msg->str = (char const *)p + t_offset;
    msg->str_len = t_len;
  }

  if (mp->msg_len > 0 && mp->msg[0].id_len == 0 && mp->msg[0].str_len > 0) {
    enum {
      plural_form_buf_size = 256,
    };
    char plural_form[plural_form_buf_size];
    if (read_header(plural_form, plural_form_buf_size, "Plural-Forms", mp->msg[0].str)) {
      if (strstr(plural_form, "nplurals=1;") == plural_form) {
        mp->plural_form = mo_plural_form_n1;
      } else if (strstr(plural_form, "nplurals=2;") == plural_form) {
        mp->plural_form = mo_plural_form_n2;
      }
    }
  }

  return true;

cleanup:
  if (mp) {
    mo_free(mpp);
  }
  return false;
}

void mo_free(struct mo **const mpp) {
  if (!mpp || !*mpp) {
    return;
  }
  struct mo *mp = *mpp;
  if (mp->msg) {
    OV_FREE(&mp->msg);
  }
  OV_FREE(mpp);
}

static struct mo_msg *find(struct mo const *const mp, char const *const id) {
  size_t l = 0, r = mp->msg_len - 1;
  while (l <= r) {
    size_t const mid = l + (r - l) / 2;
    int const cmp = strcmp(mp->msg[mid].id, id);
    if (cmp == 0) {
      return mp->msg + mid;
    }
    if (cmp < 0) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return NULL;
}

char const *mo_gettext(struct mo const *const mp, char const *const id) {
  if (!mp) {
    return id;
  }
  struct mo_msg *msg = find(mp, id);
  return msg ? msg->str : id;
}

char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id) {
  if (!mp) {
    return id;
  }
  char *tmp = NULL;
  struct mo_msg *msg = NULL;
  size_t const ctxtlen = strlen(ctxt);
  size_t const idlen = strlen(id);
  struct ov_error err = {0};
  if (!OV_ARRAY_GROW2(&tmp, ctxtlen + 1 + idlen + 1, &err)) {
    OV_ERROR_DESTROY(&err);
    goto cleanup;
  }
  strcpy(tmp, ctxt);
  strcpy(tmp + ctxtlen, "\x04");
  strcpy(tmp + ctxtlen + 1, id);
  msg = find(mp, tmp);
cleanup:
  if (tmp) {
    OV_ARRAY_DESTROY(&tmp);
  }
  return msg ? msg->str : id;
}

static char const *find_plural_form(char const *s, size_t len, unsigned long int const n) {
  unsigned long int i = 0;
  while (*s != '\0') {
    if (i == n) {
      return s;
    }
    char const *sep = (char const *)memchr(s, '\x00', len);
    if (!sep) {
      sep = s + len - 1;
    }
    size_t const sep_pos = (size_t)(sep - s);
    s += sep_pos + 1;
    len -= sep_pos + 1;
    ++i;
  }
  return NULL;
}

char const *
mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n) {
  if (!mp) {
    return n != 1 ? id_plural : id;
  }
  char const *r = NULL;
  struct mo_msg *msg = find(mp, id);
  if (msg) {
    // TODO: use implemented plural-form interpreter
    switch (mp->plural_form) {
    case mo_plural_form_unknown:
      break;
    case mo_plural_form_n1:
      r = msg->str;
      break;
    case mo_plural_form_n2:
      r = find_plural_form(msg->str, msg->str_len, n != 1);
      break;
    }
  }
  if (r) {
    return r;
  }
  return n != 1 ? id_plural : id;
}

static struct mo *g_mp = NULL;

void mo_set_default(struct mo *const mp) { g_mp = mp; }

struct mo *mo_get_default(void) { return g_mp; }

#ifndef _WIN32
bool mo_get_preferred_ui_languages(NATIVE_CHAR **dest, struct ov_error *const err) {
  if (!dest) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  char const *lang = setlocale(LC_MESSAGES, NULL);
  if (!lang) {
    lang = "C";
  }

  size_t const langlen = strlen(lang);
  if (!OV_ARRAY_GROW2(dest, langlen + 2, err)) {
    OV_ERROR_TRACE(err);
    return false;
  }

  // Copy language string
  for (size_t i = 0; i < langlen; ++i) {
    (*dest)[i] = (NATIVE_CHAR)lang[i];
  }

  // Remove locale modifier (e.g., "en_US.UTF-8" -> "en_US")
  size_t final_len = langlen;
  for (size_t i = 0; i < langlen; ++i) {
    if ((*dest)[i] == NSTR('.')) {
      final_len = i;
      break;
    }
  }

  // Add null terminator and set length
  (*dest)[final_len] = NSTR('\0');
  (*dest)[final_len + 1] = NSTR('\0');
  OV_ARRAY_SET_LENGTH(*dest, final_len + 2);

  return true;
}
#endif
