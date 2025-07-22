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

#ifndef FILE_SRC_VM_HEAP_C
#define FILE_SRC_VM_HEAP_C

#include "losu_mem.h"
#include "losu_object.h"
#include "losu_vm.h"

void __losu_vmheap_init(losu_vm_t vm, int32_t size) {
  vm->top = vm->stack = vm->mtop = vm->mstack =
      losu_mem_newvector(vm, size, losu_object);
  vm->nblocks += size * sizeof(losu_object);
  vm->stackmax = vm->stack + size - 1;
}

void __losu_vmheap_check(losu_vm_t vm, int32_t size) {
  if (vm->stackmax - vm->top <= size)
    vm_error(vm, "stack overflow");
}

void __losu_vmheap_adjtop(losu_vm_t vm, losu_object_t base, int32_t size) {
  int32_t diff = size - (vm->top - base);
  if (diff <= 0)
    vm->top = base + size;
  else {
    __losu_vmheap_check(vm, diff);
    while (diff--)
      ovtype((vm->top++)) = losu_object_type_false;
  }
}

void __losu_vmheap_rawcall(losu_vm_t vm, losu_object_t func, int32_t nres) {
  losu_object_context cinfo = {NULL};
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "can't call '%s'", obj_typestr(vm, func));
  cinfo.nres = nres;
  cinfo.func = ovfunc(func);
  cinfo.inlinecall = (void (*)(losu_vm_t vm, losu_object_t func,
                               int32_t nres))__losu_vmheap_rawcall;
  cinfo.base = func + 1;
  ovtype(func) = losu_object_type_context;
  ovcall(func) = &cinfo;
  losu_object_t fres = (cinfo.func->isC) ? __losu_vmcore_ccall(vm, &cinfo)
                                         : __losu_vmcore_exec(vm, &cinfo, NULL);
  if (nres == -1) {
    while (fres < vm->top)
      *func++ = *fres++;
    vm->top = func;
  } else {
    for (; nres > 0 && fres < vm->top; nres--)
      *func++ = *fres++;
    vm->top = func;
    for (; nres > 0; nres--)
      ovtype((vm->top++)) = losu_object_type_false;
  }
}

void __losu_vmheap_procall(losu_vm_t vm, losu_object_t func, int32_t nres) {
  losu_object_context_t cinfo = losu_objcontext_new(vm);
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "can't call '%s'", obj_typestr(vm, func));
  cinfo->nres = nres;
  cinfo->func = ovfunc(func);
  cinfo->inlinecall = (void (*)(losu_vm_t vm, losu_object_t func,
                                int32_t nres))__losu_vmheap_procall;
  cinfo->base = func + 1;
  ovtype(func) = losu_object_type_context;
  ovcall(func) = cinfo;
  losu_object_t fres = (cinfo->func->isC) ? __losu_vmcore_ccall(vm, cinfo)
                                          : __losu_vmcore_exec(vm, cinfo, NULL);
  if (nres == -1) {
    while (fres < vm->top)
      *func++ = *fres++;
    vm->top = func;
  } else {
    for (; nres > 0 && fres < vm->top; nres--)
      *func++ = *fres++;
    vm->top = func;
    for (; nres > 0; nres--)
      ovtype((vm->top++)) = losu_object_type_false;
  }
  losu_objcontext_free(vm, cinfo);
}

#endif