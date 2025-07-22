/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like "copyright" restrictions. Losu qualifies
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
#ifndef FILE_SRC_MEM_MEM_C
#define FILE_SRC_MEM_MEM_C
#include "losu_mem.h"

void* losu_mem_realloc(losu_vm_t vm, void* old, losu_ctype_size_t size) {
  void* new = NULL;
  if (size == 0) {
    if (old != NULL)
      free(old);
    return NULL;
  }
  new = (void*)(realloc(old, size));
  if (new == NULL) {
    if (vm)
      vm_error(vm, "Unable to allocate memory. from %p size: %zu B", old,
               (size_t)size);
  }
  return new;
}

void* losu_mem_grow(losu_vm_t vm,
                    void* block,
                    losu_ctype_size_t i_num,
                    int32_t inc,
                    losu_ctype_size_t size,
                    losu_ctype_size_t limit,
                    const char* errormsg) {
  if (i_num >= limit - inc)
    vm_error(vm, errormsg);
  return losu_mem_realloc(vm, block, size * (i_num + inc));
}

#endif