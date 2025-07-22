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

#ifndef FILE_SRC_API_ARGAPI_C
#define FILE_SRC_API_ARGAPI_C
#include "losu.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "losu_vm.h"

#include <stdarg.h>

/**
 * @brief create a arg list in heap
 * @param vm the losu vm handle
 * @param size size of arg list
 * @return the arg list handle
 */
losu_extern_t losu_vm_vaarg_t arg_start(losu_vm_t vm, int32_t size) {
  losu_vm_vaarg_t valist = losu_mem_new(vm, losu_vm_vaarg);
  valist->argc = 0;
  valist->size = size;
  valist->vm = vm;
  valist->argv = losu_mem_newvector(vm, valist->size, losu_object);
  return valist;
}

/**
 * @brief add a arg to arg list
 * @param valist the arg list handle
 * @param type the type of arg
 * @param ... the value of arg
 * @return 0 is ok, 1 is error
 */
losu_extern_t losu_ctype_bool arg_add(losu_vm_vaarg_t valist,
                                      int32_t type,
                                      ...) {
  va_list ap;
  va_start(ap, type);
  if (valist->argc >= valist->size)
    return 1;
  switch (type) {
    case losu_object_type_false: {
      valist->argv[valist->argc++] = __losu_objconst_null;
      break;
    }
    case losu_object_type_true: {
      valist->argv[valist->argc++] = __losu_objconst_true;
      break;
    }
    case losu_object_type_number: {
      valist->argv[valist->argc++] = (losu_object){
          .type = losu_object_type_number,
          .value.num = va_arg(ap, losu_ctype_number),
      };
      break;
    }
    case losu_object_type_string: {
      valist->argv[valist->argc++] = (losu_object){
          .type = losu_object_type_string,
          .value.str = losu_objstring_new(valist->vm, va_arg(ap, const char*)),
      };
      break;
    }
    case losu_object_type_function: {
      losu_object_function_t f = losu_objfunc_new(valist->vm, 0);
      f->isC = 1;
      f->func.capi = va_arg(ap, losu_vmapi_t);
      valist->argv[valist->argc++] = (losu_object){
          .type = losu_object_type_function,
          .value.func = f,
      };
      break;
    }
    case losu_object_type_pointer: {
      valist->argv[valist->argc++] = (losu_object){
          .type = losu_object_type_pointer,
          .value.ptr = va_arg(ap, void*),
      };
      break;
    }
    default: {
      valist->argv[valist->argc++] = va_arg(ap, losu_object);
      break;
    }
  }
  va_end(ap);
  return 0;
}

/**
 * @brief delete a arg list in heap
 * @param valist the arg list handle
 * @return 0 is ok, 1 is error
 */
losu_extern_t losu_ctype_bool arg_del(losu_vm_vaarg_t valist, int32_t n) {
  if (valist->argc < n) {
    valist->argc = 0;
    return 1;
  }
  valist->argc -= n;
  return 0;
}

/**
 * @brief move arg list to stack, and delete the arg list
 * @param valist the arg list handle
 * @return variable number
 */
losu_extern_t int32_t arg_tostack(losu_vm_vaarg_t valist) {
  int32_t n = valist->argc;
  __losu_vmheap_check(valist->vm, n);
  for (int32_t i = 0; i < n; i++)
    *(valist->vm->top++) = valist->argv[i];
  losu_mem_free(valist->vm, valist->argv);
  losu_mem_free(valist->vm, valist);
  return n;
}

#endif