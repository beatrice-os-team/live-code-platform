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
  Copyright  2025  zhang-mohan

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef FILE_SRC_EXT_STR_INDEX_C
#define FILE_SRC_EXT_STR_INDEX_C

/*
class str:
    def find(fs, ss): 返回在 fs 中以 ss 开头的字符串
        pass
    def split(fs, ss): 返回 fs 中以 ss 分割的字符串列表
        pass
    def mid(s, start, end): 获取 s 中 start 到 end 的子串
        pass
    def reverse(s, encode): 反转字符串， encode = 'utf8'? 'ascii'
        pass
    def upper(s): 转换字符串为大写
        pass
    def lower(s): 转换字符串为小写
        pass
    def concat( ... ): 连接字符串
        pass
    def count(fs, ss): 统计 fs 字符串中字符串 ss 的个数
        pass
    def new(s, n): 将 n 个 s 组成的字符串
        pass
*/

#include "losu.h"
#include "losu_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t __utf8len(uint8_t c) {
  if (c < 0x80)
    return 1;
  else if (c < 0xE0)
    return 2;
  else if (c < 0xF0)
    return 3;
  else if (c < 0xF8)
    return 4;
  else if (c < 0xFC)
    return 5;
  else if (c < 0xFE)
    return 6;
  else
    return 0;
}
static int32_t this_mid(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 3) {
    vm_error(vm, "lstr::mid(): wrong number of arguments");
    return 0;
  }
  const char* s = obj_tostr(vm, &argv[0]);
  int64_t slen = obj_getstrlen(vm, &argv[0]);
  int64_t start = obj_getnum(vm, &argv[1]);
  int64_t end = obj_getnum(vm, &argv[2]);
  // check & limit
  if (start >= slen || start < 0) {
    vm_error(vm, "lstr::mid(): start index out of range");
    return 0;
  }
  if (end < start)
    end = slen;
  stack_push(vm, obj_newstrlen(vm, s + start, end - start + 1));
  return 1;
}

static int32_t this_reverse(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "lstr::reverse(): wrong number of arguments");
    return 0;
  }
  const char* s = obj_tostr(vm, &argv[0]);
  int64_t slen = obj_getstrlen(vm, &argv[0]);
  int64_t count = slen - 1;
  losu_ctype_bool isutf8 = argc > 1 ? obj_tonum(vm, &argv[1]) : 0;
  char ans[slen];
  if (isutf8) {
    for (int64_t i = 0; i < slen;) {
      int8_t clen = __utf8len(s[i]);
      for (int8_t j = clen - 1, k = 0; j >= 0; k++, j--)
        ans[count - j] = s[i + k];
      i += clen;
      count -= clen;
    }
  } else {
    for (int64_t i = 0; i < slen; i++) {
      ans[i] = s[slen - i - 1];
    }
  }
  stack_push(vm, obj_newstrlen(vm, ans, slen));
  return 1;
}
static int32_t this_upper(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "lstr::upper(): wrong number of arguments");
    return 0;
  }
  const char* s = obj_tostr(vm, &argv[0]);
  int64_t slen = obj_getstrlen(vm, &argv[0]);
  char ans[slen];
  for (int64_t i = 0; i < slen; i++) {
    if (s[i] >= 'a' && s[i] <= 'z')
      ans[i] = s[i] - 'a' + 'A';
    else
      ans[i] = s[i];
  }
  stack_push(vm, obj_newstrlen(vm, ans, slen));
  return 1;
}
static int32_t this_lower(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "lstr::lower(): wrong number of arguments");
    return 0;
  }
  const char* s = obj_tostr(vm, &argv[0]);
  int64_t slen = obj_getstrlen(vm, &argv[0]);
  char ans[slen];
  for (int64_t i = 0; i < slen; i++) {
    if (s[i] >= 'A' && s[i] <= 'Z')
      ans[i] = s[i] - 'A' + 'a';
    else
      ans[i] = s[i];
  }
  stack_push(vm, obj_newstrlen(vm, ans, slen));
  return 1;
}
static int32_t this_concat(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "lstr::concat: wrong number of arguments");
    return 0;
  }
  int64_t anslen = 0;
  int64_t slenlist[argc];
  char* slist[argc];
  for (int32_t i = 0; i < argc; i++) {
    slist[i] = (char*)obj_tostr(vm, &argv[i]);
    slenlist[i] = obj_getstrlen(vm, &argv[i]);
    anslen += slenlist[i];
  }
  char ans[anslen];
  int64_t pos = 0;
  for (int32_t i = 0; i < argc; i++) {
    memcpy(ans + pos, slist[i], slenlist[i]);
    pos += slenlist[i];
  }
  stack_push(vm, obj_newstrlen(vm, ans, anslen));
  return 1;
}
static int32_t this_new(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 2) {
    vm_error(vm, "lstr::new() wrong number of arguments");
    return 0;
  }
  const char* s = obj_tostr(vm, &argv[0]);
  int64_t slen = obj_getstrlen(vm, &argv[0]);
  int32_t n = obj_tonum(vm, &argv[1]);
  char ans[n * slen];
  for (int i = 0; i < n; i++)
    memcpy(ans + i * slen, s, slen);
  stack_push(vm, obj_newstrlen(vm, ans, n * slen));
  return 1;
}

losu_object losu_ext_lstr(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "mid", obj_newfunc(vm, this_mid));
  obj_unitsetstr(vm, obj, "reverse", obj_newfunc(vm, this_reverse));
  obj_unitsetstr(vm, obj, "upper", obj_newfunc(vm, this_upper));
  obj_unitsetstr(vm, obj, "lower", obj_newfunc(vm, this_lower));
  obj_unitsetstr(vm, obj, "concat", obj_newfunc(vm, this_concat));
  obj_unitsetstr(vm, obj, "new", obj_newfunc(vm, this_new));

  return obj;
}
#endif