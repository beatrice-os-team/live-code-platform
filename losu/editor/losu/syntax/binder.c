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
#ifndef FILE_SRC_SYNTAX_BINDER_C
#define FILE_SRC_SYNTAX_BINDER_C

#include <math.h>

#include "binder.h"
#include "losu_mem.h"

static const struct {
  _losu_vmins_OP op;
  int32_t arg;
} _alops[] = {
    {INS_ADD, 0},         {INS_SUB, 0},         {INS_MULT, 0},
    {INS_DIV, 0},         {INS_POW, 0},         {INS_MOD, 0},
    {INS_CONCAT, 0},      {INS_JMPNE, NO_JUMP}, {INS_JMPEQ, NO_JUMP},
    {INS_JMPLT, NO_JUMP}, {INS_JMPLE, NO_JUMP}, {INS_JMPGT, NO_JUMP},
    {INS_JMPGE, NO_JUMP}, {INS_PIPE, 0},
};

void losu_syntax_binder_concat(losu_syntax_func_t func,
                               int32_t* l1,
                               int32_t l2) {
  if (*l1 == NO_JUMP)
    *l1 = l2;
  else {
    int32_t l = *l1;
    while (1) {
      int32_t n = losu_syntax_binder_getjump(func, l);
      if (n == NO_JUMP) {
        losu_syntax_binder_fixjump(func, l, l2);
        return;
      }
      l = n;
    }
  }
}
int32_t losu_syntax_binder_jump(losu_syntax_func_t func) {
  int32_t j = losu_syntax_codegenarg1(func, INS_JMP, NO_JUMP);
  if (j == func->lasttarget) {
    losu_syntax_binder_concat(func, &j, func->jump);
    func->jump = NO_JUMP;
  }
  return j;
}

