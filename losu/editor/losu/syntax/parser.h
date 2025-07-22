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

#ifndef FILE_SRC_SYNTAX_PARSER_H
#define FILE_SRC_SYNTAX_PARSER_H

#include "binder.h"
#include "losu_syntax.h"

/**
 * control segment
 * ----------------
 * control the syntax parse
 */
#if 1
static void __losu_syntaxpar_setin(losu_vm_t vm,
                                   losu_syntax_lex_t lex,
                                   losu_syntax_io_t io);

static void __losu_syntaxpar_token2str(int16_t tk, char* buf);

static void __losu_syntaxpar_error(losu_syntax_lex_t lex, int16_t tk);

static void __losu_syntaxpar_next(losu_syntax_lex_t lex);

/* func */
#if 1
static void __losu_syntaxpar_newfunc(losu_syntax_lex_t lex,
                                     losu_syntax_func_t func);

static void __losu_syntaxpar_delfunc(losu_syntax_lex_t lex);
#endif

/* check */
#if 1
static void __losu_syntaxpar_check(losu_syntax_lex_t lex, int16_t tk);

static losu_ctype_bool __losu_syntaxpar_checkblock(int16_t tk);

static void __losu_syntaxpar_checkcondition(losu_syntax_lex_t lex,
                                            losu_ctype_bool c,
                                            const char* errmsg);

static void __losu_syntaxpar_checkmatch(losu_syntax_lex_t lex,
                                        int16_t l,
                                        int16_t r,
                                        int32_t line);

static losu_object_string_t __losu_syntaxpar_checkname(losu_syntax_lex_t lex);

static void __losu_syntaxpar_checklimit(losu_syntax_lex_t lex,
                                        int32_t v,
                                        int32_t l,
                                        const char* errmsg);
#endif

/* local var */
#if 1
static void __losu_syntaxpar_newlcvar(losu_syntax_lex_t lex,
                                      losu_object_string_t name,
                                      int16_t i);

static void __losu_syntaxpar_adlcvar(losu_syntax_lex_t lex, int32_t nv);

static void __losu_syntaxpar_removelcvar(losu_syntax_lex_t lex, int16_t i);

#endif

/* global var */
static void __losu_syntaxpar_newglovar(losu_syntax_lex_t lex,
                                       losu_object_string_t name);

/* break & continue context */
#if 1
static void __losu_syntaxpar_enterbreak(losu_syntax_func_t func,
                                        losu_syntax_break_t br);
static void __losu_syntaxpar_exitbreak(losu_syntax_func_t func,
                                       losu_syntax_break_t br);
static void __losu_syntaxpar_entercontinue(losu_syntax_func_t func,
                                           losu_syntax_continue_t ctn,
                                           int32_t pc);
static void __losu_syntaxpar_exitcontinue(losu_syntax_func_t func,
                                          losu_syntax_continue_t ctn);

static void __losu_syntaxpar_enterraise(losu_syntax_func_t func,
                                        losu_syntax_raise_t r,
                                        int32_t pc);

static void __losu_syntaxpar_exitraise(losu_syntax_func_t func,
                                       losu_syntax_raise_t r);

#endif

/* stack */

static void __losu_syntaxpar_adstack(losu_syntax_lex_t lex,
                                     int32_t nv,
                                     int32_t ne);

#endif

/* stat = { ... } */
#if 1
static losu_ctype_bool __losu_syntaxpar_stat(losu_syntax_lex_t lex);

/* if */
static void __losu_syntaxpar_statif(losu_syntax_lex_t lex, int32_t line);

/* while */
static void __losu_syntaxpar_statwhile(losu_syntax_lex_t lex, int32_t line);

/* until */
static void __losu_syntaxpar_statuntil(losu_syntax_lex_t lex, int32_t line);

/* for */
#if 1
static void __losu_syntaxpar_statfor(losu_syntax_lex_t lex, int32_t line);
/* for k,v in obj */
static void __losu_syntaxpar_statforobj(losu_syntax_lex_t lex,
                                        losu_object_string_t name);
/* for i in x,y,z */
static void __losu_syntaxpar_statfornum(losu_syntax_lex_t lex,
                                        losu_object_string_t name);
static void __losu_syntaxpar_statforbody(losu_syntax_lex_t lex,
                                         losu_ctype_bool isObj);

/* async */
static void __losu_syntaxpar_statasync(losu_syntax_lex_t lex);

#endif

/* def */
#if 1
static void __losu_syntaxpar_statfunc(losu_syntax_lex_t lex, int32_t line);

