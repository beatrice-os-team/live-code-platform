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

#ifndef FILE_SRC_API_STACKAPI_C
#define FILE_SRC_API_STACKAPI_C

#include "losu.h"
#include "losu_vm.h"

/**
 * @brief stack push object to stack
 * @param vm the vm handle
 * @param o object
 */
losu_extern_t void stack_push(losu_vm_t vm, losu_object o) {
  if (vm->top == vm->stackmax)
    vm_error(vm, "stack overflow");
  *(vm->top) = o;
  vm->top++;
}

/**
 * @brief stack pop object from stack
 * @param vm the vm handle
 * @param i index
 */
losu_extern_t void stack_pop(losu_vm_t vm, int32_t i) {
  vm->top -= i;
  if (vm->top < vm->stack)
    vm->top = vm->stack;
}

/**
 * @brief stack call function
 * @param vm the vm handle
 * @param narg number of arguments
 * @param nres number of results
 * @return 0 is ok, 1 is error
 */
losu_extern_t losu_ctype_bool stack_call(losu_vm_t vm,
                                         int32_t narg,
                                         int32_t nres) {
  losu_ctype_bool oldyield = vm->yield;
  vm->yield = 0;

  losu_object_t func = vm->top - (narg + 1);
  __losu_vmsig jmp;
  __losu_vmsig_t oldjmp = vm->errjmp;

  vm->errjmp = &jmp;
  int32_t i = setjmp(jmp);
  switch (i) {
    case losu_signal_done: {
      __losu_vmheap_rawcall(vm, func, nres);
      break;
    }
    case losu_signal_error: {
      vm->top = func;
      break;
    }
    case losu_signal_kill: {
      vm->errjmp = oldjmp;
      losu_sigmsg_throw(vm, losu_signal_kill);
      break;
    }
    default: {
      break;
    }
  }

  vm->errjmp = oldjmp;
  vm->yield = oldyield;
  return i;
}

/**
 * @brief stack call function (RAW)
 * @param vm the vm handle
 * @param narg number of arguments
 * @param nres number of results
 */
losu_extern_t void stack_rawcall(losu_vm_t vm, int32_t narg, int32_t nres) {
  losu_ctype_bool oldyield = vm->yield;
  vm->yield = 0;
  losu_object_t func = vm->top - (narg + 1);
  __losu_vmheap_rawcall(vm, func, nres);
  vm->yield = oldyield;
}

#endif