void losu_syntax_binder_palist(losu_syntax_func_t func,
                               int32_t list,
                               int32_t tg) {
  if (tg == func->lasttarget)
    losu_syntax_binder_concat(func, &func->jump, list);
  else
    losu_syntax_binder_plistfunc(func, list, tg, INS_END, 0);
}
int32_t losu_syntax_binder_getL(losu_syntax_func_t func) {
  if (func->pc != func->lasttarget) {
    int32_t ltg = func->lasttarget;
    func->lasttarget = func->pc;
    losu_syntax_binder_palist(func, func->jump, ltg);
    func->jump = NO_JUMP;
  }
  return func->pc;
}
void losu_syntax_binder_goT(losu_syntax_func_t func,
                            losu_syntax_exp_t v,
                            int32_t keep) {
  losu_syntax_binder_go(func, v, 1, keep ? INS_JMPONF : INS_JMPF);
}
void losu_syntax_binder_goF(losu_syntax_func_t func,
                            losu_syntax_exp_t v,
                            int32_t keep) {
  losu_syntax_binder_go(func, v, 0, keep ? INS_JMPONT : INS_JMPT);
}
void losu_syntax_binder_tostack(losu_syntax_lex_t lex,
                                losu_syntax_exp_t v,
                                int32_t o) {
#define codelab(func, op, arg) \
  (losu_syntax_binder_getL(func), losu_syntax_codegenarg1(func, op, arg))

  losu_syntax_func_t func = lex->fs;
  if (!losu_syntax_binder_discharge(func, v)) {
    _losu_vmins_OP pre = _losu_vmcg_getOP(func->fcode->code[func->pc - 1]);
    if (!_losu_vmins_isjmp(pre) && v->value._bool.f == NO_JUMP &&
        v->value._bool.t == NO_JUMP) {
      if (o)
        losu_syntax_binder_setcallreturn(func, 1);
    } else {
      int32_t final;
      int32_t j = NO_JUMP;
      int32_t pn = NO_JUMP;
      int32_t pl = NO_JUMP;
      if (_losu_vmins_isjmp(pre) ||
          losu_syntax_binder_needval(func, v->value._bool.f, INS_JMPONF) ||
          losu_syntax_binder_needval(func, v->value._bool.t, INS_JMPONT)) {
        if (_losu_vmins_isjmp(pre))
          losu_syntax_binder_concat(func, &v->value._bool.t, func->pc - 1);
        else {
          j = codelab(func, INS_JMP, NO_JUMP);
          losu_syntax_binder_adstack(func, 1);
        }
        pn = codelab(func, INS_PUSHNULLJMP, 0);
        pl = codelab(func, INS_PUSHTRUE, 1);
        losu_syntax_binder_palist(func, j, losu_syntax_binder_getL(func));
      }
      final = losu_syntax_binder_getL(func);
      losu_syntax_binder_plistfunc(func, v->value._bool.f, pn, INS_JMPONF,
                                   final);
      losu_syntax_binder_plistfunc(func, v->value._bool.t, pl, INS_JMPONT,
                                   final);
      v->value._bool.f = v->value._bool.t = NO_JUMP;
    }
  }

#undef codelab
}
void losu_syntax_binder_adstack(losu_syntax_func_t func, int32_t n) {
  if (n > 0)
    losu_syntax_codegenarg1(func, INS_POP, n);
  else
    losu_syntax_codegenarg1(func, INS_PUSHNULL, -n);
}
void losu_syntax_binder_setvar(losu_syntax_lex_t lex, losu_syntax_exp_t var) {
  losu_syntax_func_t func = lex->fs;
  switch (var->type) {
    case losu_syntax_exptype_local:
      losu_syntax_codegenarg1(func, INS_SETLOCAL, var->value.index);
      break;
    case losu_syntax_exptype_global:
      losu_syntax_codegenarg1(func, INS_SETGLOBAL, var->value.index);
      break;
    case losu_syntax_exptype_index:
      losu_syntax_codegenarg2(func, INS_SETUNIT, 3, 3);
      break;
    default:
      break;
  }
}
losu_ctype_bool losu_syntax_binder_isopen(losu_syntax_func_t func) {
  losu_ctype_vmins_t i = func->pc > func->lasttarget
                             ? func->fcode->code[func->pc - 1]
                             : ((losu_ctype_vmins_t)INS_END);
  if (_losu_vmcg_getOP(i) == INS_CALL && _losu_vmcg_getB(i) == 255)
    return 1;
  else
    return 0;
}
void losu_syntax_binder_setcallreturn(losu_syntax_func_t func, int32_t nres) {
  if (losu_syntax_binder_isopen(func)) {
    _losu_vmcg_setB(func->fcode->code[func->pc - 1], nres);
    losu_syntax_codegen_dtstack(func, nres);
  }
}
int32_t losu_syntax_binder_number(losu_syntax_func_t func,
                                  losu_ctype_number n) {
  losu_object_scode_t fc = func->fcode;
  int32_t c = fc->nlcnum;
  /*
    int32_tlim = c < LOOKBACKNUMS ? 0 : c - LOOKBACKNUMS;
    while (--c >= lim)
  */
  int32_t lim = c < _losu_vmlim_maxnumbuff ? 0 : c - _losu_vmlim_maxnumbuff;
  while (--c >= lim)
    if (fc->lcnum[c] == n)
      return c;

  losu_mem_growvector(func->vm, fc->lcnum, fc->nlcnum, 1, losu_ctype_number,
                      _losu_vmins_maxU, "'number' overflow");
  c = fc->nlcnum++;
  fc->lcnum[c] = n;
  return c;
}

int32_t losu_syntax_binder_strconst(losu_syntax_func_t func,
                                    losu_object_string_t s) {
  losu_object_scode_t f = func->fcode;
  int32_t c = s->cstidx;
  if (c >= f->nlcstr || f->lcstr[c] != s) {
    losu_mem_growvector(func->vm, f->lcstr, f->nlcstr, 1, losu_object_string_t,
                        _losu_vmins_maxU, "'string' overflow");
    c = f->nlcstr++;
    s->cstidx = c;
    f->lcstr[c] = s;
  }
  return c;
}

