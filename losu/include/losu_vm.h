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
#ifndef FILE_SRC_LOSU_VM_H
#define FILE_SRC_LOSU_VM_H
#include "losu.h"

/**
 * signal system
 */
#if 1
/* losu_extern_t losu_sigmsg_t losu_sigmsg_create(losu_sigmsg_t msg[]);

losu_extern_t losu_ctype_bool losu_sigmsg_check(losu_sigmsg_t signal,
                                                losu_sigmsg_t check); */

losu_extern_t void losu_sigmsg_throw(struct losu_vm* vm, losu_signal_t s);
#endif

/**
 * vm heap
 */
#if 1
void __losu_vmheap_init(losu_vm_t vm, int32_t size);

void __losu_vmheap_check(losu_vm_t vm, int32_t size);

void __losu_vmheap_adjtop(losu_vm_t vm, losu_object_t base, int32_t size);

void __losu_vmheap_rawcall(losu_vm_t vm, losu_object_t func, int32_t nres);

void __losu_vmheap_procall(losu_vm_t vm, losu_object_t func, int32_t nres);

#endif

/**
 * vm core:
 *      execution engine
 */
#if 1
losu_object_t __losu_vmcore_exec(losu_vm_t vm,
                                 losu_object_context_t cinfo,
                                 losu_object_t recall);

losu_object_t __losu_vmcore_ccall(losu_vm_t vm, losu_object_context_t cinfo);

#endif

/**
 * vm data
 */
#if 1
losu_ctype_bool __losu_vmdata_tonum(losu_vm_t* vm, losu_object_t obj);
losu_ctype_bool __losu_vmdata_tostring(losu_vm_t* vm, losu_object_t obj);

#endif

#endif