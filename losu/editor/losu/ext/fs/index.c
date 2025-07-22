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

#ifndef FILE_SRC_EXT_FS_INDEX_C
#define FILE_SRC_EXT_FS_INDEX_C

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu.h"
#include "losu_mem.h"

/*
class fs:
    def read(path):
        pass
    def write(path, content):
        pass
    def append(path, content):
        pass
    def rename(path, newname):
        pass
    def remove(path):
        pass
*/

static int32_t this_read(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "fs::read(): wrong number of arguments");
    return 0;
  }
  FILE* fp = fopen(obj_tostr(vm, &argv[0]), "rb");
  if (!fp) {
    vm_error(vm, "fs::read(): %s", strerror(errno));
    return 0;
  }
  // 读取完整的文件
  fseek(fp, 0, SEEK_END);
  uint64_t flen = ftell(fp);
  if (flen >= UINT32_MAX)
    vm_error(vm, "fs::read(): file too large");
  fseek(fp, 0, SEEK_SET);
  char* tmp = losu_mem_malloc(vm, flen);
  fread(tmp, 1, flen, fp);
  if (ferror(fp)) {
    fclose(fp);
    vm_error(vm, "fs::read(): %s", strerror(errno));
    return 0;
  }
  stack_push(vm, obj_newstrlen(vm, tmp, flen));
  fclose(fp);
  return 1;
}

static int32_t this_write(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "fs::write(): wrong number of arguments");
    return 0;
  }
  FILE* fp = fopen(obj_tostr(vm, &argv[0]), "wb");
  if (!fp) {
    vm_error(vm, "fs::write(): %s", strerror(errno));
    return 0;
  }
  fwrite(obj_tostr(vm, &argv[1]), obj_getstrlen(vm, &argv[1]), 1, fp);
  if (ferror(fp)) {
    fclose(fp);
    vm_error(vm, "fs::write(): %s", strerror(errno));
    return 0;
  }
  fclose(fp);
  stack_push(vm, obj_newtrue(vm));
  return 1;
}

static int32_t this_append(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "fs::append(): wrong number of arguments");
    return 0;
  }
  FILE* fp = fopen(obj_tostr(vm, &argv[0]), "ab");
  if (!fp) {
    vm_error(vm, "fs::append(): %s", strerror(errno));
    return 0;
  }
  fwrite(obj_tostr(vm, &argv[1]), obj_getstrlen(vm, &argv[1]), 1, fp);
  if (ferror(fp)) {
    fclose(fp);
    vm_error(vm, "fs::append(): %s", strerror(errno));
    return 0;
  }
  fclose(fp);
  stack_push(vm, obj_newtrue(vm));
  return 1;
}

static int32_t this_rename(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 2) {
    vm_error(vm, "fs::rename(): wrong number of argument");
    return 0;
  }
  rename(obj_tostr(vm, &argv[0]), obj_tostr(vm, &argv[1]));
  if (errno) {
    vm_error(vm, "fs::rename(): %s", strerror(errno));
    return 0;
  }
  stack_push(vm, obj_newtrue(vm));
  return 1;
}
static int32_t this_remove(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (!argc) {
    vm_error(vm, "fs::remove(): wrong number of argument");
    return 0;
  }
  remove(obj_tostr(vm, &argv[0]));
  if (errno) {
    vm_error(vm, "fs::remove(): %s", strerror(errno));
    return 0;
  }
  stack_push(vm, obj_newtrue(vm));
  return 1;
}

losu_object losu_ext_fs(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "read", obj_newfunc(vm, this_read));
  obj_unitsetstr(vm, obj, "write", obj_newfunc(vm, this_write));
  obj_unitsetstr(vm, obj, "append", obj_newfunc(vm, this_append));
  obj_unitsetstr(vm, obj, "rename", obj_newfunc(vm, this_rename));
  obj_unitsetstr(vm, obj, "remove", obj_newfunc(vm, this_remove));
  return obj;
}

#endif