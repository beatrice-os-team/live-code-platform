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

#ifndef FILE_SRC_LIB_LIBVM_C
#define FILE_SRC_LIB_LIBVM_C

#include "libvm.h"
#include "losu_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
losu_extern_t void losu_lib_init_vm(losu_vm_t vm) {
  vm_setval(vm, "eval", obj_newfunc(vm, losu_lib_vm_eval));
  vm_setval(vm, "exec", obj_newfunc(vm, losu_lib_vm_exec));
  vm_setval(vm, "package", obj_newfunc(vm, losu_lib_vm_package));
  vm_setval(vm, "assert", obj_newfunc(vm, losu_lib_vm_assert));
}

static int32_t losu_lib_vm_eval(losu_vm_t vm,
                                int32_t argc,
                                losu_object argv[]) {
  losu_object_t oldtop = vm->top;
  if (argc == 0)
    return 0;
  if (vm_loadbyte(vm, obj_getstr(vm, &argv[0]), obj_getstrlen(vm, &argv[0]),
                  "inline string"))
    return 0;
  stack_rawcall(vm, 0, -1);
  return vm->top - oldtop;
}
static int32_t losu_lib_vm_exec(losu_vm_t vm,
                                int32_t argc,
                                losu_object argv[]) {
  losu_object_t oldtop = vm->top;
  if (argc == 0)
    return 0;
  const char* file = obj_getstr(vm, &argv[0]);
  if (vm_loadfile(vm, file, file))
    return 0;
  stack_rawcall(vm, 0, -1);
  return vm->top - oldtop;
}
static int32_t losu_lib_vm_package(losu_vm_t vm,
                                   int32_t argc,
                                   losu_object argv[]) {
  losu_object_t oldtop = vm->top;
  if (argc == 0)
    return 0;

  const char* oldpath = vm->path;
  const char* losupath = vm->path ? vm->path : "./";
  const char* usrpath = obj_getstr(vm, &argv[0]);
  size_t len = strlen(losupath) + strlen(usrpath) + 16;

  char* newpath = losu_mem_malloc(vm, len);
  char* filename = losu_mem_malloc(vm, len);
  snprintf(newpath, len, "%s%s/", losupath, usrpath);
  snprintf(filename, len, "%smain.losu", newpath);
  vm->path = newpath;
  if (vm_loadfile(vm, filename, filename)) {
    vm->path = oldpath;
    losu_mem_free(vm, newpath);
    vm_error(vm, "package '%s' : '%s' not found ", usrpath, filename);
    return 0;
  }
  stack_rawcall(vm, 0, -1);
  vm->path = oldpath;
  losu_mem_free(vm, filename);
  losu_mem_free(vm, newpath);
  return vm->top - oldtop;
}
static int32_t losu_lib_vm_assert(losu_vm_t vm,
                                  int32_t argc,
                                  losu_object argv[]) {
  losu_object assert = argv[0];
  if (argc == 0)
    assert = obj_newnull(vm);
  if (obj_type(vm, &assert) == losu_object_type_false)
    vm_error(vm, "assertion failed: %s",
             argc > 1 ? obj_tostr(vm, &argv[1]) : "");
  stack_push(vm, obj_newtrue(vm));
  return 1;
}

#endif