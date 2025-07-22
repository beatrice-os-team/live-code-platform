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
#ifndef FILE_SRC_SYNTAX_CODEGEN_C
#define FILE_SRC_SYNTAX_CODEGEN_C

#include "codegen.h"
#include "binder.h"
#include "losu_bytecode.h"
#include "losu_mem.h"

int32_t losu_syntax_codegen(losu_syntax_func_t func,
                            _losu_vmins_OP o,
                            int32_t arg1,
                            int32_t arg2) {
  losu_syntax_lex_t lex = func->lexer;
  losu_object_scode_t f = func->fcode;
  losu_ctype_vmins_t i = func->pc > func->lasttarget
                             ? func->fcode->code[func->pc - 1]
                             : ((losu_ctype_vmins_t)INS_END);
  int32_t dt = _vmops[o].dlt;
  losu_ctype_bool optd = 0;

  /* Optimizer */
  switch (o) {
    /* V: pushnull/true pop */
    case INS_PUSHNULL: {
      if (arg1 == 0)
        return NO_JUMP;
      dt = arg1;
      switch (_losu_vmcg_getOP(i)) {
        case INS_PUSHNULL:
          _losu_vmcg_setU(i, _losu_vmcg_getU(i) + arg1);
          optd = 1;
          break;
        default:
          break;
      }
      break;
    }
    case INS_PUSHTRUE: {
      if (arg1 == 0)
        return NO_JUMP;
      dt = arg1;
      switch (_losu_vmcg_getOP(i)) {
        case INS_PUSHTRUE:
          _losu_vmcg_setU(i, _losu_vmcg_getU(i) + arg1);
          optd = 1;
          break;
        default:
          break;
      }
      break;
    }
    case INS_POP: {
      if (arg1 == 0)
        return NO_JUMP;
      dt = -arg1;
      switch (_losu_vmcg_getOP(i)) {
        case INS_SETUNIT: {
          _losu_vmcg_setB(i, _losu_vmcg_getB(i) + arg1);
          optd = 1;
          break;
        }
        default:
          break;
      }
      break;
    }
    /* V: pushfunc */
    case INS_PUSHFUNCTION: {
      dt = -arg2 + 1;
      break;
    }
    /* V: setunit setlist setmap */
    case INS_SETUNIT: {
      dt = -arg2;
      break;
    }
    case INS_SETLIST: {
      if (arg2 == 0)
        return NO_JUMP;
      dt = -arg2;
      break;
    }
    case INS_SETMAP: {
      if (arg1 == 0)
        return NO_JUMP;
      dt = -2 * arg1;
      break;
    }
    /* jumps */
    case INS_JMPNE: {
      if (i == _losu_vmcg_newU(INS_PUSHNULL, 1)) {
        i = _losu_vmcg_newS(INS_JMPT, NO_JUMP);
        optd = 1;
      }
      break;
    }
    case INS_JMPEQ: {
      if (i == _losu_vmcg_newU(INS_PUSHNULL, 1)) {
        i = _losu_vmcg_newOP(INS_NOT);
        dt = -1;
        optd = 1;
      }
      break;
    }
    case INS_JMPT:
    case INS_JMPONT: {
      switch (_losu_vmcg_getOP(i)) {
        case INS_NOT: {
          i = _losu_vmcg_newS(INS_JMPF, NO_JUMP);
          optd = 1;
          break;
        }
        case INS_PUSHNULL: {
          if (_losu_vmcg_getU(i) == 1) {
            func->pc--;
            losu_syntax_codegen_dtstack(func, -1);
            return NO_JUMP;
          }
          break;
        }
        default:
          break;
      }
      break;
    }
    case INS_JMPF:
    case INS_JMPONF: {
      switch (_losu_vmcg_getOP(i)) {
        case INS_NOT: {
          i = _losu_vmcg_newS(INS_JMPT, NO_JUMP);
          optd = 1;
          break;
        }
        case INS_PUSHNULL: {
          if (_losu_vmcg_getU(i) == 1) {
            i = _losu_vmcg_newS(INS_JMP, NO_JUMP);
            optd = 1;
          }
          break;
        }
        default:
          break;
      }
      break;
    }
    /* alu */
    case INS_NEG: {
      switch (_losu_vmcg_getOP(i)) {
        case INS_PUSHNUM: {
          losu_ctype_number n = func->fcode->lcnum[_losu_vmcg_getU(i)];
          _losu_vmcg_setU(i, losu_syntax_binder_number(func, -n));
          optd = 1;
          break;
        }
        default: {
          break;
        }
      }
      break;
    }

    default:
      break;
  }
  losu_syntax_codegen_dtstack(func, dt);
  if (optd) {
    func->fcode->code[func->pc - 1] = i;
    return func->pc - 1;
  }
  switch (_vmops[o].op) {
    case INS_MODE_OP: {
      i = _losu_vmcg_newOP(o);
      break;
    }
    case INS_MODE_IU: {
      i = _losu_vmcg_newU(o, arg1);
      break;
    }
    case INS_MODE_IS: {
      i = _losu_vmcg_newS(o, arg1);
      break;
    }
    case INS_MODE_IBA: {
      i = _losu_vmcg_newBA(o, arg1, arg2);
      break;
    }
    default:
      break;
  }
  if (lex->lastline > func->lastline) {
    losu_mem_growvector(func->vm, f->lineinfo, f->nlineinfo, 2, int32_t,
                        _losu_vmlim_maxint32, "line number overflow");
    if (lex->lastline > func->lastline + 1)
      f->lineinfo[f->nlineinfo++] = -(lex->lastline - (func->lastline + 1));
    f->lineinfo[f->nlineinfo++] = func->pc;
    func->lastline = lex->lastline;
  }
  losu_mem_growvector(func->vm, func->fcode->code, func->pc, 1,
                      losu_ctype_vmins_t, _losu_vmlim_maxint32,
                      "instruction overflow");
  func->fcode->code[func->pc] = i;
  return func->pc++;
}

void losu_syntax_codegen_dtstack(losu_syntax_func_t func, int32_t dt) {
  func->stacklevel += dt;
  if (func->stacklevel > func->fcode->maxstacksize) {
    if (func->stacklevel > _losu_vmlim_maxstack)
      losu_syntax_error(func->lexer, "too complex expressions");
    func->fcode->maxstacksize = func->stacklevel;
  }
}

#endif