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
#ifndef FILE_SRC_VM_CORE_C
#define FILE_SRC_VM_CORE_C

#include "core.h"
#include "alu.h"
#include "losu_bytecode.h"
#include "losu_gc.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "losu_vm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

losu_object_t __losu_vmcore_ccall(losu_vm_t vm, losu_object_context_t cinfo) {
  int32_t n = (*cinfo->func->func.capi)(vm, vm->top - cinfo->base, cinfo->base);
  return vm->top - n;
}
losu_object_t __losu_vmcore_exec(losu_vm_t vm,
                                 losu_object_context_t cinfo,
                                 losu_object_t recall) {
#ifndef dojmp
#define dojmp(pc, i)            \
  {                             \
    (pc) += _losu_vmcg_getS(i); \
  }
#endif

  cinfo = recall ? ovcall(recall) : cinfo;
  losu_object_scode_t fcode = cinfo->func->func.sdef;
  losu_object_string_t* lcstr = fcode->lcstr;
  losu_ctype_number* lcnum = fcode->lcnum;
  losu_object_t closure = cinfo->func->closure;
  losu_object_scode_t* lcfcode = fcode->lcscode;
  losu_object_t top;
  losu_object_t base = cinfo->base;
  losu_ctype_vmins_t* pc;

  if (recall) {
    losu_object_t next = ovcall(recall)->nextobj;
    if (next) {
      int32_t nres = ovcall(next)->nres;
      // printf("recall %p\n", next);
      losu_object_t fres = __losu_vmcore_exec(vm, NULL, next);
      if (nres == -1) {
        while (fres < vm->top)
          *next++ = *fres++;
        vm->top = next;
      } else {
        for (; nres > 0 && fres < vm->top; nres--)
          *next++ = *fres++;
        vm->top = next;
        for (; nres > 0; nres--)
          ovtype((vm->top++)) = losu_object_type_false;
      }
    }

  } else {
    __losu_vmheap_check(vm, fcode->maxstacksize);
    if (fcode->isVarg)
      __losu_vmcore_adjvarg(vm, base, fcode->narg);
    else
      __losu_vmheap_adjtop(vm, base, fcode->narg);
    cinfo->pc = fcode->code;
  }
  pc = cinfo->pc;
  cinfo->pc_t = &pc;
  top = vm->top;
  // int tmp = 0;
  while (1) {
    losu_ctype_vmins_t i = *(pc++);
    // printf("[%d]Op:%d\n",tmp++, _losu_vmcg_getOP(i));
    switch (_losu_vmcg_getOP(i)) {
      case INS_END: {
        vm->top = top;
        return top;
      }
      case INS_RETURN: {
        vm->top = top;
        return base + _losu_vmcg_getU(i);
      }
      case INS_CALL: {
        vm->top = top;  // update top
        cinfo->pc = pc;
        int16_t nres = _losu_vmcg_getB(i);
        if (nres == 255)
          nres = -1;
        losu_object_t func = base + _losu_vmcg_getA(i);
        if (ovtype(func) == losu_object_type_unit)
          losu_vm_alu_call(vm, func, cinfo->inlinecall, nres);
        else
          (*cinfo->inlinecall)(vm, func, nres);
        top = vm->top;
        losu_gc_newtask(vm);
        break;
      }
      case INS_PUSHNULL: {
        _losu_vmins_U n = _losu_vmcg_getU(i);
        do {
          ovtype((top++)) = losu_object_type_false;
        } while (--n > 0);
        break;
      }
      case INS_PUSHTRUE: {
        _losu_vmins_U n = _losu_vmcg_getU(i);
        do {
          ovtype((top++)) = losu_object_type_true;
        } while (--n > 0);
        break;
      }
      case INS_POP: {
        top -= _losu_vmcg_getU(i);
        break;
      }
      case INS_PUSHSTRING: {
        ovtype(top) = losu_object_type_string;
        ovIstr(top) = (losu_object_string_t)lcstr[_losu_vmcg_getU(i)];
        top++;
        break;
      }
      case INS_PUSHNUM: {
        ovtype(top) = losu_object_type_number;
        ovnumber(top) = lcnum[_losu_vmcg_getU(i)];
        top++;
        break;
      }
      case INS_PUSHUPVALUE: {
        *top++ = closure[_losu_vmcg_getU(i)];
        break;
      }
      case INS_GETLOCAL: {
        *top++ = *(base + _losu_vmcg_getU(i));
        break;
      }
      case INS_GETGLOBAL: {
        vm->top = top;
        *top = *losu_objunit_getstr(
            vm, vm->global, (losu_object_string_t)lcstr[_losu_vmcg_getU(i)]);
        top++;
        break;
      }
      case INS_GETUNIT: {
        vm->top = top;  // top-2 = get(top-2,vm.top-1)
        if (ovtype(top - 2) == losu_object_type_string) {
          losu_object_t str = top - 2;
          size_t idx = obj_tonum(vm, top - 1);
          if (idx >= ovIstr(str)->len)
            *(top - 2) = __losu_objconst_null;
          else
            *(top - 2) = (losu_object){
                .type = losu_object_type_number,
                .value.num = (losu_ctype_number)((uint8_t)(ovSstr(str)[idx])),
            };
          top--;
          break;
        }
        if (ovtype(top - 2) != losu_object_type_unit)
          vm_error(vm, "can't read properties of '%s'",
                   obj_typestr(vm, top - 2));
        *(top - 2) = *losu_objunit_get(vm, ovhash(top - 2), vm->top - 1);
        top--;
        break;
      }
      case INS_PUSHSELF: {
        losu_object tmp;
        tmp = *(top - 1);
        ovtype(top) = losu_object_type_string;
        ovIstr((top++)) = (losu_object_string_t)lcstr[_losu_vmcg_getU(i)];
        vm->top = top;
        if (ovtype(top - 2) == losu_object_type_unit)
          *(top - 2) = *losu_objunit_get(vm, ovhash(top - 2), vm->top - 1);
        else
          *(top - 2) = __losu_objconst_null;
        *(top - 1) = tmp;
        break;
      }
      case INS_CREATEUNIT: {
        vm->top = top;
        losu_gc_newtask(vm);
        ovhash(top) = losu_objunit_new(vm, _losu_vmcg_getA(i));
        ovhash(top)->type = _losu_vmcg_getB(i);
        ovtype(top) = losu_object_type_unit;
        top++;
        break;
      }
      case INS_SETLOCAL: {
        *(base + _losu_vmcg_getU(i)) = *(--top);
        break;
      }
      case INS_SETGLOBAL: {
        vm->top = top;
        losu_object_string_t name =
            (losu_object_string_t)lcstr[_losu_vmcg_getU(i)];
        losu_object_t oval = losu_objunit_getstr(vm, vm->global, name);
        if (oval->type != losu_object_type_false)
          *oval = *(vm->top - 1);
        else {
          losu_object key = {
              .type = losu_object_type_string,
              .value.str = name,
          };
          *losu_objunit_set(vm, vm->global, &key) = *(vm->top - 1);
        }
        top--;
        break;
      }
      case INS_SETUNIT: {
        losu_object_t t = top - _losu_vmcg_getA(i);
        vm->top = top;
        if (ovtype(t) != losu_object_type_unit)
          vm_error(vm, "can't write properties of '%s'", obj_typestr(vm, t));
        *losu_objunit_set(vm, ovhash(t), t + 1) = *(vm->top - 1);
        // __losuVmcoreSetunit(vm, t, t + 1);
        top -= _losu_vmcg_getB(i);
        break;
      }
      case INS_SETLIST: {
        uint32_t aux = _losu_vmcg_getA(i) * _losu_vmlim_maxsetlist;
        _losu_vmins_B n = _losu_vmcg_getB(i);
        losu_object_hash_t arr = ovhash((top - n - 1));
        vm->top = top - n;
        for (; n; n--)
          *losu_objunit_setnum(vm, arr, aux + n - 1) = *(--top);
        break;
      }
      case INS_SETMAP: {
        _losu_vmins_U n = _losu_vmcg_getU(i);
        losu_object_t ftop = top - 2 * n;
        losu_object_hash_t arr = ovhash((ftop - 1));
        vm->top = ftop;
        for (; n; n--) {
          top -= 2;
          *losu_objunit_set(vm, arr, top) = *(top + 1);
        }
        break;
      }
      case INS_PIPE: {
        vm->top = top;  // update top
        cinfo->pc = pc;
        losu_object tmp = *(top - 1);
        *(top - 1) = *(top - 2);
        *(top - 2) = tmp;
        // int16_t nres = _losu_vmcg_getB(i);
        // if (nres == 255)
        //   nres = -1;
        // losu_object_t func = base + _losu_vmcg_getA(i);
        // printf("call %p,base %p\n", func, base);
        (*cinfo->inlinecall)(vm, top - 2, 1);
        top = vm->top;
        losu_gc_newtask(vm);
        break;
      }
      case INS_ADD: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_add(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't add %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        ovnumber(top - 2) = ovnumber(top - 2) + ovnumber(top - 1);
        top--;
        break;
      }
      case INS_SUB: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_sub(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't sub %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        ovnumber(top - 2) = ovnumber(top - 2) - ovnumber(top - 1);
        top--;
        break;
      }
      case INS_MULT: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_mult(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't mult %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        ovnumber(top - 2) = ovnumber(top - 2) * ovnumber(top - 1);
        top--;
        break;
      }
      case INS_DIV: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_div(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't div %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        if (ovnumber(top - 1) == 0)
          vm_error(vm, "can't div by '0");
        ovnumber(top - 2) = ovnumber(top - 2) / ovnumber(top - 1);
        top--;
        break;
      }
      case INS_POW: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_pow(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't pow %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        ovnumber(top - 2) = pow(ovnumber(top - 2), ovnumber(top - 1));
        top--;
        break;
      }
      case INS_MOD: {
        if (ovtype(top - 2) != losu_object_type_number ||
            ovtype(top - 1) != losu_object_type_number) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_mod(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't mod %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        ovnumber(top - 2) = fmod(ovnumber(top - 2), ovnumber(top - 1));
        top--;
        break;
      }
      case INS_CONCAT: {
        if (ovtype(top - 2) != losu_object_type_string ||
            ovtype(top - 1) != losu_object_type_string) {
          if (ovtype(top - 2) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_concat(vm, top - 2, top - 1);
            *(top - 2) = *(vm->top - 1);
            top--;
            vm->top = top;
            break;
          }
          vm_error(vm, "can't concat %s and %s", obj_typestr(vm, top - 2),
                   obj_typestr(vm, top - 1));
        }
        if (ovIstr((top - 1))->len > 0) {
          size_t p;
          uint64_t nl = ovIstr((top - 2))->len + ovIstr((top - 1))->len;
          if (nl >= _losu_vmlim_maxstrlen)
            vm_error(vm, "string length overflow");
          if (nl > vm->nbufftmp) {
            losu_mem_reallocvector(vm, vm->bufftmp, nl, unsigned char);
            vm->nblocks += (nl - vm->nbufftmp) * sizeof(char);
            vm->nbufftmp = nl;
          }

          p = ovIstr((top - 2))->len;
          memcpy(vm->bufftmp, ovSstr((top - 2)), p);
          memcpy(vm->bufftmp + p, ovSstr((top - 1)), ovIstr((top - 1))->len);
          ovIstr((top - 2)) =
              losu_objstring_newlen(vm, (const char*)(vm->bufftmp), nl);
        }
        top--;
        vm->top = top;
        losu_gc_newtask(vm);
        break;
      }
      case INS_NEG: {
        if (ovtype((top - 1)) != losu_object_type_number) {
          if (ovtype(top - 1) == losu_object_type_unit &&
              ovhash(top - 2)->type == obj_unittype_object) {
            vm->top = top;
            losu_vm_alu_neg(vm, top - 1);
            *(top - 1) = *(vm->top - 1);
            vm->top = top;
            break;
          }
          vm_error(vm, "can't neg %s", obj_typestr(vm, top - 1));
        }
        ovnumber((top - 1)) = -ovnumber((top - 1));
        break;
      }
      case INS_NOT: {
        ovtype((top - 1)) = (ovtype((top - 1)) == losu_object_type_false)
                                ? losu_object_type_true
                                : losu_object_type_false;
        break;
      }
      case INS_JMPNE: {
        top -= 2;
        if (!losu_object_iseq(vm, top, top + 1))
          dojmp(pc, i);
        break;
      }
      case INS_JMPEQ: {
        top -= 2;
        if (losu_object_iseq(vm, top, top + 1))
          dojmp(pc, i);
        break;
      }
      case INS_JMPLT: {
        top -= 2;
        if (losu_object_less(vm, top, top + 1))
          dojmp(pc, i);
        break;
      }
      case INS_JMPLE: {
        top -= 2;
        if (!losu_object_less(vm, top + 1, top))
          dojmp(pc, i);
        break;
      }
      case INS_JMPGT: {
        top -= 2;
        if (losu_object_less(vm, top + 1, top))
          dojmp(pc, i);
        break;
      }
      case INS_JMPGE: {
        top -= 2;
        if (!losu_object_less(vm, top, top + 1))
          dojmp(pc, i);
        break;
      }
      case INS_JMPT: {
        if (ovtype((--top)) != losu_object_type_false)
          dojmp(pc, i);
        break;
      }
      case INS_JMPF: {
        if (ovtype((--top)) == losu_object_type_false)
          dojmp(pc, i);
        break;
      }
      case INS_JMPONT: {
        if (ovtype((top - 1)) == losu_object_type_false)
          top--;
        else
          dojmp(pc, i);
        break;
      }
      case INS_JMPONF: {
        if (ovtype((top - 1)) != losu_object_type_false)
          top--;
        else
          dojmp(pc, i);
        break;
      }
      case INS_JMP: {
        dojmp(pc, i);
        break;
      }
      case INS_PUSHNULLJMP: {
        ovtype((top++)) = losu_object_type_false;
        pc++;
        break;
      }
      case INS_FORPREP: {
        if (ovtype((top - 1)) != losu_object_type_number)
          vm_error(vm, "invalid 'for' exper, '%s' type.", "__STEP__");
        if (ovtype((top - 2)) != losu_object_type_number)
          vm_error(vm, "invalid 'for' exper, '%s' type.", "__MAX__");
        if (ovtype((top - 3)) != losu_object_type_number)
          vm_error(vm, "invalid 'for' exper, '%s' type.", "__INIT__");
        if (ovnumber((top - 1)) > 0
                ? ovnumber((top - 3)) > ovnumber((top - 2))
                : ovnumber((top - 3)) < ovnumber((top - 2))) {
          top -= 3;
          dojmp(pc, i);
        }
        break;
      }
      case INS_FORNEXT: {
        if (ovtype((top - 3)) != losu_object_type_number)
          vm_error(vm, "invalid 'for' exper, '%s' type.", "__STEP__");
        ovnumber((top - 3)) += ovnumber((top - 1));
        if (ovnumber((top - 1)) > 0 ? ovnumber((top - 3)) > ovnumber((top - 2))
                                    : ovnumber((top - 3)) < ovnumber((top - 2)))
          top -= 3;
        else
          dojmp(pc, i);
        break;
      }
      case INS_FORINPREP: {
        losu_hash_node_t node;
        if (ovtype((top - 1)) != losu_object_type_unit)
          vm_error(vm, "invalid 'for' exper, '%s' type.", "__THIS__");
        node = (losu_hash_node_t)losu_objunit_getnext(
            vm, ovhash((top - 1)), (losu_object_t)&__losu_objconst_null);
        if (node == NULL) {
          top--;
          dojmp(pc, i);
        } else {
          top += 2;
          *(top - 2) = node->key;
          *(top - 1) = node->value;
        }
        break;
      }
      case INS_FORINNEXT: {
        losu_hash_node_t node;
        node = (losu_hash_node_t)losu_objunit_getnext(vm, ovhash((top - 3)),
                                                      top - 2);
        if (node == NULL)
          top -= 3;
        else {
          *(top - 2) = node->key;
          *(top - 1) = node->value;
          dojmp(pc, i);
        }
        break;
      }
      case INS_PUSHFUNCTION: {
        vm->top = top;
        {
          losu_object_function_t func =
              __losu_vmcore_function(vm, _losu_vmcg_getB(i));
          func->func.sdef = (losu_object_scode_t)lcfcode[_losu_vmcg_getA(i)];
          func->isC = 0;
        }
        top = vm->top;
        losu_gc_newtask(vm);
        break;
      }
      case INS_YIELD: {
        vm->top = top;
        cinfo->pc = pc;
        if (vm->yield)
          losu_sigmsg_throw(vm, losu_signal_yield);
        break;
      }
      case INS_IMPORT: {
        losu_object_string_t modname_t = lcstr[_losu_vmcg_getU(i)];
        const char* modname = modname_t->str;
        /* binary */
        losu_ctype_bool isfound = 0;
        for (uint32_t i = 0; i < vm->nmodule; i++) {
          if (strcmp(vm->module[i].name, modname) == 0) {
            if (!vm->module[i].isloaded) {
              *losu_objunit_setstr(vm, vm->global, modname_t) =
                  vm->module[i].load(vm);
              vm->module[i].isloaded = 1;
            }
            isfound = 1;
            break;
          }
        }
        if (!isfound) {
          vm_error(vm, "module not found: %s", modname);
          break;
        }

        break;
      }
      case INS_ASYNC: {
        vm->top = top;  // update top
        losu_object_t func = base + _losu_vmcg_getU(i);
        int32_t nargs = top - func - 1;
        losu_object_coroutine_t newcoro = losu_objcoro_new(vm, 256);
        *(newcoro->stack) = *func;
        for (int32_t i = 1; i <= nargs; i++)
          *(newcoro->stack + i) = *(func + i);
        newcoro->top = newcoro->stack + nargs + 1;
        vm->top = top = func;
        break;
      }
      case INS_COPY: {
        *top = *(top - 1);
        top++;
        break;
      }
      case INS_TEST: {
        printf("vm stack: %p\n", top);
        break;
      }
      default: {
        vm_error(vm, "Incorrect quality format, unknown OP '%d'.",
                 _losu_vmcg_getOP(i));
        break;
      }
    }
  }
  return NULL;
}

static void __losu_vmcore_adjvarg(losu_vm_t vm,
                                  losu_object_t base,
                                  int32_t nfixarg) {
  int32_t nv = (vm->top - base) - nfixarg;
  if (nv < 0)
    __losu_vmheap_adjtop(vm, base, nfixarg);
  {
    losu_object_t felem = base + nfixarg;
    int32_t i;
    losu_object_hash_t hunit = losu_objunit_new(vm, 0);
    hunit->type = obj_unittype_array;
    for (i = 0; felem + i < vm->top; i++)
      *losu_objunit_setnum(vm, hunit, i + 1) = *(felem + i);
    *losu_objunit_setnum(vm, hunit, 0) = (losu_object){
        .type = losu_object_type_number,
        .value.num = i,
    };
    vm->top = felem;
    ovtype(vm->top) = losu_object_type_unit;
    ovhash(vm->top) = hunit;
    if (vm->top == vm->stackmax)
      __losu_vmheap_check(vm, 1);
    vm->top++;
  }
}

static losu_object_function_t __losu_vmcore_function(losu_vm_t vm,
                                                     int32_t niss) {
  losu_object_function_t f = losu_objfunc_new(vm, niss);
  vm->top -= niss;
  while (niss--)
    f->closure[niss] = *(vm->top + niss);
  ovfunc(vm->top) = f;
  ovtype(vm->top) = losu_object_type_function;
  if (vm->top == vm->stackmax)
    __losu_vmheap_check(vm, 1);
  vm->top++;
  return f;
}
#endif