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

#ifndef FILE_SRC_OBJECT_OBJFUNC_C
#define FILE_SRC_OBJECT_OBJFUNC_C

#include <string.h>
#include "losu_mem.h"
#include "losu_object.h"

#define sizeObjFunc(n) sizeof(losu_object_function) + sizeof(losu_object) * n;

#define sizeObjScode(f)                                                 \
  ((sizeof(losu_object_scode) + f->nlcnum * sizeof(losu_ctype_number) + \
    f->nlcstr * sizeof(losu_object_string_t) +                          \
    f->nlcscode * sizeof(losu_object_scode_t) +                         \
    f->ncode * sizeof(losu_ctype_vmins_t) +                             \
    f->nlocalvar * sizeof(losu_objscode_locvar) +                       \
    f->nlineinfo * sizeof(int32_t)))

/* function shell */
#if 1
/**
 * @brief create a func object
 * @param vm losu vm handle
 * @param issnum ext size
 * @return object handle
 */
losu_object_function_t losu_objfunc_new(losu_vm_t vm, int32_t issnum) {
  losu_ctype_size_t size = sizeObjFunc(issnum);
  losu_object_function_t f = (losu_object_function_t)losu_mem_malloc(vm, size);
  vm->nblocks += size; /* malloc */
  memset((void*)f, 0, size);
  f->next = vm->funcpool;
  vm->funcpool = f;
  f->mark = f;
  f->nclosure = issnum;
  return f;
}

/**
 * @brief free a func object
 * @param vm losu vm handle
 * @param f func object
 * @note f must be a func object!
 */
void losu_objfunc_free(losu_vm_t vm, losu_object_function_t f) {
  vm->nblocks -= sizeObjFunc(f->nclosure);
  losu_mem_free(vm, f);
}

#endif

/* script func code */
#if 1
/**
 * @brief create script code block
 * @param vm losu vm handle
 * @return code handle
 */
losu_object_scode_t losu_objsocde_new(losu_vm_t vm) {
  losu_object_scode_t f = losu_mem_new(vm, losu_object_scode);
  /* don't need add gc, syntax will do */
  memset((void*)f, 0, sizeof(losu_object_scode));
  f->marked = 0;
  f->next = vm->inspool;
  vm->inspool = f;
  return f;
}

/**
 * @brief free script code
 * @param vm losu vm handle
 * @param f code handle
 */
void losu_objscode_free(losu_vm_t vm, losu_object_scode_t f) {
  if (f->ncode > 0)
    vm->nblocks -= sizeObjScode(f);
  losu_mem_free(vm, f->code);
  losu_mem_free(vm, f->localvar);
  losu_mem_free(vm, f->lcstr);
  losu_mem_free(vm, f->lcnum);
  losu_mem_free(vm, f->lcscode);
  losu_mem_free(vm, f->lineinfo);
  losu_mem_free(vm, f);
}

#endif

#endif