void losu_syntax_binder_prefix(losu_syntax_lex_t lex,
                               _losu_vmins_unOP op,
                               losu_syntax_exp_t v) {
#define swap(v)                          \
  {                                      \
    int32_t tmp = v->value._bool.f;      \
    v->value._bool.f = v->value._bool.t; \
    v->value._bool.t = tmp;              \
  }

  losu_syntax_func_t func = lex->fs;
  if (op == INS_UNOP_NEG) {
    losu_syntax_binder_tostack(lex, v, 1);
    losu_syntax_codegenarg0(func, INS_NEG);
  } else {
    losu_ctype_vmins_t* pre;
    losu_syntax_binder_discharge(func, v);
    if (v->value._bool.t == NO_JUMP && v->value._bool.f == NO_JUMP)
      losu_syntax_binder_setcallreturn(func, 1);

    pre = &(func->fcode->code[func->pc - 1]);
    if (_losu_vmins_isjmp(_losu_vmcg_getOP((*pre))))
      _losu_vmcg_setOP((*pre),
                       losu_syntax_binder_jumpinvert(_losu_vmcg_getOP((*pre))));
    else
      losu_syntax_codegenarg0(func, INS_NOT);

    swap(v);
  }

#undef swap
}
void losu_syntax_binder_infix(losu_syntax_lex_t lex,
                              _losu_vmins_binOP op,
                              losu_syntax_exp_t v) {
  losu_syntax_func_t func = lex->fs;
  switch (op) {
    case INS_BINOP_AND: {
      losu_syntax_binder_goT(func, v, 1);
      break;
    }
    case INS_BINOP_OR: {
      losu_syntax_binder_goF(func, v, 1);
      break;
    }
    default: {
      losu_syntax_binder_tostack(lex, v, 1);
      break;
    }
  }
}
void losu_syntax_binder_posfix(losu_syntax_lex_t lex,
                               _losu_vmins_binOP op,
                               losu_syntax_exp_t v1,
                               losu_syntax_exp_t v2) {
  losu_syntax_func_t func = lex->fs;
  switch (op) {
    case INS_BINOP_AND: {
      losu_syntax_binder_discharge(func, v2);
      if (v2->value._bool.t == NO_JUMP && v2->value._bool.f == NO_JUMP)
        losu_syntax_binder_setcallreturn(func, 1);
      v1->value._bool.t = v2->value._bool.t;
      losu_syntax_binder_concat(func, &v1->value._bool.f, v2->value._bool.f);
      break;
    }
    case INS_BINOP_OR: {
      losu_syntax_binder_discharge(func, v2);
      if (v2->value._bool.t == NO_JUMP && v2->value._bool.f == NO_JUMP)
        losu_syntax_binder_setcallreturn(func, 1);
      v1->value._bool.f = v2->value._bool.f;
      losu_syntax_binder_concat(func, &v1->value._bool.t, v2->value._bool.t);
      break;
    }
    default: {
      losu_syntax_binder_tostack(lex, v2, 1);
      losu_syntax_codegenarg1(func, _alops[op].op, _alops[op].arg);
      break;
    }
  }
}

