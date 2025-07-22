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

#ifndef FILE_SRC_LOSU_OBJECT_H
#define FILE_SRC_LOSU_OBJECT_H
#include <stdlib.h>
#include "losu.h"

/**
 * Object segment
 *      const
 *      function
 *      index
 * */
#if 1

/* static object */
losu_extern_t const losu_object __losu_objconst_null;    /* False */
losu_extern_t const losu_object __losu_objconst_true;    /* True */



/* function */
losu_ctype_bool losu_object_iseq(losu_vm_t vm,
                                 const losu_object_t t1,
                                 const losu_object_t t2);

losu_ctype_bool losu_object_less(losu_vm_t vm,
                                 const losu_object_t t1,
                                 const losu_object_t t2);

losu_ctype_bool losu_object_str2num(losu_vm_t vm,
                                    const char* s,
                                    losu_ctype_number* n);

/* index */
// losu_object_t o;
#define ovtype(o) ((o)->type)
#define ovnumber(o) ((o)->value.num)
#define ovIstr(o) ((o)->value.str)
#define ovSstr(o) ((o)->value.str->str)
#define ovfunc(o) ((o)->value.func)
#define ovhash(o) ((o)->value.hash)
#define ovcall(o) ((o)->value.context)
#define ovcontext(o) ((o)->value.context)
#define ovcoro(o) ((o)->value.coro)
#define ovptr(o) ((o)->value.ptr)

#endif

/**
 * Object 'string'
 */
#if 1
/* string */
losu_object_string_t losu_objstring_new(losu_vm_t vm, const char* s);

losu_object_string_t losu_objstring_newlen(losu_vm_t vm,
                                           const char* s,
                                           losu_ctype_size_t len);
losu_object_string_t losu_objstring_newconst(losu_vm_t vm, const char* s);

/* string pool */
void __losu_objstrpool_init(losu_vm_t vm);

void __losu_objstrpool_resize(losu_vm_t vm,
                              __losu_vm_strpool_t strpool,
                              losu_ctype_hash_t s);

void __losu_objstrpool_deinit(losu_vm_t vm);

#ifndef losu_objstring_constmark
#define losu_objstring_constmark 2
#endif

#endif

/**
 * Object 'function'
 *      with: 'scode'
 */
#if 1
/* function shell */
losu_object_function_t losu_objfunc_new(losu_vm_t vm, int32_t issnum);

void losu_objfunc_free(losu_vm_t vm, losu_object_function_t f);

/* script func code */
losu_object_scode_t losu_objsocde_new(losu_vm_t vm);

void losu_objscode_free(losu_vm_t vm, losu_object_scode_t f);

#endif

/**
 * Object 'unit'
 */
#if 1
/* unit: new & del */
losu_object_hash_t losu_objunit_new(losu_vm_t vm, int32_t size);

void losu_objunit_remove(losu_vm_t vm, losu_object_t key);

void losu_objunit_free(losu_vm_t vm, losu_object_hash_t t);

/* unit: set */
losu_object_t losu_objunit_set(losu_vm_t vm,
                               losu_object_hash_t t,
                               losu_object_t key);

losu_object_t losu_objunit_setnum(losu_vm_t vm,
                                  losu_object_hash_t t,
                                  losu_ctype_number key);

losu_object_t losu_objunit_setstr(losu_vm_t vm,
                                  losu_object_hash_t t,
                                  losu_object_string_t key);

/* unit: get */
const losu_object_t losu_objunit_get(losu_vm_t vm,
                                     losu_object_hash_t t,
                                     losu_object_t key);

const losu_object_t losu_objunit_getany(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_object_t key);

const losu_object_t losu_objunit_getnum(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_ctype_number key);

const losu_object_t losu_objunit_getstr(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_object_string_t key);

const losu_object_t losu_objunit_getglobal(losu_vm_t vm, const char* name);

const losu_hash_node_t losu_objunit_getnext(losu_vm_t vm,
                                            losu_object_hash_t t,
                                            losu_object_t key);

#endif

/**
 * Object 'context'
 */
#if 1
losu_object_context_t losu_objcontext_new(losu_vm_t vm);

void losu_objcontext_free(losu_vm_t vm, losu_object_context_t ct);

#endif

/**
 * Object ' coroutine'
 */
#if 1
losu_object_coroutine_t losu_objcoro_new(losu_vm_t vm, int32_t size);

void losu_objcoro_free(losu_vm_t vm, losu_object_coroutine_t co);

#define losu_objcoro_stateREADY ((losu_objcoro_state_t)0)
#define losu_objcoro_stateRUN ((losu_objcoro_state_t)1)
#define losu_objcoro_stateYIELD ((losu_objcoro_state_t)2)
#define losu_objcoro_stateEND ((losu_objcoro_state_t)3)

#endif

#endif
