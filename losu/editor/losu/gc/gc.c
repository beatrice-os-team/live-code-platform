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

#ifndef FILE_SRC_GC_GC_C
#define FILE_SRC_GC_GC_C

#include "collect.h"
#include "losu_gc.h"
#include "losu_object.h"
#include "mark.h"

/**
 * @brief create a 'GC' task
 * @param vm losu vm handle
 * @return 1 = task done, 0 = task cancel
 */
losu_ctype_bool losu_gc_newtask(losu_vm_t vm) {
  /* check */
  if (vm->nblocks > vm->gcHook)
    vm->gcHook = vm->nblocks;
  if (vm->nblocks >= vm->gcDymax && vm->gc == 0) {
    vm->gc = 1;
    /* mark-clean GC */
    __losu_gc_markwork(vm);
    losu_gc_clean(vm, 0);
    vm->gcDymax *= 2;
    if (vm->gcDymax > vm->gcMax)
      vm->gcDymax = vm->gcMax;
    vm->gc = 0;
    return 1;
  }
  return 0;
}

/**
 * @brief do clean
 * @param vm losu vm handle
 * @param all clean 'ALL'? 1 = yes, 0 = no
 */
void losu_gc_clean(losu_vm_t vm, losu_ctype_bool all) {
  __losu_gc_cltunitdeinit(vm);
  __losu_gc_cltunit(vm);
  __losu_gc_cltstring(vm, all);
  __losu_gc_cltscode(vm);
  __losu_gc_cltfunc(vm);
  __losu_gc_cltcontext(vm);
  __losu_gc_cltcoro(vm, all);
  __losu_gc_cltbuff(vm);
}

#endif