int32_t losu_syntax_binder_getjump(losu_syntax_func_t func, int32_t pc) {
  int32_t o = _losu_vmcg_getS(func->fcode->code[pc]);
  if (o == NO_JUMP)
    return NO_JUMP;
  else
    return (pc + 1) + o;
}
void losu_syntax_binder_fixjump(losu_syntax_func_t func,
                                int32_t pc,
                                int32_t dt) {
  losu_ctype_vmins_t* jmp = &func->fcode->code[pc];
  if (dt == NO_JUMP)
    _losu_vmcg_setS((*jmp), NO_JUMP);
  else {
    int32_t o = dt - (pc + 1);
    if (abs(o) > _losu_vmins_maxS)
      losu_syntax_error(func->lexer, "too long jump");
    _losu_vmcg_setS((*jmp), o);
  }
}
void losu_syntax_binder_plistfunc(losu_syntax_func_t func,
                                  int32_t list,
                                  int32_t tg,
                                  _losu_vmins_OP ins,
                                  int32_t instg) {
  losu_ctype_vmins_t* code = func->fcode->code;
  while (list != NO_JUMP) {
    int32_t next = losu_syntax_binder_getjump(func, list);
    losu_ctype_vmins_t* i = &code[list];
    _losu_vmins_OP op = _losu_vmcg_getOP((*i));
    if (op == ins)
      losu_syntax_binder_fixjump(func, list, instg);
    else {
      losu_syntax_binder_fixjump(func, list, tg);
      if (op == INS_JMPONT)
        _losu_vmcg_setOP((*i), INS_JMPT);
      else if (op == INS_JMPONF)
        _losu_vmcg_setOP((*i), INS_JMPF);
    }
    list = next;
  }
}
void losu_syntax_binder_go(losu_syntax_func_t func,
                           losu_syntax_exp_t v,
                           int32_t inv,
                           _losu_vmins_OP jump) {
  int32_t prepc;
  losu_ctype_vmins_t* pre;
  int32_t *golist, *exitlist;
  if (!inv) {
    golist = &v->value._bool.f;
    exitlist = &v->value._bool.t;
  } else {
    golist = &v->value._bool.t;
    exitlist = &v->value._bool.f;
  }
  losu_syntax_binder_discharge(func, v);
  if (v->value._bool.t == NO_JUMP && v->value._bool.f == NO_JUMP)
    losu_syntax_binder_setcallreturn(func, 1);
  prepc = func->pc - 1;
  pre = &func->fcode->code[prepc];
  if (!_losu_vmins_isjmp(_losu_vmcg_getOP(*pre)))
    prepc = losu_syntax_codegenarg1(func, jump, NO_JUMP);
  else if (inv)
    _losu_vmcg_setOP(*pre,
                     losu_syntax_binder_jumpinvert(_losu_vmcg_getOP(*pre)));
  losu_syntax_binder_concat(func, exitlist, prepc);
  losu_syntax_binder_palist(func, *golist, losu_syntax_binder_getL(func));
  *golist = NO_JUMP;
}
losu_ctype_bool losu_syntax_binder_discharge(losu_syntax_func_t func,
                                             losu_syntax_exp_t v) {
  switch (v->type) {
    case losu_syntax_exptype_local: {
      losu_syntax_codegenarg1(func, INS_GETLOCAL, v->value.index);
      break;
    }
    case losu_syntax_exptype_global: {
      losu_syntax_codegenarg1(func, INS_GETGLOBAL, v->value.index);
      break;
    }
    case losu_syntax_exptype_index: {
      losu_syntax_codegenarg0(func, INS_GETUNIT);
      break;
    }
    case losu_syntax_exptype_expr: {
      return 0;
    }
    default:
      break;
  }
  v->type = losu_syntax_exptype_expr;
  v->value._bool.t = v->value._bool.f = NO_JUMP;
  return 1;
}
losu_ctype_bool losu_syntax_binder_needval(losu_syntax_func_t func,
                                           int32_t l,
                                           _losu_vmins_OP op) {
  for (; l != NO_JUMP; l = losu_syntax_binder_getjump(func, l))
    if (_losu_vmcg_getOP(func->fcode->code[l]) != op)
      return 1;
  return 0;
}
_losu_vmins_OP losu_syntax_binder_jumpinvert(_losu_vmins_OP op) {
  switch (op) {
    case INS_JMPNE:
      return INS_JMPEQ;
    case INS_JMPEQ:
      return INS_JMPNE;

    case INS_JMPLT:
      return INS_JMPGE;
    case INS_JMPLE:
      return INS_JMPGT;

    case INS_JMPGT:
      return INS_JMPLE;
    case INS_JMPGE:
      return INS_JMPLT;

    case INS_JMPT:
    case INS_JMPONT:
      return INS_JMPF;

    case INS_JMPF:
    case INS_JMPONF:
      return INS_JMPT;

    default:
      return INS_END;
  }
}

#endif