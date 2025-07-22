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

#ifndef FILE_SRC_GC_MARK_C
#define FILE_SRC_GC_MARK_C
#include "mark.h"
#include <stdio.h>
#include "losu_gc.h"
#include "losu_object.h"

/**
 * @brief do 'mark'
 * @param vm losu vm
 */
void __losu_gc_markwork(losu_vm_t vm) {
  losu_gc gc;
  gc.func = NULL;
  gc.unit = vm->global;
  vm->global->mark = NULL;
  __losu_gc_markstack(vm, &gc);
  while (1) {
    if (gc.func) {
      losu_object_function_t f = gc.func;
      gc.func = f->mark;
      for (int32_t i = 0; i < f->nclosure; i++)
        __losu_gc_markobj(&gc, &f->closure[i]);
    } else if (gc.unit) {
      losu_object_hash_t h = gc.unit;
      gc.unit = h->mark;
      for (int32_t i = 0; i < h->size; i++) {
        losu_hash_node_t n = &h->node[i];
        if (ovtype((&n->key)) != losu_object_type_false) {
          __losu_gc_markobj(&gc, &n->key);
          __losu_gc_markobj(&gc, &n->value);
        }
      }
    } else
      break;
  }
}

void __losu_gc_markstack(losu_vm_t vm, losu_gc_t gc) {
  losu_object_t o = NULL;

  if (vm->stack == vm->mstack)
    vm->mtop = vm->top;
  for (o = vm->mstack; o < vm->mtop; o++)
    __losu_gc_markobj(gc, o);
  // printf("@@@@ %p-%p,%p-%p\n", vm->stack, vm->top, vm->mstack, vm->mtop);
  for (o = vm->stack; o < vm->top; o++)
    __losu_gc_markobj(gc, o);
  /* mark coropool */
  losu_object_coroutine_t coro = NULL;
  for (coro = vm->coropool; coro; coro = coro->next) {
    if (coro->sta != losu_objcoro_stateEND && coro->stack != vm->stack) {
      for (o = coro->stack; o < coro->top; o++)
        __losu_gc_markobj(gc, o);
    }
  }
  // printf("mark end\n");
}

void __losu_gc_markobj(losu_gc_t gc, losu_object_t o) {
  switch (ovtype(o)) {
    case losu_object_type_string: {
      ovIstr(o)->marked = 1;
      break;
    }
    case losu_object_type_function: {
      __losu_gc_markfunc(gc, ovfunc(o));
      break;
    }
    case losu_object_type_unit: {
      if (ovhash(o)->mark == ovhash(o)) {
        ovhash(o)->mark = gc->unit;
        gc->unit = ovhash(o);
      }
      break;
    }
    case losu_object_type_context: {
      ovcontext(o)->marked = 1;
      if (ovcontext(o)->func)
        __losu_gc_markfunc(gc, ovcontext(o)->func);
      if (ovtype(&(ovcontext(o)->assert)) == losu_object_type_function)
        __losu_gc_markobj(gc, &(ovcontext(o)->assert));
      break;
    }
    default:
      break;
  }
}

void __losu_gc_markfunc(losu_gc_t gc, losu_object_function_t f) {
  if (f->mark == f) {
    if (!f->isC)
      __losu_gc_markscode(gc, f->func.sdef);
    f->mark = gc->func;
    gc->func = f;
  }
}

void __losu_gc_markscode(losu_gc_t gc, losu_object_scode_t f) {
  if (!f->marked) {
    f->marked = 1;
    /* mark src (if exsist) */
    /* if (f->src)
      f->src->marked = 1; */
    /* mark local string */
    for (int32_t i = 0; i < f->nlcstr; i++)
      f->lcstr[i]->marked = 1;
    /* mark local scode */
    for (int32_t i = 0; i < f->nlcscode; i++)
      __losu_gc_markscode(gc, f->lcscode[i]);
    /* mark locval */
    for (int32_t i = 0; i < f->nlocalvar; i++)
      f->localvar[i].name->marked = 1;
  }
}

#endif