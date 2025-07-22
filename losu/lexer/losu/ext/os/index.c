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

#ifndef FILE_SRC_EXT_OS_INDEX_C
#define FILE_SRC_EXT_OS_INDEX_C

/*
class os:
    def system(cmd):
        pass
    def exec(cmd, mode, ... ):
        pass
    def getenv(name):
        pass
    def setenv(name, value):
        pass
*/

#include "losu.h"
#include "losu_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int32_t this_system(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "os::system(): wrong number of arguments");
    return 0;
  }
  int i = system(obj_tostr(vm, &argv[0]));
  stack_push(vm, obj_newnum(vm, i));
  return 1;
}
static int32_t this_exec(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "os::exec(): wrong number of arguments");
    return 0;
  }
  uint8_t mode = (argc > 1 ? obj_tostr(vm, &argv[1]) : "r")[0] == 'w';
  // 0 is read, 1 is write
  FILE* p = NULL;
  if (mode) {
    p = popen(obj_tostr(vm, &argv[0]), "w");
  } else {
    p = popen(obj_tostr(vm, &argv[0]), "r");
  }
  if (!p) {
    vm_error(vm, "os::exec(): popen() failed");
    return 0;
  }
  if (mode) {
    if (argc > 2) {
      const char* s = obj_tostr(vm, &argv[2]);
      size_t len = obj_getstrlen(vm, &argv[2]);
      fwrite(s, 1, len, p);
    }
    stack_push(vm, obj_newtrue(vm));
  } else {
    char buffer[128];
    char* result = NULL;
    size_t total = 0, n;

    while ((n = fread(buffer, 1, sizeof(buffer), p)) > 0) {
      result = losu_mem_realloc(vm, result, total + n);
      memcpy(result + total, buffer, n);
      total += n;
    }
    stack_push(vm, obj_newstrlen(vm, result, total));
    losu_mem_free(vm, result);
  }
  pclose(p);
  return 1;
}
static int32_t this_getenv(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "os::getenv(): wrong number of arguments");
    return 0;
  }
  char* ans = getenv(obj_tostr(vm, &argv[0]));
  if (ans) {
    stack_push(vm, obj_newstr(vm, ans));
  } else {
    stack_push(vm, obj_newnull(vm));
  }
  return 1;
}
static int32_t this_setenv(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc < 2) {
    vm_error(vm, "os::setenv(): wrong number of arguments");
    return 0;
  }
  const char* name = obj_tostr(vm, &argv[0]);
  const char* value = obj_tostr(vm, &argv[1]);
  size_t len = obj_getstrlen(vm, &argv[0]) + obj_getstrlen(vm, &argv[1]) + 1;
  char* env = losu_mem_malloc(vm, len);
  snprintf(env, len, "%s=%s", name, value);
  if (putenv(env)) {
    losu_mem_free(vm, env);
    vm_error(vm, "os::setenv(): putenv() failed");
    return 0;
  }
  stack_push(vm, obj_newtrue(vm));
  return 1;
}

losu_object losu_ext_os(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "system", obj_newfunc(vm, this_system));
  obj_unitsetstr(vm, obj, "exec", obj_newfunc(vm, this_exec));
  obj_unitsetstr(vm, obj, "getenv", obj_newfunc(vm, this_getenv));
  obj_unitsetstr(vm, obj, "setenv", obj_newfunc(vm, this_setenv));
  return obj;
}

#endif