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

#ifndef FILE_SRC_PORT_LOSU_MAIN_C
#define FILE_SRC_PORT_LOSU_MAIN_C

#include "losu.h"
#include "losu_logo.h"
#include "losu_mem.h"

#include <stdio.h>
#include <string.h>

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stdout, "Losu Package Runner - run losu package with 'main.losu'\n");
    fprintf(stdout, "sdk info: v%s\n", LOSU_RELEASE);
    fprintf(stdout, "usage:\n\tlosu [path of package]\n");
    fprintf(stdout, "%s\n", logo_txt);
  } else {
    // get path
    const char* path_raw = argv[1];
    size_t path_raw_len = strlen(path_raw);

    // set path
    char path[path_raw_len + 2];
    memset(path, 0, path_raw_len + 2);
    memcpy(path, path_raw, path_raw_len);
    if (path[path_raw_len - 1] != '/')
      path[path_raw_len] = '/';
    size_t path_len = strlen(path);

    // set filename
    char filename[path_len + sizeof("main.losu")];
    memset(filename, 0, path_len + sizeof("main.losu"));
    memcpy(filename, path, path_len);
    memcpy(filename + path_len, "main.losu", sizeof("main.losu"));
    losu_vm_t vm = vm_create(1024);
    // path
    vm_setpath(vm, path);
    // args
    int newargc = argc - 2;
    if (newargc > 0) {
      losu_object_t args = losu_mem_malloc(vm, sizeof(losu_object) * newargc);
      for (int i = 0; i < newargc; i++)
        args[i] = obj_newstr(vm, argv[i + 2]);
      vm_setargs(vm, newargc, args);
    } else {
      vm_setargs(vm, 0, NULL);
    }
    // module
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    // dofile
    if (vm_dofile(vm, filename) == 0) {
      losu_ctype_bool b = 1;
      while (b) {
        b = 0;
        for (losu_object_coroutine_t coro = vm->coropool; coro;
             coro = coro->next) {
          if (vm_await(vm, coro) != -1)
            b = 1;
        }
      }
    }
    vm_close(vm);
  }
  return 0;
}

#endif
