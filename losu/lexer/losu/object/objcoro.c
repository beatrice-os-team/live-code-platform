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

#ifndef FILE_SRC_OBJECT_OBJCORO_C
#define FILE_SRC_OBJECT_OBJCORO_C

#include <stdlib.h>
#include <string.h>

#include "losu.h"
#include "losu_mem.h"
#include "losu_object.h"

/**
 * @brief create new coro, with stack
 * @param vm losu vm handle
 * @param size stack size
 * @return object handle
 */
losu_object_coroutine_t losu_objcoro_new(losu_vm_t vm, int32_t size) {
  losu_object_coroutine_t new_coro = losu_mem_new(vm, losu_object_coroutine);
  memset(new_coro, 0, sizeof(losu_object_coroutine));
  vm->nblocks += sizeof(losu_object_coroutine); /* malloc */
  new_coro->stack = losu_mem_malloc(vm, size * sizeof(losu_object));
  memset(new_coro->stack, 0, size * sizeof(losu_object));
  new_coro->nstack = size;
  vm->nblocks += size * sizeof(losu_object);
  new_coro->sta = losu_objcoro_stateREADY;
  new_coro->next = vm->coropool;
  vm->coropool = new_coro;
  return new_coro;
}

/**
 * @brief free a coro object
 * @param vm losu vm handle
 * @param ct coro handle
 */
void losu_objcoro_free(losu_vm_t vm, losu_object_coroutine_t co) {
  vm->nblocks -= sizeof(losu_object_coroutine);
  vm->nblocks -= co->nstack * sizeof(losu_object);
  losu_mem_free(vm, co->stack);
  losu_mem_free(vm, co); /* free */
}

#endif