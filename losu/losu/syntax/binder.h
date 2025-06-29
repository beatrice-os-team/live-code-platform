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
#ifndef FILE_SRC_SYNTAX_BINDER_H
#define FILE_SRC_SYNTAX_BINDER_H

#include "losu_syntax.h"

void losu_syntax_binder_concat(losu_syntax_func_t func,
                               int32_t* l1,
                               int32_t l2);

int32_t losu_syntax_binder_jump(losu_syntax_func_t func);

void losu_syntax_binder_concat(losu_syntax_func_t func,
                               int32_t* l1,
                               int32_t l2);

int32_t losu_syntax_binder_jump(losu_syntax_func_t func);

void losu_syntax_binder_palist(losu_syntax_func_t func,
                               int32_t list,
                               int32_t target);

int32_t losu_syntax_binder_getL(losu_syntax_func_t func);

void losu_syntax_binder_goT(losu_syntax_func_t func,
                            losu_syntax_exp_t v,
                            int32_t keep);

void losu_syntax_binder_goF(losu_syntax_func_t func,
                            losu_syntax_exp_t v,
                            int32_t keep);

void losu_syntax_binder_tostack(losu_syntax_lex_t lex,
                                losu_syntax_exp_t v,
                                int32_t o);

void losu_syntax_binder_adstack(losu_syntax_func_t func, int32_t n);

void losu_syntax_binder_setvar(losu_syntax_lex_t lex, losu_syntax_exp_t var);

losu_ctype_bool losu_syntax_binder_isopen(losu_syntax_func_t func);

void losu_syntax_binder_setcallreturn(losu_syntax_func_t func, int32_t nres);

int32_t losu_syntax_binder_number(losu_syntax_func_t func, losu_ctype_number f);

int32_t losu_syntax_binder_strconst(losu_syntax_func_t func,
                                    losu_object_string_t s);

void losu_syntax_binder_prefix(losu_syntax_lex_t lex,
                               _losu_vmins_unOP op,
                               losu_syntax_exp_t v);

void losu_syntax_binder_infix(losu_syntax_lex_t lex,
                              _losu_vmins_binOP op,
                              losu_syntax_exp_t v);

void losu_syntax_binder_posfix(losu_syntax_lex_t lex,
                               _losu_vmins_binOP op,
                               losu_syntax_exp_t v1,
                               losu_syntax_exp_t v2);

int32_t losu_syntax_binder_getjump(losu_syntax_func_t func, int32_t pc);

void losu_syntax_binder_fixjump(losu_syntax_func_t func,
                                int32_t pc,
                                int32_t dt);

void losu_syntax_binder_plistfunc(losu_syntax_func_t func,
                                  int32_t list,
                                  int32_t tg,
                                  _losu_vmins_OP ins,
                                  int32_t instg);

void losu_syntax_binder_go(losu_syntax_func_t func,
                           losu_syntax_exp_t v,
                           int32_t inv,
                           _losu_vmins_OP jump);

losu_ctype_bool losu_syntax_binder_discharge(losu_syntax_func_t func,
                                             losu_syntax_exp_t v);

losu_ctype_bool losu_syntax_binder_needval(losu_syntax_func_t func,
                                           int32_t l,
                                           _losu_vmins_OP v);

_losu_vmins_OP losu_syntax_binder_jumpinvert(_losu_vmins_OP op);

#endif