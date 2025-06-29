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

#ifndef FILE_SRC_OBJECT_OBJCONTEXT_C
#define FILE_SRC_OBJECT_OBJCONTEXT_C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu_mem.h"
#include "losu_object.h"

/**
 * @brief create new context(dynamic, for coro)
 * @param vm losu vm handle
 * @return object handle
 */
losu_object_context_t losu_objcontext_new(losu_vm_t vm) {
  losu_object_context_t new_context = losu_mem_new(vm, losu_object_context);
  vm->nblocks += sizeof(losu_object_context); /* malloc */
  memset(new_context, 0, sizeof(losu_object_context));
  new_context->next = vm->callpool;
  new_context->pre = NULL;
  if (vm->callpool)
    vm->callpool->pre = new_context;
  vm->callpool = new_context;
  return new_context;
}

/**
 * @brief free a context
 * @param vm losu vm handle
 * @param ct context handle
 */
void losu_objcontext_free(losu_vm_t vm, losu_object_context_t ct) {
  if (ct->pre)
    ct->pre->next = ct->next;
  if (ct->next)
    ct->next->pre = ct->pre;
  if (ct == vm->callpool)
    vm->callpool = ct->next;
  vm->nblocks -= sizeof(losu_object_context);
  losu_mem_free(vm, ct); /* free */
}

#endif