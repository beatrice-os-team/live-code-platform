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

    THE SOFTWARE IS PROlosu_syntax_exptype_indexDED “AS IS”, WITHOUT WARRANTY OF
  ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
  EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FILE_SRC_VM_ALU_C
#define FILE_SRC_VM_ALU_C

#include "alu.h"
#include "losu_object.h"

static losu_object __obj_new(losu_vm_t vm,
                             losu_object obj,
                             int32_t argc,
                             losu_object argv[]) {
  if (obj.type != losu_object_type_unit)
    return obj_newnull(vm);
  losu_object newobj;
  // check obj.__super__
  losu_object surper = *obj_unitindexstr(vm, obj, "__super__");
  losu_object_t init = obj_unitindexstr(vm, obj, "__init__");
  if (surper.type == losu_object_type_unit)
    newobj = __obj_new(vm, surper, argc, argv);
  else
    newobj = obj_newunit(vm, obj_unittype_class);
  for (losu_hash_node_t node = obj_unititer(vm, obj, NULL); node;
       node = obj_unititer(vm, obj, node)) {
    obj_unitset(vm, newobj, node->key, node->value);
  }
  if (init->type == losu_object_type_function) {
    stack_push(vm, *init);
    stack_push(vm, newobj);
    for (int32_t i = 1; i < argc; i++)
      stack_push(vm, argv[i]);
    stack_rawcall(vm, argc, 0);
  }
  return newobj;
}
losu_extern_t void losu_vm_alu_call(losu_vm_t vm,
                                    losu_object_t obj,
                                    void (*inlinecall)(struct losu_vm*,
                                                       struct losu_object*,
                                                       int32_t),
                                    int16_t nres) {
  if (nres == -1)
    nres = 1;
  losu_object_t func = NULL;
  if (ovhash(obj)->type == obj_unittype_object) {
    func = losu_objunit_getstr(vm, ovhash(obj),
                               losu_objstring_new(vm, "__call__"));
    if (ovtype(func) != losu_object_type_function)
      vm_error(vm, "invalid overload operator __call__");
    vm->top++;
    for (losu_object_t i = vm->top - 1; i != obj; i--)
      *i = *(i - 1);
    *obj = *func;
    (*inlinecall)(vm, obj, nres);
  } else if (ovhash(obj)->type == obj_unittype_class) {
    int argc = vm->top - obj;
    losu_object newobj = __obj_new(vm, *obj, argc, obj);
    ovhash(&newobj)->type = obj_unittype_object;
    obj_unitsetstr(vm, newobj, "__super__", obj_newnull(vm));
    obj_unitsetstr(vm, newobj, "__init__", obj_newnull(vm));
    obj_unitsetstr(vm, newobj, "__reflect__", *obj);

    *obj = newobj;
    losu_object_t fres = obj;
    vm->top = obj + nres;
    if (vm->stackmax - vm->top <= 1)
      vm_error(vm, "stack overflow");
    for (losu_object_t i = obj + 1; i < vm->top; i++)
      ovtype(i) = losu_object_type_false;
  } else
    vm_error(vm, "unsupported overload operator () of '%s'",
             obj_typestr(vm, obj));
}

losu_extern_t void losu_vm_alu_add(losu_vm_t vm,
                                   losu_object_t self,
                                   losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__add__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator +");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}

losu_extern_t void losu_vm_alu_sub(losu_vm_t vm,
                                   losu_object_t self,
                                   losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__sub__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator -");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}

losu_extern_t void losu_vm_alu_mult(losu_vm_t vm,
                                    losu_object_t self,
                                    losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__mult__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator *");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}

losu_extern_t void losu_vm_alu_div(losu_vm_t vm,
                                   losu_object_t self,
                                   losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__div__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator /");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}

losu_extern_t void losu_vm_alu_pow(losu_vm_t vm,
                                   losu_object_t self,
                                   losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__pow__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator **");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}

losu_extern_t void losu_vm_alu_mod(losu_vm_t vm,
                                   losu_object_t self,
                                   losu_object_t other) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__mod__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator %%");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}
losu_extern_t void losu_vm_alu_concat(losu_vm_t vm,
                                      losu_object_t self,
                                      losu_object_t other) {
  losu_object_t func = losu_objunit_getstr(
      vm, ovhash(self), losu_objstring_new(vm, "__concat__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator &");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_push(vm, *other);
  stack_rawcall(vm, 2, 1);
}
losu_extern_t void losu_vm_alu_neg(losu_vm_t vm, losu_object_t self) {
  losu_object_t func =
      losu_objunit_getstr(vm, ovhash(self), losu_objstring_new(vm, "__neg__"));
  if (ovtype(func) != losu_object_type_function)
    vm_error(vm, "invalid overload operator ~");
  stack_push(vm, *func);
  stack_push(vm, *self);
  stack_rawcall(vm, 1, 1);
}
#endif