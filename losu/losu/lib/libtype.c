/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like restrictions. Losu qualifies
  as open source software. However, Losu is not public property, and founder
  'chen-chaochen' retains its copyright.

    Losu has been registered with the National Copyright Administration of the
  People's Republic of China, and adopts the MIT license as the copyright
  licensing contract under which the right holder conditionally licenses its
  reproduction, distribution, and modification rights to an unspecified public.

    If you use Losu, please follow the public MIT agreement or choose to enter
  into a dedicated license agreement with us.

  The MIT LICENSE is as follows
  --------------------
  Copyright  2020  chen-chaochen

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROlosu_syntax_exptype_indexDED “AS IS”, WITHOUT WARRANTY OF
  ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
  EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FILE_SRC_LIB_LIBTYPE_C
#define FILE_SRC_LIB_LIBTYPE_C

#include "libtype.h"
#include "losu_mem.h"
#include "losu_object.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize the type library.
 * @param vm the losu vm handle
 */
losu_extern_t void losu_lib_init_type(losu_vm_t vm) {
  // type
  vm_setval(vm, "type", obj_newfunc(vm, losu_lib_type_type));
  // int float
  vm_setval(vm, "int", obj_newfunc(vm, losu_lib_type_int));
  vm_setval(vm, "float", obj_newfunc(vm, losu_lib_type_float));
  // str asc chr
  vm_setval(vm, "str", obj_newfunc(vm, losu_lib_type_str));
  vm_setval(vm, "chr", obj_newfunc(vm, losu_lib_type_chr));
  // len
  vm_setval(vm, "len", obj_newfunc(vm, losu_lib_type_len));
  // stringify
  // vm_setval(vm, "stringify", obj_newfunc(vm, losu_lib_type_stringify));
}

static int32_t losu_lib_type_type(losu_vm_t vm,
                                  int32_t argc,
                                  losu_object argv[]) {
  losu_object ans = argc ? argv[0] : obj_newnull(vm);
  stack_push(vm, obj_newstr(vm, obj_typestr(vm, &ans)));
  return 1;
}
static int32_t losu_lib_type_int(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  losu_object ans;
  if (argc) {
    obj_tonum(vm, &argv[0]);
    ovnumber(&argv[0]) = (int32_t)ovnumber(&argv[0]);
    ans = argv[0];
  } else {
    ans = obj_newnum(vm, 0);
  }
  stack_push(vm, ans);
  return 1;
  return 1;
}
static int32_t losu_lib_type_float(losu_vm_t vm,
                                   int32_t argc,
                                   losu_object argv[]) {
  losu_object ans;
  if (argc) {
    obj_tonum(vm, &argv[0]);
    ans = argv[0];
  } else {
    ans = obj_newnum(vm, NAN);
  }
  stack_push(vm, ans);
  return 1;
}
static int32_t losu_lib_type_str(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  losu_object ans;
  if (argc) {
    obj_tostr(vm, &argv[0]);
    ans = argv[0];
  } else {
    ans = obj_newstr(vm, "");
  }
  stack_push(vm, ans);
  return 1;
}

static int32_t losu_lib_type_chr(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  char* tmp = losu_mem_malloc(vm, argc + 1);
  memset(tmp, 0, argc + 1);
  for (int32_t i = 0; i < argc; i++)
    tmp[i] = (char)(obj_tonum(vm, &argv[i]));
  stack_push(vm, obj_newstrlen(vm, tmp, argc));
  return 1;
}
static int32_t losu_lib_type_len(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  losu_vm_vaarg_t va = arg_start(vm, argc);
  for (int32_t i = 0; i < argc; i++) {
    if (obj_type(vm, &argv[i]) == losu_object_type_string)
      arg_add(va, losu_object_type_number,
              (losu_ctype_number)(ovIstr(&argv[i])->len));
    else
      arg_add(va, losu_object_type_false);
  }
  return arg_tostack(va);
}

/* 
static uint32_t utf8_to_codepoint(const unsigned char* s, int* len);
static int escaped_unicode_length(const char* s);
char* stringify(const char* input);
static int32_t losu_lib_type_stringify(losu_vm_t vm,
                                       int32_t argc,
                                       losu_object argv[]) {
  if (!argc) {
    stack_push(vm, obj_newstr(vm, "false"));
  } else {
    const char* input = obj_tostr(vm, &argv[0]);
    char* buff = stringify(input);
    stack_push(vm, obj_newstr(vm, buff));
    losu_mem_free(vm, buff);
  }
  return 1;
}

static uint32_t utf8_to_codepoint(const unsigned char* s, int* len) {
  uint32_t cp = 0;
  if ((s[0] & 0x80) == 0x00) {
    cp = s[0];
    *len = 1;
  } else if ((s[0] & 0xE0) == 0xC0) {
    cp = ((uint32_t)(s[0] & 0x1F)) << 6 | (s[1] & 0x3F);
    *len = 2;
  } else if ((s[0] & 0xF0) == 0xE0) {
    cp = ((uint32_t)(s[0] & 0x0F)) << 12 | ((uint32_t)(s[1] & 0x3F)) << 6 |
         (s[2] & 0x3F);
    *len = 3;
  } else if ((s[0] & 0xF8) == 0xF0) {
    cp = ((uint32_t)(s[0] & 0x07)) << 18 | ((uint32_t)(s[1] & 0x3F)) << 12 |
         ((uint32_t)(s[2] & 0x3F)) << 6 | (s[3] & 0x3F);
    *len = 4;
  } else {
    cp = 0xFFFD;
    *len = 1;
  }
  return cp;
}

static int escaped_unicode_length(const char* s) {
  int len = 0;
  const unsigned char* us = (const unsigned char*)s;
  while (*us) {
    int utf8_len;
    uint32_t cp = utf8_to_codepoint(us, &utf8_len);
    us += utf8_len;

    if (cp < 0x80) {
      len += 1;
    } else if (cp < 0xFFFF) {
      len += 6;
    } else {
      len += 12;
    }

    if (cp == '"' || cp == '\\') {
      len += 1;
    }
  }
  return len + 2;
}

char* stringify(const char* input) {
  int out_len = escaped_unicode_length(input);
  char* out = (char*)malloc(out_len + 1);
  char* p = out;

  *p++ = '"';

  const unsigned char* s = (const unsigned char*)input;
  while (*s) {
    int utf8_len;
    uint32_t cp = utf8_to_codepoint(s, &utf8_len);
    s += utf8_len;

    if (cp == '"' || cp == '\\') {
      *p++ = '\\';
      *p++ = (char)cp;
    } else if (cp < 0x80) {
      *p++ = (char)cp;
    } else if (cp < 0xFFFF) {
      snprintf(p, 7, "\\u%04X", cp);
      p += 6;
    } else {
      // Surrogate pair
      uint32_t w1 = 0xD800 + ((cp - 0x10000) >> 10);
      uint32_t w2 = 0xDC00 + ((cp - 0x10000) & 0x3FF);
      snprintf(p, 13, "\\u%04x\\u%04x", w1, w2);
      p += 12;
    }
  }

  *p++ = '"';
  *p = '\0';

  return out;
}
 */
#endif