static void __losu_syntaxpar_statfunc_name(losu_syntax_lex_t lex,
                                           losu_syntax_exp_t v);

static void __losu_syntaxpar_statfunc_body(losu_syntax_lex_t lex,

                                           int32_t line);

static void __losu_syntaxpar_statfunc_arg(losu_syntax_lex_t lex);

static int32_t __losu_syntaxpar_func_varlevel(losu_syntax_lex_t lex,
                                              losu_object_string_t n,
                                              losu_syntax_exp_t v);

#endif

/* lambda */
#if 1
static void __losu_syntaxpar_statlambda(losu_syntax_lex_t lex);

static void __losu_syntaxpar_statlambda_arg(losu_syntax_lex_t lex);

static void __losu_syntaxpar_statlambda_body(losu_syntax_lex_t lex);

#endif

/* let global */
#if 1
static void __losu_syntaxpar_statlet(losu_syntax_lex_t lex);

static void __losu_syntaxpar_statglo(losu_syntax_lex_t lex);
#endif

/* name */
#if 1
static void __losu_syntaxpar_statname(losu_syntax_lex_t lex);

static int32_t __losu_syntaxpar_statnamelist(losu_syntax_lex_t lex,
                                             losu_syntax_exp_t v,
                                             int32_t nvar);

#endif

/* match */
#if 1
static void __losu_syntaxpar_statmatch(losu_syntax_lex_t lex, int32_t line);
#endif

/* except assert raise */
#if 1
static void __losu_syntaxpar_statexcept(losu_syntax_lex_t lex);

static void __losu_syntaxpar_statraise(losu_syntax_lex_t lex);

#endif

/* return */
static void __losu_syntaxpar_statreturn(losu_syntax_lex_t lex);

/* break */
static void __losu_syntaxpar_statbreak(losu_syntax_lex_t lex);

/* continue */
static void __losu_syntaxpar_statcontinue(losu_syntax_lex_t lex);

/* yield */
static void __losu_syntaxpar_statyield(losu_syntax_lex_t lex);

/* class */
static void __losu_syntaxpar_statclass(losu_syntax_lex_t lex,
                                       int32_t line,
                                       losu_ctype_bool isGlobal);
static int32_t __losu_syntaxpar_class_varlevel(losu_syntax_lex_t lex,
                                               losu_object_string_t n,
                                               losu_syntax_exp_t v);
static int32_t __losu_syntaxpar_class_global_varlevel(losu_syntax_lex_t lex,
                                                      losu_object_string_t n,
                                                      losu_syntax_exp_t v);

/* import */
static void __losu_syntaxpar_statimport(losu_syntax_lex_t lex);

/* block */
static void __losu_syntaxpar_statblock(losu_syntax_lex_t lex);
#endif

/**
 * expression
 * */
#if 1
typedef int32_t (*varnamelevel)(losu_syntax_lex_t lex,
                                losu_object_string_t n,
                                losu_syntax_exp_t v);

static int32_t __losu_syntaxpar_explist(losu_syntax_lex_t lex);

static void __losu_syntaxpar_exp(losu_syntax_lex_t lex);

static int32_t __losu_syntaxpar_subexp(losu_syntax_lex_t lex,
                                       losu_syntax_exp_t v,
                                       int32_t left);

static int32_t __losu_syntaxpar_subexp_unop(int16_t tk);

static int32_t __losu_syntaxpar_subexp_binop(int16_t tk);

static void __losu_syntaxpar_subexp_stat(losu_syntax_lex_t lex,
                                         losu_syntax_exp_t v);

static void __losu_syntaxpar_expvarfunc(losu_syntax_lex_t lex,
                                        losu_syntax_exp_t v,
                                        varnamelevel getlocal,
                                        losu_ctype_bool async);

static void __losu_syntaxpar_firstname(losu_syntax_lex_t lex,
                                       losu_object_string_t n,
                                       losu_syntax_exp_t var,
                                       varnamelevel getlocal);

static void __losu_syntaxpar_expFargs(losu_syntax_lex_t lex,
                                      losu_ctype_bool slf,
                                      losu_ctype_bool async);

static void __losu_syntaxpar_exp_unitlist(losu_syntax_lex_t lex);

static void __losu_syntaxpar_exp_unitmap(losu_syntax_lex_t lex);

static int32_t __losu_syntaxpar_exp_varlevel(losu_syntax_lex_t lex,
                                             losu_object_string_t n,
                                             losu_syntax_exp_t v);

#endif

#endif