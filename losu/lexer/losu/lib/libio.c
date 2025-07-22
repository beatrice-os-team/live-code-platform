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

#ifndef FILE_SRC_LIB_LIBIO_C
#define FILE_SRC_LIB_LIBIO_C

#include "libio.h"
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize the io library
 * @param vm the losu vm handle
 */
losu_extern_t void losu_lib_init_io(losu_vm_t vm) {
  vm_setval(vm, "print", obj_newfunc(vm, losu_lib_io_print));
  vm_setval(vm, "input", obj_newfunc(vm, losu_lib_io_input));
  vm_setval(vm, "exit", obj_newfunc(vm, losu_lib_io_exit));
}

static int32_t losu_lib_io_print(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  for (int32_t i = 0; i < argc; i++)
    fprintf(stdout, "%s", obj_tostr(vm, &argv[i]));
  fprintf(stdout, "\n");
  stack_push(vm, argc ? obj_newtrue(vm) : obj_newnull(vm));
  return 1;
}

static int32_t losu_lib_io_input(losu_vm_t vm,
                                 int32_t argc,
                                 losu_object argv[]) {
  const char* mode = argc ? obj_tostr(vm, &argv[0]) : "string";
  char buff[L_tmpnam];
  losu_ctype_number n = 0;
  if (!strncmp(mode, "string", 6)) {
    stack_push(
        vm, obj_newstr(vm, emscripten_run_script_string("prompt('请输入')")));
  } else if (!strncmp(mode, "number", 6)) {
    stack_push(
        vm,
        obj_newnum(vm, atof(emscripten_run_script_string("prompt('请输入')"))));
  } else if (!strncmp(mode, "line", 4)) {
    // 获取一行
    buff[strlen(buff) - 1] = '\0';
    stack_push(
        vm, obj_newstr(vm, emscripten_run_script_string("prompt('请输入')")));
  }
  return 1;
}

static int32_t losu_lib_io_exit(losu_vm_t vm,
                                int32_t argc,
                                losu_object argv[]) {
  exit(argc ? obj_tonum(vm, &argv[0]) : 0);
  return 0;
}

#endif