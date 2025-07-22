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
#ifndef FILE_SRC_EXT_LOSU_INDEX_C
#define FILE_SRC_EXT_LOSU_INDEX_C

#include <string.h>

#include "losu.h"
#include "losu_mem.h"

static int32_t this_losu_path(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  stack_push(vm, obj_newstr(vm, vm->path));
  return 1;
}

losu_object losu_ext_losu(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "release", obj_newstr(vm, LOSU_RELEASE));
  char* release = losu_mem_malloc(vm, strlen(LOSU_RELEASE) + 1);
  strncpy(release, LOSU_RELEASE, strlen(LOSU_RELEASE) + 1);
  char* tmp = strtok(release, "-");
  obj_unitsetstr(vm, obj, "version", obj_newstr(vm, tmp));
  tmp = strtok(NULL, "-");
  obj_unitsetstr(vm, obj, "arch", obj_newstr(vm, tmp));
  tmp = strtok(NULL, "-");
  obj_unitsetstr(vm, obj, "os", obj_newstr(vm, tmp));
  tmp = strtok(NULL, "-");
  obj_unitsetstr(vm, obj, "libc", obj_newstr(vm, tmp));
  obj_unitsetstr(vm, obj, "path", obj_newfunc(vm, this_losu_path));
  losu_mem_free(vm, release);
  return obj;
}

#endif