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

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef FILE_SRC_EXT_DLLIB_INDEX_C
#define FILE_SRC_EXT_DLLIB_INDEX_C

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static int32_t this_dlopen(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc)
    return 0;
  char filename[PATH_MAX];
  snprintf(filename, PATH_MAX, "%s%s", vm->path ? vm->path : "./",
           obj_tostr(vm, &argv[0]));
  void* handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
  if (!handle)
    vm_error(vm, "dlopen error: %s", dlerror());
  stack_push(vm, obj_newptr(vm, handle));
  return 1;
}

// static int32_t this_dlopen_blob(losu_vm_t vm,
//                                 int32_t argc,
//                                 losu_object argv[]) {
//   char tmpfile[L_tmpnam];
//   if (!argc)
//     return 0;
//   tmpnam(tmpfile);
//   FILE* fp = fopen(tmpfile, "wb");
//   if (!fp)
//     return 0;
//   fwrite(obj_tostr(vm, &argv[0]), sizeof(char), obj_getstrlen(vm, &argv[0]),
//          fp);
//   fclose(fp);
//   void* handle = dlopen(tmpfile, RTLD_NOW | RTLD_LOCAL);
//   if (!handle)
//     vm_error(vm, "dlopen error: %s", dlerror());
//   stack_push(vm, obj_newptr(vm, handle));
//   return 1;
// }

static int32_t this_dlsym(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc < 2)
    return 0;
  void* handle = obj_toptr(vm, &argv[0]);
  const char* name = obj_tostr(vm, &argv[1]);
  losu_vmapi_t func = (losu_vmapi_t)dlsym(handle, name);
  stack_push(vm, obj_newfunc(vm, func));
  return 1;
}

static int32_t this_dlclose(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc)
    return 0;
  dlclose(obj_toptr(vm, &argv[0]));
  return 0;
}

losu_object losu_ext_dylib(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "dlopen", obj_newfunc(vm, this_dlopen));
  // obj_unitsetstr(vm, obj, "dlopen_blob", obj_newfunc(vm, this_dlopen_blob));
  obj_unitsetstr(vm, obj, "dlsym", obj_newfunc(vm, this_dlsym));
  obj_unitsetstr(vm, obj, "dlclose", obj_newfunc(vm, this_dlclose));
  return obj;
}

#endif