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

#ifndef FILE_SRC_GC_COLLECT_C
#define FILE_SRC_GC_COLLECT_C

#include "losu.h"
#include "losu_gc.h"
#include "losu_mem.h"
#include "losu_object.h"

#include <stdio.h>

void __losu_gc_cltstring(losu_vm_t vm, losu_ctype_bool all) {
#define sizestring(l) ((sizeof(losu_object_string) + (l) * sizeof(char)))

  for (int32_t i = 0; i < vm->strpool.size; i++) {
    losu_object_string_t* s = &vm->strpool.strobj[i];
    losu_object_string_t next;
    while ((next = *s) != NULL) {
      if (next->marked && !all) {
        if (next->marked < losu_objstring_constmark)
          next->marked = 0;
        s = &next->next;
      } else {
        *s = next->next;
        vm->strpool.nsize--;
        vm->nblocks -=
            ((sizeof(losu_object_string) + (next->len) * sizeof(char)));
        losu_mem_free(vm, next);
      }
    }
  }
  if (vm->strpool.nsize < vm->strpool.size / 4 && vm->strpool.size > 8)
    __losu_objstrpool_resize(vm, &vm->strpool, vm->strpool.size / 2);

#undef sizestring
}

void __losu_gc_cltunitdeinit(losu_vm_t vm) {
  losu_object_hash_t* h = &vm->hashpool;
  losu_object_hash_t next;
  while ((next = *h) != NULL) {
    if (next->mark == next && next->type == obj_unittype_object) {
      losu_object_string_t del = losu_objstring_new(vm, "__del__");
      losu_object_t func = (losu_object_t)losu_objunit_getstr(vm, next, del);
      if (ovtype(func) == losu_object_type_function) {
        stack_push(vm, *func);
        stack_push(vm, (losu_object){
                           .type = losu_object_type_unit,
                           .value.hash = next,
                       });
        stack_rawcall(vm, 1, 0);
        *losu_objunit_setstr(vm, next, del) = __losu_objconst_null;
        h = &vm->hashpool;
        continue;
      }
    }
    h = &next->next;
  }
}

void __losu_gc_cltunit(losu_vm_t vm) {
  losu_object_hash_t* h = &vm->hashpool;
  losu_object_hash_t next;
  while ((next = *h) != NULL) {
    if (next->mark != next) {
      next->mark = next;
      h = &next->next;
    } else {
      *h = next->next;
      // if (next->type == obj_unittype_object) {
      //   losu_object_t func = (losu_object_t)losu_objunit_getstr(
      //       vm, next, losu_objstring_new(vm, "__del__"));
      //   // printf("@%d\n",func->type);
      //   if (ovtype(func) == losu_object_type_function) {
      //     stack_push(vm, *func);
      //     stack_push(vm, (losu_object){
      //                        .type = losu_object_type_unit,
      //                        .value.hash = next,
      //                    });
      //     stack_rawcall(vm, 1, 0);
      //   }
      // }
      losu_objunit_free(vm, next);
    }
  }
}

void __losu_gc_cltscode(losu_vm_t vm) {
  losu_object_scode_t* f = &vm->inspool;
  losu_object_scode_t next;
  while ((next = *f) != NULL) {
    if (next->marked) {
      /* printf ("Gc: %p\n",next); */
      next->marked = 0;
      f = &next->next;
    } else {
      *f = next->next;
      losu_objscode_free(vm, next);
    }
  }
}

void __losu_gc_cltfunc(losu_vm_t vm) {
  losu_object_function_t* f = &vm->funcpool;
  losu_object_function_t next;
  while ((next = *f) != NULL) {
    if (next->mark != next) {
      next->mark = next;
      f = &next->next;
    } else {
      *f = next->next;
      losu_objfunc_free(vm, next);
    }
  }
}

void __losu_gc_cltcontext(losu_vm_t vm) {
  losu_object_context_t f = vm->callpool;
  losu_object_context_t tmp = NULL;
  while (f != NULL) {
    if (f->marked) {
      f->marked = 0;
      f = f->next;
    } else {
      tmp = f;
      f = f->next;
      losu_objcontext_free(vm, tmp);
    }
  }
}

void __losu_gc_cltcoro(losu_vm_t vm, losu_ctype_bool all) {
  losu_object_coroutine_t* f = &vm->coropool;
  losu_object_coroutine_t next;
  while ((next = *f) != NULL) {
    if (next->sta != losu_objcoro_stateEND && !all) {
      f = &next->next;
    } else {
      *f = next->next;
      losu_objcoro_free(vm, next);
    }
  }
}

void __losu_gc_cltbuff(losu_vm_t vm) {
  if (vm->bufftmp) {
    vm->nblocks -= vm->nbufftmp * sizeof(char);
    vm->nbufftmp = 0;
    losu_mem_free(vm, vm->bufftmp);
    vm->bufftmp = NULL;
  }
}

#endif