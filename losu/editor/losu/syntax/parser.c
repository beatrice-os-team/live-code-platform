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

#ifndef FILE_SRC_SYNTAX_PARSER_C
#define FILE_SRC_SYNTAX_PARSER_C

#include "parser.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "token.h"

#include <stdio.h>
#include <string.h>

const char* losu_syntax_inlinefunc[] = {
    "global",
    "_",
    /* io */
    "print",
    "input",
    "exit",
    /* type */
    "type",
    "int",
    "float",
    "str",
    "chr",
    "len",
    /* vm */
    "eval",
    "exec",
    "package",
    "assert",
};

/**
 * @brief parse the source code
 * @param vm losu vm handle
 * @param io in io handle
 * @return  the scode object
 */
losu_object_scode_t losu_syntax_parse(losu_vm_t vm, losu_syntax_io_t io) {
  losu_syntax_lex lex = {0};
  char tmp[256] = {0};
  char tk[64] = {0};
  lex.idt = (losu_syntax_lex_idt){
      .nowidt = 0,
      .read = 0,
      .size = 4,
      .tmp = 0,
  };

  losu_syntax_func func = {0};
  losu_ctype_bool islast = 0;

  __losu_syntaxpar_setin(vm, &lex, io);
  if (vm->top == vm->stackmax)
    losu_syntax_error(&lex, "stack overflow");
  // init lex
  losu_syntax_lex_init(&lex);
  __losu_syntaxpar_newfunc(&lex, &func);
  for (int i = 0; i < sizeof(losu_syntax_inlinefunc) / sizeof(const char*); i++)
    __losu_syntaxpar_newglovar(
        &lex, losu_objstring_newconst(vm, losu_syntax_inlinefunc[i]));
  __losu_syntaxpar_next(&lex);
  while (!islast && !__losu_syntaxpar_checkblock(lex.tk.token))
    islast = __losu_syntaxpar_stat(&lex);
  __losu_syntaxpar_token2str(lex.tk.token, tk);
  snprintf(tmp, 32, "invalid token '%s'", tk);
  __losu_syntaxpar_checkcondition(&lex, (lex.tk.token == TOKEN_EOZ), tmp);
  __losu_syntaxpar_delfunc(&lex);
  return func.fcode;
}

/**
 * control segment
 * ----------------
 * control the syntax parse
 */
#if 1

/**
 * @brief set the input io
 * @param vm the vm
 * @param lex the lex
 * @param io the io
 */
static void __losu_syntaxpar_setin(losu_vm_t vm,
                                   losu_syntax_lex_t lex,
                                   losu_syntax_io_t io) {
  lex->vm = vm;
  lex->tkahead.token = TOKEN_EOZ;
  lex->io = io;
  lex->fs = NULL;
  lex->lastline = lex->linenumber = 1;
}

/**
 * @brief  get the token string
 * @param tk  the token
 * @param buf  the buffer
 */
static void __losu_syntaxpar_token2str(int16_t tk, char* buf) {
  if (tk < 255) {
    buf[0] = (char)((unsigned char)tk);
    buf[1] = '\0';
  } else {
    for (int32_t i = 0;
         i < sizeof(losu_syntax_keyword) / sizeof(losu_syntax_keyword_t); i++) {
      /* code */
      if (losu_syntax_keyword[i].token == tk) {
        snprintf(buf, 16, "%s", losu_syntax_keyword[i].str);
        return;
      }
    }
  }
}

/**
 * @brief throw error from syntax parser
 * @param lex  the lexer
 * @param tk  the token
 */
static void __losu_syntaxpar_error(losu_syntax_lex_t lex, int16_t tk) {
  char t[16] = {0};
  __losu_syntaxpar_token2str(tk, t);
  losu_syntax_error(lex, "excpected '%s'", t);
  return;
}

/**
 * @brief  add new function block to lexer
 * @param lex  the lexer
 * @param func  the function
 */
static void __losu_syntaxpar_newfunc(losu_syntax_lex_t lex,
                                     losu_syntax_func_t func) {
  losu_object_scode_t f = (losu_object_scode_t)losu_objsocde_new(lex->vm);
  func->prev = lex->fs;
  func->lexer = lex;
  func->vm = lex->vm;
  func->stacklevel = 0;
  func->naloc = 0;
  func->nclosure = 0;
  func->breaklist = NULL;
  func->contlist = NULL;
  func->fcode = f;
  func->pc = 0;
  func->lasttarget = 0;
  func->lastline = 0;
  func->jump = NO_JUMP;

  lex->fs = func;

  f->code = NULL;
  f->maxstacksize = 0;
  f->narg = 0;
  f->isVarg = 0;
  f->marked = 0;
}

/**
 * @brief  del last func block from lex
 * @param lex lexer
 */
static void __losu_syntaxpar_delfunc(losu_syntax_lex_t lex) {
  losu_vm_t vm = lex->vm;
  losu_syntax_func_t func = lex->fs;
  losu_object_scode_t f = func->fcode;
  losu_syntax_codegenarg0(func, INS_END);
  losu_syntax_binder_getL(func);
  __losu_syntaxpar_removelcvar(lex, func->naloc);
  losu_mem_reallocvector(vm, f->localvar, f->nlocalvar, losu_objscode_locvar);
  losu_mem_reallocvector(vm, f->code, func->pc, losu_ctype_vmins_t);
  losu_mem_reallocvector(vm, f->lcstr, f->nlcstr, losu_object_string_t);
  losu_mem_reallocvector(vm, f->lcnum, f->nlcnum, losu_ctype_number);
  losu_mem_reallocvector(vm, f->lcscode, f->nlcscode, losu_object_scode_t);
  losu_mem_reallocvector(vm, f->lineinfo, f->nlineinfo + 1, int32_t);
  f->lineinfo[f->nlineinfo++] = INT32_MAX;

#define sizeObjScode(f)                                                 \
  ((sizeof(losu_object_scode) + f->nlcnum * sizeof(losu_ctype_number) + \
    f->nlcstr * sizeof(losu_object_string_t) +                          \
    f->nlcscode * sizeof(losu_object_scode_t) +                         \
    f->ncode * sizeof(losu_ctype_vmins_t) +                             \
    f->nlocalvar * sizeof(losu_objscode_locvar) +                       \
    f->nlineinfo * sizeof(int32_t)))

  f->ncode = func->pc;
  vm->nblocks += sizeObjScode(f);
#undef sizeObjScode
  lex->fs = func->prev;
}

/**
 * @brief check token
 * @param lex lexer
 * @param tk token
 */
static void __losu_syntaxpar_check(losu_syntax_lex_t lex, int16_t tk) {
  if (lex->tk.token != tk)
    __losu_syntaxpar_error(lex, tk);
  __losu_syntaxpar_next(lex);
}

/**
 * @brief check the token is block
 * @param tk token
 * @return  true if block
 */
static losu_ctype_bool __losu_syntaxpar_checkblock(int16_t tk) {
  switch (tk) {
    case TOKEN_ELSE:
    case TOKEN_ELSEIF:
    case TOKEN_PASS:
    case TOKEN_EOZ:
      return 1;
    default:
      return 0;
  }
  return 0;
}

/**
 * @brief check the condition
 * @param lex lexer
 * @param c condition
 * @param errmsg error message
 */
static void __losu_syntaxpar_checkcondition(losu_syntax_lex_t lex,
                                            losu_ctype_bool c,
                                            const char* errmsg) {
  if (!c)
    losu_syntax_error(lex, errmsg);
}
/**
 * @brief check token (L&R) is match
 * @param lex lexer
 * @param l left token
 * @param r right token
 * @param line line
 */
static void __losu_syntaxpar_checkmatch(losu_syntax_lex_t lex,
                                        int16_t l,
                                        int16_t r,
                                        int32_t line) {
  if (lex->tk.token != r) {
    char tl[16], tr[16];
    __losu_syntaxpar_token2str(l, tl);
    __losu_syntaxpar_token2str(r, tr);
    losu_syntax_error(lex, " '%s' (line %d), expected '%s' ", tl, line, tr);
  }
  __losu_syntaxpar_next(lex);
}

/**
 * @brief check token is TOKEN_NAME
 * @param lex lexer
 * @return if is, return token value
 */
static losu_object_string_t __losu_syntaxpar_checkname(losu_syntax_lex_t lex) {
  losu_object_string_t ts;
  __losu_syntaxpar_checkcondition(lex, lex->tk.token == TOKEN_NAME,
                                  "expected [TOKEN_NAME] ");
  ts = lex->tk.info.s;
  __losu_syntaxpar_next(lex);
  return ts;
}

/**
 * @brief check limit (if v<=l)
 * @param lex lexer
 * @param v value
 * @param l limit
 * @param errmsg error message
 */
static void __losu_syntaxpar_checklimit(losu_syntax_lex_t lex,
                                        int32_t v,
                                        int32_t l,
                                        const char* errmsg) {
  if (v <= l)
    return;
  losu_syntax_error(lex, "too many [%s], max: %d", errmsg, l);
}

/**
 * @brief new local variable
 * @param lex lexer
 * @param name name
 * @param i index
 */
static void __losu_syntaxpar_newlcvar(losu_syntax_lex_t lex,
                                      losu_object_string_t name,
                                      int16_t i) {
  losu_syntax_func_t func = lex->fs;
  losu_object_scode_t f = func->fcode;
  __losu_syntaxpar_checklimit(lex, func->naloc + i + 1, _losu_vmlim_maxlocvar,
                              "local variables");
  losu_mem_growvector(lex->vm, f->localvar, f->nlocalvar, 1,
                      losu_objscode_locvar, _losu_vmins_maxA,
                      "local variables");
  f->localvar[f->nlocalvar].name = name;
  func->aloc[func->naloc + i] = f->nlocalvar++;
}

/**
 * @brief adjust a local variable to the function
 * @param lex the lexer
 * @param nv the number of variables
 */
static void __losu_syntaxpar_adlcvar(losu_syntax_lex_t lex, int32_t nv) {
  while (nv--)
    lex->fs->fcode->localvar[lex->fs->aloc[lex->fs->naloc++]].startpc =
        lex->fs->pc;
}

/**
 * @brief remove a local variable from the function
 * @param lex the lexer
 * @param i the index of the variable
 */
static void __losu_syntaxpar_removelcvar(losu_syntax_lex_t lex, int16_t i) {
  losu_syntax_func_t func = lex->fs;
  while (i--)
    func->fcode->localvar[func->aloc[--(func->naloc)]].endpc = func->pc;
}

/**
 * @brief new global variable
 * @param lex the lexer
 * @param name the name
 */
static void __losu_syntaxpar_newglovar(losu_syntax_lex_t lex,
                                       losu_object_string_t name) {
  losu_syntax_func_t func = lex->fs;
  // is local
  for (int32_t i = func->naloc - 1; i >= 0; i--) {
    if (name == func->fcode->localvar[func->aloc[i]].name) {
      losu_syntax_error(lex, "'%s' conflict with local variable.", name->str);
      return;
    }
  }
  if (func->prev) {
    for (int32_t i = func->prev->naloc - 1; i >= 0; i--) {
      if (name == func->prev->fcode->localvar[func->prev->aloc[i]].name) {
        losu_syntax_error(lex, "'%s' conflict with upvalue variable.",
                          name->str);
        return;
      }
    }
  }
  for (int16_t i = 0; i < func->nglovar; i++)
    if (func->glovar[i] == name)
      return;
  __losu_syntaxpar_checklimit(lex, func->nglovar + 1, _losu_vmlim_maxglovar,
                              "global variables");
  func->glovar[func->nglovar++] = name;
}

/**
 * @brief enter break
 * @param func the function
 * @param br the break handle
 */
static void __losu_syntaxpar_enterbreak(losu_syntax_func_t func,
                                        losu_syntax_break_t br) {
  br->stacklevel = func->stacklevel;
  br->breaklist = NO_JUMP;
  br->pre = func->breaklist;
  func->breaklist = br;
}

/**
 * @brief leave break
 * @param func the function
 * @param br the break handle
 */
static void __losu_syntaxpar_exitbreak(losu_syntax_func_t func,
                                       losu_syntax_break_t br) {
  func->breaklist = br->pre;
  losu_syntax_binder_palist(func, br->breaklist, losu_syntax_binder_getL(func));
}

/**
 * @brief enter continue context
 * @param func the function
 * @param ctn the continue handle
 * @param pc the pc
 */
static void __losu_syntaxpar_entercontinue(losu_syntax_func_t func,
                                           losu_syntax_continue_t ctn,
                                           int32_t pc) {
  ctn->stacklevel = func->stacklevel;
  ctn->tmppc = NO_JUMP;
  ctn->startpos = pc;
  ctn->pre = func->contlist;
  func->contlist = ctn;
}

/**
 * @brief leave the continue context
 * @param func the function
 * @param ctn the continue handle
 */
static void __losu_syntaxpar_exitcontinue(losu_syntax_func_t func,
                                          losu_syntax_continue_t ctn) {
  func->contlist = ctn->pre;
}

static void __losu_syntaxpar_enterraise(losu_syntax_func_t func,
                                        losu_syntax_raise_t r,
                                        int32_t pc) {
  r->stacklevel = func->stacklevel;
  r->startpc = pc;
  r->pre = func->raiselist;
  func->raiselist = r;
}

static void __losu_syntaxpar_exitraise(losu_syntax_func_t func,
                                       losu_syntax_raise_t r) {
  func->raiselist = r->pre;
}

/**
 * @brief adjust the stack
 * @param lex the lexer
 * @param nv the number of variables
 * @param ne the number of expressions
 */
static void __losu_syntaxpar_adstack(losu_syntax_lex_t lex,
                                     int32_t nv,
                                     int32_t ne) {
  losu_syntax_func_t func = lex->fs;
  int32_t d = ne - nv;
  if (ne > 0 && losu_syntax_binder_isopen(func)) {
    d--;
    if (d < 0) {
      losu_syntax_binder_setcallreturn(func, -d);
      d = 0;
    } else
      losu_syntax_binder_setcallreturn(func, 0);
  }
  losu_syntax_binder_adstack(func, d);
}

/**
 * @brief get next token
 * @param lex the lexer
 */
static void __losu_syntaxpar_next(losu_syntax_lex_t lex) {
  lex->lastline = lex->linenumber;
  if (lex->tkahead.token != TOKEN_EOZ) {
    lex->tk = lex->tkahead;
    lex->tkahead.token = TOKEN_EOZ;
  } else
    lex->tk.token =
        losu_syntax_lex_next(lex, (losu_syntax_tkvalue_t)(&(lex->tk.info)));
}

#endif

/**
 * stat = {
 *    if        |
 *    while     |
 *    until     |
 *    for       |
 *    def       |
 *    let       |
 *    global    |
 *    name      |
 *    return    |
 *    break     |
 *    continue  |
 *    yield     |
 *    class     |
 *    import    |
 *    async     |
 *    '('       |
 *    '{'       |
 *    '['       |
 * }
 *  */
#if 1

/**
 * @brief check stat of block
 * @param lex the lexer
 * @return 1 is block end
 */
static losu_ctype_bool __losu_syntaxpar_stat(losu_syntax_lex_t lex) {
  int32_t line = lex->linenumber;
  switch (lex->tk.token) {
    case TOKEN_IF: {
      __losu_syntaxpar_statif(lex, line);

      break;
    }
    case TOKEN_WHILE: {
      __losu_syntaxpar_statwhile(lex, line);

      break;
    }
    case TOKEN_UNTIL: {
      __losu_syntaxpar_statuntil(lex, line);

      break;
    }
    case TOKEN_FOR: {
      __losu_syntaxpar_statfor(lex, line);

      break;
    }
    case TOKEN_DEF: {
      __losu_syntaxpar_statfunc(lex, line);

      break;
    }
    case TOKEN_LET: {
      __losu_syntaxpar_statlet(lex);
      break;
    }
    case TOKEN_GLOBAL: {
      lex->tkahead.token = losu_syntax_lex_next(
          lex, (losu_syntax_tkvalue_t)(&((lex)->tkahead.info)));
      if (lex->tkahead.token == TOKEN_ASSIGN) {
        lex->tk.token = TOKEN_NAME;
        lex->tk.info.s = losu_objstring_new(lex->vm, "global");
        __losu_syntaxpar_statname(lex);
      } else if (lex->tkahead.token == TOKEN_CLASS) {
        __losu_syntaxpar_next(lex);
        __losu_syntaxpar_statclass(lex, line, 1);
      } else if (lex->tkahead.token == TOKEN_NAME)
        __losu_syntaxpar_statglo(lex);
      else
        losu_syntax_error(lex, "invalid global statement");
      break;
    }
    case TOKEN_NAME: {
      __losu_syntaxpar_statname(lex);
      break;
    }
    case TOKEN_RETURN: {
      __losu_syntaxpar_statreturn(lex);
      return 1;
    }
    case TOKEN_BREAK: {
      __losu_syntaxpar_statbreak(lex);
      return 1;
    }
    case TOKEN_CONTINUE: {
      __losu_syntaxpar_statcontinue(lex);
      return 1;
    }
    case TOKEN_YIELD: {
      __losu_syntaxpar_statyield(lex);
      break;
    }
    case TOKEN_CLASS: {
      __losu_syntaxpar_statclass(lex, line, 0);

      break;
    }
    case TOKEN_IMPORT: {
      __losu_syntaxpar_statimport(lex);
      break;
    }
    case TOKEN_ASYNC: {
      __losu_syntaxpar_statasync(lex);
      break;
    }
    case TOKEN_MATCH: {
      __losu_syntaxpar_statmatch(lex, lex->linenumber);
      break;
    }
    case TOKEN_EXCEPT: {
      __losu_syntaxpar_statexcept(lex);
      break;
    }
    case TOKEN_RAISE: {
      __losu_syntaxpar_statraise(lex);
      break;
    }
    case ';': {
      __losu_syntaxpar_next(lex);
      break;
    }
    default: {
      losu_syntax_func_t func = lex->fs;
      losu_syntax_warning(lex, "raw experssion is not suggested");
      if (!__losu_syntaxpar_checkblock(lex->tk.token))
        __losu_syntaxpar_explist(lex);
      losu_syntax_codegenarg1(func, INS_RETURN, func->naloc);
      func->stacklevel = func->naloc;
      return 1;
    }
  }
  return 0;
}

/* if while until */
#if 1
static void __losu_syntaxpar_statif(losu_syntax_lex_t lex, int32_t line) {
#define condition(lex, v)                      \
  {                                            \
    __losu_syntaxpar_subexp((lex), (v), -1);   \
    losu_syntax_binder_goT((lex)->fs, (v), 0); \
  }

#define then(lex, v)                    \
  {                                     \
    __losu_syntaxpar_next((lex));       \
    condition((lex), (v));              \
    __losu_syntaxpar_check((lex), ':'); \
    __losu_syntaxpar_statblock((lex));  \
  }

  losu_syntax_func_t func = lex->fs;
  losu_syntax_exp v = {0};
  int32_t el = NO_JUMP;
  then(lex, &v);
  while (lex->tk.token == TOKEN_PASS) {
    __losu_syntaxpar_next(lex);
    if (lex->tk.token == TOKEN_ELSEIF) {
      losu_syntax_binder_concat(func, &el, losu_syntax_binder_jump(func));
      losu_syntax_binder_palist(func, v.value._bool.f,
                                losu_syntax_binder_getL(func));
      then(lex, &v);
    } else if (lex->tk.token == TOKEN_ELSE) {
      // lex->tkahead.token = losu_syntax_lex_next(
      //     lex, (losu_syntax_tkvalue_t)(&(lex->tkahead.info)));
      // if (lex->tkahead.token == ':')
      //   __losu_syntaxpar_next(lex);
      __losu_syntaxpar_next(lex);
      __losu_syntaxpar_check(lex, ':');

      losu_syntax_binder_concat(func, &el, losu_syntax_binder_jump(func));
      losu_syntax_binder_palist(func, v.value._bool.f,
                                losu_syntax_binder_getL(func));
      // __losu_syntaxpar_next(lex);
      __losu_syntaxpar_statblock(lex);
      losu_syntax_binder_palist(func, el, losu_syntax_binder_getL(func));
      __losu_syntaxpar_checkmatch(lex, TOKEN_IF, TOKEN_PASS, line);
      break;
    } else {
      losu_syntax_binder_concat(func, &el, v.value._bool.f);
      losu_syntax_binder_palist(func, el, losu_syntax_binder_getL(func));
      break;
    }
  }

#undef condition
#undef then
}

static void __losu_syntaxpar_statwhile(losu_syntax_lex_t lex, int32_t line) {
#define condtion(lex, v)                       \
  {                                            \
    __losu_syntaxpar_subexp((lex), (v), -1);   \
    losu_syntax_binder_goT((lex)->fs, (v), 0); \
  }

  losu_syntax_func_t func = lex->fs;
  losu_syntax_exp v = {0};
  losu_syntax_break br = {0};
  losu_syntax_continue ctn = {0};
  int32_t with = losu_syntax_binder_getL(func);
  __losu_syntaxpar_enterbreak(func, &br);
  __losu_syntaxpar_entercontinue(func, &ctn, with);
  __losu_syntaxpar_next(lex);
  condtion(lex, &v);
  __losu_syntaxpar_check(lex, ':');
  __losu_syntaxpar_statblock(lex);
  losu_syntax_binder_palist(func, losu_syntax_binder_jump(func), with);
  losu_syntax_binder_palist(func, v.value._bool.f,
                            losu_syntax_binder_getL(func));
  __losu_syntaxpar_checkmatch(lex, TOKEN_WHILE, TOKEN_PASS, line);
  __losu_syntaxpar_exitbreak(func, &br);
  __losu_syntaxpar_exitcontinue(func, &ctn);

#undef condtion
}

static void __losu_syntaxpar_statuntil(losu_syntax_lex_t lex, int32_t line) {
#define condtion(lex, v)                       \
  {                                            \
    __losu_syntaxpar_subexp((lex), (v), -1);   \
    losu_syntax_binder_goT((lex)->fs, (v), 0); \
  }

  losu_syntax_func_t func = lex->fs;
  losu_syntax_exp v = {0};
  losu_syntax_break br = {0};
  losu_syntax_continue ctn = {0};
  int32_t until = losu_syntax_binder_getL(func);
  __losu_syntaxpar_enterbreak(func, &br);
  __losu_syntaxpar_entercontinue(func, &ctn, until);
  __losu_syntaxpar_next(lex);
  condtion(lex, &v);
  __losu_syntaxpar_check(lex, ':');
  __losu_syntaxpar_statblock(lex);
  losu_syntax_binder_palist(func, losu_syntax_binder_jump(func), until);
  losu_syntax_binder_palist(func, v.value._bool.t,
                            losu_syntax_binder_getL(func));
  __losu_syntaxpar_checkmatch(lex, TOKEN_UNTIL, TOKEN_PASS, line);
  __losu_syntaxpar_exitbreak(func, &br);
  __losu_syntaxpar_exitcontinue(func, &ctn);

#undef condtion
}
#endif

/* for */
#if 1
static void __losu_syntaxpar_statfor(losu_syntax_lex_t lex, int32_t line) {
  losu_syntax_func_t func = lex->fs;
  losu_object_string_t name = NULL;
  losu_syntax_break br = {0};

  __losu_syntaxpar_enterbreak(func, &br);

  __losu_syntaxpar_next(lex);
  name = __losu_syntaxpar_checkname(lex);
  switch (lex->tk.token) {
    case ',': {
      __losu_syntaxpar_statforobj(lex, name);
      break;
    }
    case TOKEN_NAME: {
      if (strcmp("in", lex->tk.info.s->str) == 0) {
        __losu_syntaxpar_statfornum(lex, name);
        break;
      }
      losu_syntax_error(lex, "invalid 'for' statement");
      break;
    }
    default: {
      losu_syntax_error(lex, "invalid 'for' statement");
      break;
    }
  }
  __losu_syntaxpar_checkmatch(lex, TOKEN_FOR, TOKEN_PASS, line);
  __losu_syntaxpar_exitbreak(func, &br);
}

/* for k,v in obj */
static void __losu_syntaxpar_statforobj(losu_syntax_lex_t lex,
                                        losu_object_string_t name) {
  losu_object_string_t val;
  __losu_syntaxpar_next(lex);
  val = __losu_syntaxpar_checkname(lex);
  __losu_syntaxpar_checkcondition(
      lex,
      (lex->tk.token == TOKEN_NAME && !strncmp(lex->tk.info.s->str, "in\0", 3)),
      "invalid 'for' statement");
  __losu_syntaxpar_next(lex);
  __losu_syntaxpar_exp(lex);
  __losu_syntaxpar_newlcvar(lex, losu_objstring_newconst(lex->vm, "__THIS__"),
                            0);
  __losu_syntaxpar_newlcvar(lex, name, 1);
  __losu_syntaxpar_newlcvar(lex, val, 2);
  __losu_syntaxpar_statforbody(lex, 1);
}

/* for i in x,y,z */
static void __losu_syntaxpar_statfornum(losu_syntax_lex_t lex,
                                        losu_object_string_t name) {
  __losu_syntaxpar_next(lex);
  __losu_syntaxpar_exp(lex);
  __losu_syntaxpar_check(lex, ',');
  __losu_syntaxpar_exp(lex);
  if (lex->tk.token == ',') {
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_exp(lex);
  } else
    losu_syntax_codegenarg1(lex->fs, INS_PUSHNUM,
                            losu_syntax_binder_number(lex->fs, 1));

  __losu_syntaxpar_newlcvar(lex, name, 0);
  __losu_syntaxpar_newlcvar(lex, losu_objstring_newconst(lex->vm, "__MAX__"),
                            1);
  __losu_syntaxpar_newlcvar(lex, losu_objstring_newconst(lex->vm, "__STEP__"),
                            2);
  __losu_syntaxpar_statforbody(lex, 0);
}

static void __losu_syntaxpar_statforbody(losu_syntax_lex_t lex,
                                         losu_ctype_bool isObj) {
  struct {
    _losu_vmins_OP pre;
    _losu_vmins_OP loop;
  } const _inline_ins[] = {
      {INS_FORPREP, INS_FORNEXT},
      {INS_FORINPREP, INS_FORINNEXT},
  };

  losu_syntax_continue ctn = {0};
  losu_syntax_func_t func = lex->fs;

  int32_t prep = losu_syntax_codegenarg1(func, _inline_ins[isObj].pre, NO_JUMP);
  __losu_syntaxpar_entercontinue(func, &ctn, 0);
  int32_t block = losu_syntax_binder_getL(func);
  __losu_syntaxpar_check(lex, ':');
  __losu_syntaxpar_adlcvar(lex, 3);
  __losu_syntaxpar_statblock(lex);
  int32_t endpc =
      losu_syntax_codegenarg1(func, _inline_ins[isObj].loop, NO_JUMP);
  losu_syntax_binder_palist(func, endpc, block);
  losu_syntax_binder_palist(func, prep, losu_syntax_binder_getL(func));
  /* fixed continue */
  int32_t tmppc = func->contlist->tmppc;
  if (tmppc != NO_JUMP)
    _losu_vmcg_setS(func->fcode->code[tmppc], endpc - tmppc - 1);

  __losu_syntaxpar_exitcontinue(func, &ctn);
  __losu_syntaxpar_removelcvar(lex, 3);
}
#endif

/* def */
#if 1
static void __losu_syntaxpar_statfunc(losu_syntax_lex_t lex, int32_t line) {
  losu_syntax_exp v = {0};
  __losu_syntaxpar_next(lex);
  __losu_syntaxpar_statfunc_name(lex, &v);
  __losu_syntaxpar_statfunc_body(lex, line);
  losu_syntax_binder_setvar(lex, &v);
}

static void __losu_syntaxpar_statfunc_name(losu_syntax_lex_t lex,
                                           losu_syntax_exp_t v) {
  // losu_ctype_bool i = 0;
  losu_object_string_t name = __losu_syntaxpar_checkname(lex);
  __losu_syntaxpar_firstname(lex, name, v, __losu_syntaxpar_func_varlevel);
}

static void __losu_syntaxpar_statfunc_body(losu_syntax_lex_t lex,
                                           int32_t line) {
  losu_syntax_func nfs = {0};
  __losu_syntaxpar_newfunc(lex, &nfs);
  __losu_syntaxpar_check(lex, '(');
  __losu_syntaxpar_statfunc_arg(lex);
  __losu_syntaxpar_check(lex, ')');
  __losu_syntaxpar_check(lex, ':');
  while (!__losu_syntaxpar_checkblock(lex->tk.token))
    if (__losu_syntaxpar_stat(lex))
      break;
  __losu_syntaxpar_checkmatch(lex, TOKEN_DEF, TOKEN_PASS, line);
  __losu_syntaxpar_delfunc(lex);

  losu_syntax_func_t func = lex->fs;
  losu_object_scode_t f = func->fcode;
  for (int32_t i = 0; i < nfs.nclosure; i++)
    losu_syntax_binder_tostack(lex, &(nfs.closure[i]), 1);
  losu_mem_growvector(lex->vm, f->lcscode, f->nlcscode, 1, losu_object_scode_t,
                      _losu_vmins_maxA, " 'function' overflow ");

  f->lcscode[f->nlcscode++] = nfs.fcode;
  losu_syntax_codegenarg2(func, INS_PUSHFUNCTION, f->nlcscode - 1,
                          nfs.nclosure);
}

static void __losu_syntaxpar_statfunc_arg(losu_syntax_lex_t lex) {
  losu_ctype_bool isV = 0;
  int32_t np = 0;
  losu_syntax_func_t func = lex->fs;
  if (lex->tk.token != ')') {
    do {
      switch (lex->tk.token) {
        case TOKEN_ARG: {
          __losu_syntaxpar_next(lex);
          isV = 1;
          break;
        }
        case TOKEN_NAME: {
          __losu_syntaxpar_newlcvar(lex, __losu_syntaxpar_checkname(lex), np++);
          break;
        }
        default: {
          losu_syntax_error(lex, "invalid args of 'def'");
          break;
        }
      }
    } while (((lex->tk.token == ',') ? (__losu_syntaxpar_next(lex), 1) : 0) &&
             !isV);
  }
  __losu_syntaxpar_adlcvar(lex, np);
  func->fcode->narg = func->naloc;
  func->fcode->isVarg = isV;
  if (isV) {
    if (lex->tk.token != ')')
      losu_syntax_error(lex, "after '...' must be ')'");
    __losu_syntaxpar_newlcvar(lex, losu_objstring_newconst(lex->vm, "_"), 0);
    __losu_syntaxpar_adlcvar(lex, 1);
  }
  losu_syntax_codegen_dtstack(func, func->naloc);
}

/**
 * @brief get var level of def [name]
 * @param lex the lexer
 * @param n name
 * @param v exp handle
 * @return -1 is global, 0 is local, >0 is upvalue
 */
static int32_t __losu_syntaxpar_func_varlevel(losu_syntax_lex_t lex,
                                              losu_object_string_t n,
                                              losu_syntax_exp_t v) {
  // losu_syntax_func_t func = lex->fs;
  /* is local */
  // for (int32_t i = func->naloc - 1; i >= 0; i--) {
  //   if (n == func->fcode->localvar[func->aloc[i]].name) {
  //     v->type = losu_syntax_exptype_local;
  //     v->value.index = i;
  //     return 0;
  //   }
  // }
  /* global */
  v->type = losu_syntax_exptype_global;
  __losu_syntaxpar_newglovar(lex, n);
  return -1;
}

#endif

/* lamba */
#if 1
static void __losu_syntaxpar_statlambda(losu_syntax_lex_t lex) {
  losu_syntax_func nfs = {0};
  __losu_syntaxpar_newfunc(lex, &nfs);
  __losu_syntaxpar_statlambda_arg(lex);
  __losu_syntaxpar_check(lex, ':');
  __losu_syntaxpar_statlambda_body(lex);
  __losu_syntaxpar_delfunc(lex);

  losu_syntax_func_t func = lex->fs;
  losu_object_scode_t f = func->fcode;
  for (int32_t i = 0; i < nfs.nclosure; i++)
    losu_syntax_binder_tostack(lex, &(nfs.closure[i]), 1);
  losu_mem_growvector(lex->vm, f->lcscode, f->nlcscode, 1, losu_object_scode_t,
                      _losu_vmins_maxA, " 'lambda' overflow ");

  f->lcscode[f->nlcscode++] = nfs.fcode;
  losu_syntax_codegenarg2(func, INS_PUSHFUNCTION, f->nlcscode - 1,
                          nfs.nclosure);
}

static void __losu_syntaxpar_statlambda_arg(losu_syntax_lex_t lex) {
  losu_ctype_bool isV = 0;
  int32_t np = 0;
  losu_syntax_func_t func = lex->fs;
  if (lex->tk.token != ':') {
    do {
      switch (lex->tk.token) {
        case TOKEN_ARG: {
          __losu_syntaxpar_next(lex);
          isV = 1;
          break;
        }
        case TOKEN_NAME: {
          __losu_syntaxpar_newlcvar(lex, __losu_syntaxpar_checkname(lex), np++);
          break;
        }
        default: {
          break;
        }
      }
    } while (((lex->tk.token == ',') ? (__losu_syntaxpar_next(lex), 1) : 0) &&
             !isV);
  }
  __losu_syntaxpar_adlcvar(lex, np);
  func->fcode->narg = func->naloc;
  func->fcode->isVarg = isV;
  if (isV) {
    if (lex->tk.token != ':')
      losu_syntax_error(lex, "after '...' must be ':'");
    __losu_syntaxpar_newlcvar(lex, losu_objstring_newconst(lex->vm, "_"), 0);
    __losu_syntaxpar_adlcvar(lex, 1);
  }
  losu_syntax_codegen_dtstack(func, func->naloc);
}

static void __losu_syntaxpar_statlambda_body(losu_syntax_lex_t lex) {
  losu_syntax_exp v = {0};
  __losu_syntaxpar_subexp(lex, &v, -1);
  losu_syntax_binder_tostack(lex, &v, 0);
  losu_syntax_codegenarg1(lex->fs, INS_RETURN, lex->fs->naloc);
  lex->fs->stacklevel = lex->fs->naloc;
}
#endif

/* let global */
#if 1
static void __losu_syntaxpar_statlet(losu_syntax_lex_t lex) {
#define isSet(lex, c) \
  ((lex->tk.token == c) ? (__losu_syntaxpar_next(lex), 1) : 0)

  int32_t nv = 0, ne = 0;
  do {
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_newlcvar(lex, __losu_syntaxpar_checkname(lex), nv++);
  } while (lex->tk.token == ',');
  if (isSet(lex, '='))
    ne = __losu_syntaxpar_explist(lex);
  else
    ne = 0;
  __losu_syntaxpar_adstack(lex, nv, ne);
  __losu_syntaxpar_adlcvar(lex, nv);

#undef isSet
}

static void __losu_syntaxpar_statglo(losu_syntax_lex_t lex) {
  // int32_t nv = 0;
  do {
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_newglovar(lex, __losu_syntaxpar_checkname(lex));
  } while (lex->tk.token == ',');
}

#endif

/* name */
#if 1
static void __losu_syntaxpar_statname(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  losu_syntax_exp v = {0};

  __losu_syntaxpar_expvarfunc(lex, &v, __losu_syntaxpar_exp_varlevel, 0);
  // printf("@\n");
  if (v.type == losu_syntax_exptype_expr) {
    __losu_syntaxpar_checkcondition(lex, losu_syntax_binder_isopen(func),
                                    " invalid exprssion of 'name'");
    losu_syntax_binder_setcallreturn(func, 0);
  } else {
    losu_syntax_binder_adstack(func, __losu_syntaxpar_statnamelist(lex, &v, 1));
  }
}

static int32_t __losu_syntaxpar_statnamelist(losu_syntax_lex_t lex,
                                             losu_syntax_exp_t v,
                                             int32_t nvar) {
  int32_t left = 0;
  if (lex->tk.token == ',') {
    losu_syntax_exp nv = {0};
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_expvarfunc(lex, &nv, __losu_syntaxpar_exp_varlevel, 0);
    __losu_syntaxpar_checkcondition(lex, (nv.type != losu_syntax_exptype_expr),
                                    " invalid exprssion of 'name'");
    left = __losu_syntaxpar_statnamelist(lex, &nv, nvar + 1);
  } else {
    __losu_syntaxpar_check(lex, '=');
    __losu_syntaxpar_adstack(lex, nvar, __losu_syntaxpar_explist(lex));
  }
  if (v->type != losu_syntax_exptype_index)
    losu_syntax_binder_setvar(lex, v);
  else {
    losu_syntax_codegenarg2(lex->fs, INS_SETUNIT, left + nvar + 2, 1);
    left += 2;
  }
  return left;
}

#endif

/* match */
#if 1
static void __losu_syntaxpar_statmatch(losu_syntax_lex_t lex, int32_t line) {
#define then(lex, v)                                     \
  {                                                      \
    __losu_syntaxpar_next(lex);                          \
    __losu_syntaxpar_subexp((lex), (v), -1);             \
    losu_syntax_binder_go(((lex)->fs), v, 1, INS_JMPNE); \
    __losu_syntaxpar_check((lex), ':');                  \
    __losu_syntaxpar_statblock((lex));                   \
  }

  losu_syntax_func_t func = lex->fs;
  losu_syntax_exp v = {0};
  int32_t el = NO_JUMP;
  losu_ctype_bool is = 0;
  // losu_syntax_codegenarg0(func, INS_TEST);
  __losu_syntaxpar_next(lex);
  __losu_syntaxpar_exp(lex);
  __losu_syntaxpar_newlcvar(lex, losu_objstring_new(lex->vm, ""), 0);
  __losu_syntaxpar_adlcvar(lex, 1);
  __losu_syntaxpar_check(lex, ':');
  while (1) {
    if (lex->tk.token == TOKEN_CASE) {
      int32_t case_line = lex->linenumber;
      if (is == 0) {
        is = 1;
        losu_syntax_codegenarg0(func, INS_COPY);
        then(lex, &v);
        __losu_syntaxpar_checkmatch(lex, TOKEN_CASE, TOKEN_PASS, case_line);
      } else {
        losu_syntax_binder_concat(func, &el, losu_syntax_binder_jump(func));
        losu_syntax_binder_palist(func, v.value._bool.f,
                                  losu_syntax_binder_getL(func));
        losu_syntax_codegenarg0(func, INS_COPY);
        then(lex, &v);
        __losu_syntaxpar_checkmatch(lex, TOKEN_CASE, TOKEN_PASS, case_line);
      }
    } else if (lex->tk.token == TOKEN_ELSE) {
      if (is == 0)
        losu_syntax_error(lex, "no case before 'else'");
      int32_t else_line = lex->linenumber;
      __losu_syntaxpar_next(lex);
      __losu_syntaxpar_check(lex, ':');

      losu_syntax_binder_concat(func, &el, losu_syntax_binder_jump(func));
      losu_syntax_binder_palist(func, v.value._bool.f,
                                losu_syntax_binder_getL(func));
      // __losu_syntaxpar_next(lex);
      __losu_syntaxpar_statblock(lex);
      losu_syntax_binder_palist(func, el, losu_syntax_binder_getL(func));
      __losu_syntaxpar_checkmatch(lex, TOKEN_ELSE, TOKEN_PASS, else_line);
      break;
    } else {
      losu_syntax_binder_concat(func, &el, v.value._bool.f);
      losu_syntax_binder_palist(func, el, losu_syntax_binder_getL(func));
      break;
    }
  }
  losu_syntax_codegenarg1(lex->fs, INS_POP, 1);
  __losu_syntaxpar_removelcvar(lex, 1);
  __losu_syntaxpar_checkmatch(lex, TOKEN_MATCH, TOKEN_PASS, line);
  // losu_syntax_codegenarg0(func, INS_TEST);

#undef then
}
#endif

/* except ... assert */
#if 1
static void __losu_syntaxpar_statexcept(losu_syntax_lex_t lex) {
  // losu_syntax_codegenarg0(lex->fs, INS_TEST);
  int32_t line = lex->linenumber;
  losu_syntax_func_t func = lex->fs;
  __losu_syntaxpar_next(lex);
  // lambda except
  /* losu_syntax_func nfs = {0};
  __losu_syntaxpar_newfunc(lex, &nfs);
  __losu_syntaxpar_statlambda_arg(lex);
  __losu_syntaxpar_check(lex, ':');
  while (!__losu_syntaxpar_checkblock(lex->tk.token))
    if (__losu_syntaxpar_stat(lex))
      break;
  __losu_syntaxpar_checkmatch(lex, TOKEN_EXCEPT, TOKEN_PASS, line);
  __losu_syntaxpar_delfunc(lex);

  losu_syntax_func_t func = lex->fs;
  losu_object_scode_t f = func->fcode;
  for (int32_t i = 0; i < nfs.nclosure; i++)
    losu_syntax_binder_tostack(lex, &(nfs.closure[i]), 1);
  losu_mem_growvector(lex->vm, f->lcscode, f->nlcscode, 1, losu_object_scode_t,
                      _losu_vmins_maxA, " 'except' overflow ");

  f->lcscode[f->nlcscode++] = nfs.fcode;
  losu_syntax_codegenarg2(func, INS_PUSHFUNCTION, f->nlcscode - 1,
                          nfs.nclosure); */
  __losu_syntaxpar_exp(lex);
  __losu_syntaxpar_newlcvar(lex, losu_objstring_new(lex->vm, "__raise__"), 0);
  __losu_syntaxpar_adlcvar(lex, 1);
  __losu_syntaxpar_check(lex, ':');
  // set tmppc
  losu_syntax_raise rlabel = {0};
  // losu_syntax_codegenarg0(lex->fs, INS_TEST);
  losu_syntax_codegenarg1(func, INS_JMP, 1);
  int32_t tmppc = losu_syntax_codegenarg1(func, INS_JMP, 1);
  __losu_syntaxpar_enterraise(func, &rlabel, tmppc);
  __losu_syntaxpar_statblock(lex);
  __losu_syntaxpar_checkmatch(lex, TOKEN_EXCEPT, TOKEN_PASS, line);

  losu_syntax_codegenarg1(func, INS_POP, 1);
  __losu_syntaxpar_removelcvar(lex, 1);
  __losu_syntaxpar_exitraise(func, &rlabel);
  // set pc;
  _losu_vmcg_setS(func->fcode->code[tmppc], func->pc - tmppc - 1);
  // printf("%d\n", func->pc - tmppc - 1);
  // losu_syntax_codegenarg0(lex->fs, INS_TEST);
}

static void __losu_syntaxpar_statraise(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  losu_syntax_raise_t r = func->raiselist;
  int32_t c = func->stacklevel;
  __losu_syntaxpar_next(lex);
  if (r) {
    losu_syntax_binder_adstack(func, c - r->stacklevel);
    __losu_syntaxpar_explist(lex);
    losu_syntax_codegenarg2(func, INS_CALL, r->stacklevel - 1, 0);
    // losu_syntax_codegenarg1(func, INS_TEST, 1);
    losu_syntax_codegenarg1(func, INS_JMP, r->startpc - func->pc - 1);
    func->stacklevel = c;
  } else
    losu_syntax_error(lex, "'raise' outside 'assert'");
}

#endif

/* return break continue yield block */
#if 1
static void __losu_syntaxpar_statreturn(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  __losu_syntaxpar_next(lex);
  if (!__losu_syntaxpar_checkblock(lex->tk.token))
    __losu_syntaxpar_explist(lex);
  losu_syntax_codegenarg1(func, INS_RETURN, func->naloc);
  func->stacklevel = func->naloc;
}

static void __losu_syntaxpar_statbreak(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  losu_syntax_break_t br = func->breaklist;
  int32_t c = func->stacklevel;
  __losu_syntaxpar_next(lex);
  if (br) {
    losu_syntax_binder_adstack(func, c - br->stacklevel);
    losu_syntax_binder_concat(func, &br->breaklist,
                              losu_syntax_binder_jump(func));
    func->stacklevel = c;
  } else
    losu_syntax_error(lex, "'break' outside loop");
}

static void __losu_syntaxpar_statcontinue(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  losu_syntax_continue_t ctn = func->contlist;
  int32_t c = func->stacklevel;
  __losu_syntaxpar_next(lex);
  if (ctn) {
    losu_syntax_binder_adstack(func, c - ctn->stacklevel);
    if (ctn->tmppc == NO_JUMP)
      ctn->tmppc =
          losu_syntax_codegenarg1(func, INS_JMP, ctn->startpos - func->pc - 1);
    else
      losu_syntax_codegenarg1(func, INS_JMP, ctn->tmppc - func->pc - 1);
    func->stacklevel = c;
  } else
    losu_syntax_error(lex, "'continue' outside loop");
}

static void __losu_syntaxpar_statyield(losu_syntax_lex_t lex) {
  __losu_syntaxpar_next(lex);
  losu_syntax_codegenarg0(lex->fs, INS_YIELD);
}

static void __losu_syntaxpar_statblock(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  int32_t naloc = func->naloc;
  losu_ctype_bool is = 0;
  while (!is && !__losu_syntaxpar_checkblock(lex->tk.token))
    is = __losu_syntaxpar_stat(lex);
  losu_syntax_binder_adstack(func, func->naloc - naloc);
  __losu_syntaxpar_removelcvar(lex, func->naloc - naloc);
}
#endif

/* class */
static void __losu_syntaxpar_statclass(losu_syntax_lex_t lex,
                                       int32_t line,
                                       losu_ctype_bool isGlobal) {
  losu_syntax_exp v = {0};
  losu_syntax_func_t func = lex->fs;
  int32_t niss = 0;
  losu_syntax_token tk = lex->tk;
  __losu_syntaxpar_next(lex);
  losu_object_string_t name = __losu_syntaxpar_checkname(lex);
  if (!isGlobal)
    __losu_syntaxpar_firstname(lex, name, &v, __losu_syntaxpar_class_varlevel);
  else
    __losu_syntaxpar_firstname(lex, name, &v,
                               __losu_syntaxpar_class_global_varlevel);

  int32_t pc = losu_syntax_codegenarg2(func, INS_CREATEUNIT, 0, 0);
  if (lex->tk.token == ':')
    __losu_syntaxpar_next(lex);
  else if (lex->tk.token == '(') {
    __losu_syntaxpar_next(lex);
    losu_syntax_codegenarg1(
        func, INS_PUSHSTRING,
        losu_syntax_binder_strconst(func,
                                    losu_objstring_new(lex->vm, "__super__")));
    __losu_syntaxpar_exp(lex);
    niss++;
    if (niss % _losu_vmlim_maxsetmap == 0)
      losu_syntax_codegenarg1(func, INS_SETMAP, _losu_vmlim_maxsetmap);
    __losu_syntaxpar_check(lex, ')');
    __losu_syntaxpar_check(lex, ':');
  } else
    __losu_syntaxpar_check(lex, ':');

  // if (lex->tk.token != TOKEN_PASS) {
  if (lex->tk.token == TOKEN_PASS)
    losu_syntax_error(lex, "empty class is not allowed");
  while (lex->tk.token != TOKEN_PASS) {
    switch (lex->tk.token) {
      case TOKEN_NAME: {
        losu_syntax_codegenarg1(
            func, INS_PUSHSTRING,
            losu_syntax_binder_strconst(func, __losu_syntaxpar_checkname(lex)));
        __losu_syntaxpar_check(lex, '=');
        __losu_syntaxpar_exp(lex);
        break;
      }
      case TOKEN_DEF: {
        __losu_syntaxpar_next(lex);
        losu_syntax_codegenarg1(
            func, INS_PUSHSTRING,
            losu_syntax_binder_strconst(func, __losu_syntaxpar_checkname(lex)));

        __losu_syntaxpar_statfunc_body(lex, lex->linenumber);

        break;
      }
      default: {
        losu_syntax_error(lex, "invalid class field");
        break;
      }
    }
    niss++;
    if (niss % _losu_vmlim_maxsetmap == 0)
      losu_syntax_codegenarg1(func, INS_SETMAP, _losu_vmlim_maxsetmap);
  }
  losu_syntax_codegenarg1(func, INS_SETMAP, niss % _losu_vmlim_maxsetmap);
  // }
  __losu_syntaxpar_checkmatch(lex, tk.token, TOKEN_PASS, line);
  _losu_vmcg_setA(func->fcode->code[pc], niss);
  _losu_vmcg_setB(func->fcode->code[pc], obj_unittype_class);

  losu_syntax_binder_setvar(lex, &v);
}
static int32_t __losu_syntaxpar_class_varlevel(losu_syntax_lex_t lex,
                                               losu_object_string_t n,
                                               losu_syntax_exp_t v) {
  losu_syntax_func_t func = lex->fs;
  /* is local */
  for (int32_t i = func->naloc - 1; i >= 0; i--) {
    if (n == func->fcode->localvar[func->aloc[i]].name) {
      v->type = losu_syntax_exptype_local;
      v->value.index = i;
      return 0;
    }
  }
  /* new local */
  __losu_syntaxpar_newlcvar(lex, n, 0);
  __losu_syntaxpar_adstack(lex, 1, 0);
  __losu_syntaxpar_adlcvar(lex, 1);
  return __losu_syntaxpar_class_varlevel(lex, n, v);
}

static int32_t __losu_syntaxpar_class_global_varlevel(losu_syntax_lex_t lex,
                                                      losu_object_string_t n,
                                                      losu_syntax_exp_t v) {
  v->type = losu_syntax_exptype_global;
  __losu_syntaxpar_newglovar(lex, n);
  return -1;
}

/* import */
static void __losu_syntaxpar_statimport(losu_syntax_lex_t lex) {
  __losu_syntaxpar_next(lex);
  losu_object_string_t modname = __losu_syntaxpar_checkname(lex);
  losu_syntax_codegenarg1(lex->fs, INS_IMPORT,
                          losu_syntax_binder_strconst(lex->fs, modname));
  __losu_syntaxpar_newglovar(lex, modname);
}

/* async */
static void __losu_syntaxpar_statasync(losu_syntax_lex_t lex) {
  losu_syntax_exp v = {0};
  __losu_syntaxpar_next(lex);
  __losu_syntaxpar_expvarfunc(lex, &v, __losu_syntaxpar_exp_varlevel, 1);
}

#endif

/* experssion */
#if 1
static int32_t __losu_syntaxpar_explist(losu_syntax_lex_t lex) {
  int32_t n = 1;
  losu_syntax_exp v = {0};
  __losu_syntaxpar_subexp(lex, &v, -1);
  while (lex->tk.token == ',') {
    losu_syntax_binder_tostack(lex, &v, 1);
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_subexp(lex, &v, -1);
    n++;
  }
  losu_syntax_binder_tostack(lex, &v, 0);
  return n;
}

static void __losu_syntaxpar_exp(losu_syntax_lex_t lex) {
  losu_syntax_exp v = {0};
  __losu_syntaxpar_subexp(lex, &v, -1);
  losu_syntax_binder_tostack(lex, &v, 1);
}

static int32_t __losu_syntaxpar_subexp(losu_syntax_lex_t lex,
                                       losu_syntax_exp_t v,
                                       int32_t left) {
  int32_t op, uop;
  struct {
    char left;
    char right;
  } const priority[] = {
      {5, 5}, /* + */
      {5, 5}, /* - */
      {6, 6}, /* * */
      {6, 6}, /* / */
      {9, 8}, /* ** */
      {6, 6}, /* % */
      {5, 5}, /* & */

      {2, 2}, /* != */
      {2, 2}, /* == */

      {2, 2}, /* < */
      {2, 2}, /* <= */
      {2, 2}, /* > */
      {2, 2}, /* >= */
      {0, 0}, /* |> */

      {1, 1}, /* and */
      {1, 1}, /* or */
  };
  uop = __losu_syntaxpar_subexp_unop(lex->tk.token);
  if (uop != INS_UNOP_NULL) {
    __losu_syntaxpar_next(lex);
    __losu_syntaxpar_subexp(lex, v, 7);
    losu_syntax_binder_prefix(lex, uop, v);
  } else
    __losu_syntaxpar_subexp_stat(lex, v);
  op = __losu_syntaxpar_subexp_binop(lex->tk.token);
  while (op != INS_BINOP_NULL && priority[op].left > left) {
    losu_syntax_exp v2 = {0};
    int32_t l2;
    __losu_syntaxpar_next(lex);
    losu_syntax_binder_infix(lex, op, v);
    l2 = __losu_syntaxpar_subexp(lex, &v2, priority[op].right);
    losu_syntax_binder_posfix(lex, op, v, &v2);
    op = l2;
  }
  return op;
}

static int32_t __losu_syntaxpar_subexp_unop(int16_t tk) {
  switch (tk) {
    case TOKEN_NOT:
      return INS_UNOP_NOT;
    case '-':
      return INS_UNOP_NEG;
    default:
      return INS_UNOP_NULL;
  }
}

static int32_t __losu_syntaxpar_subexp_binop(int16_t tk) {
  switch (tk) {
    case '+':
      return INS_BINOP_ADD;
    case '-':
      return INS_BINOP_SUB;
    case '*':
      return INS_BINOP_MULT;
    case '/':
      return INS_BINOP_DIV;
    case TOKEN_POW:
      return INS_BINOP_POW;
    case '%':
      return INS_BINOP_MOD;
    case '&':
      return INS_BINOP_CONCAT;
    case TOKEN_EQ:
      return INS_BINOP_EQ;
    case TOKEN_NE:
      return INS_BINOP_NE;
    case '<':
      return INS_BINOP_LT;
    case TOKEN_LE:
      return INS_BINOP_LE;
    case '>':
      return INS_BINOP_GT;
    case TOKEN_GE:
      return INS_BINOP_GE;
    case TOKEN_AND:
      return INS_BINOP_AND;
    case TOKEN_OR:
      return INS_BINOP_OR;
    case TOKEN_PIPE:
      return INS_BINOP_PIPE;
    default:
      return INS_BINOP_NULL;
  }
}

static void __losu_syntaxpar_subexp_stat(losu_syntax_lex_t lex,
                                         losu_syntax_exp_t v) {
  losu_syntax_func_t func = lex->fs;
  switch (lex->tk.token) {
    case TOKEN_NUMBER: {
      losu_ctype_number n = lex->tk.info.num;
      __losu_syntaxpar_next(lex);
      losu_syntax_codegenarg1(func, INS_PUSHNUM,
                              losu_syntax_binder_number(func, n));
      break;
    }
    case TOKEN_STRING: {
      losu_syntax_codegenarg1(
          func, INS_PUSHSTRING,
          losu_syntax_binder_strconst(func, lex->tk.info.s));
      __losu_syntaxpar_next(lex);
      break;
    }

    case TOKEN_TRUE: {
      losu_syntax_codegenarg1(func, INS_PUSHTRUE, 1);
      __losu_syntaxpar_next(lex);
      break;
    }
    case TOKEN_FALSE: {
      losu_syntax_codegenarg1(func, INS_PUSHNULL, 1);
      __losu_syntaxpar_next(lex);
      break;
    }
    case TOKEN_LAMBDA: {
      __losu_syntaxpar_next(lex);
      if (lex->tk.token == TOKEN_DEF) {
        __losu_syntaxpar_next(lex);
        __losu_syntaxpar_statfunc_body(lex, lex->linenumber);
      } else
        __losu_syntaxpar_statlambda(lex);
      break;
    }
    case '{': {
      __losu_syntaxpar_exp_unitmap(lex);
      break;
    }
    case '[': {
      __losu_syntaxpar_exp_unitlist(lex);
      break;
    }
    case TOKEN_NAME: {
      __losu_syntaxpar_expvarfunc(lex, v, __losu_syntaxpar_exp_varlevel, 0);
      return;
    }
    case TOKEN_GLOBAL: {
      lex->tk.token = TOKEN_NAME;
      lex->tk.info.s = losu_objstring_new(lex->vm, "global");
      __losu_syntaxpar_expvarfunc(lex, v, __losu_syntaxpar_exp_varlevel, 0);
      return;
    }
    case '(': {
      __losu_syntaxpar_next(lex);
      __losu_syntaxpar_subexp(lex, v, -1);
      __losu_syntaxpar_check(lex, ')');
      return;
    }
    default: {
      losu_syntax_error(lex, "invalid expression field");
      return;
    }
  }
  v->type = losu_syntax_exptype_expr;
  v->value._bool.t = v->value._bool.f = NO_JUMP;
}

static void __losu_syntaxpar_expvarfunc(losu_syntax_lex_t lex,
                                        losu_syntax_exp_t v,
                                        varnamelevel getlocal,
                                        losu_ctype_bool async) {
#define lookahead(lex)                                         \
  {                                                            \
    (lex)->tkahead.token = losu_syntax_lex_next(               \
        lex, (losu_syntax_tkvalue_t)(&((lex)->tkahead.info))); \
  }

  losu_ctype_bool needself = 0;
  __losu_syntaxpar_firstname(lex, __losu_syntaxpar_checkname(lex), v, getlocal);
  while (1) {
    switch (lex->tk.token) {
      case '.': {
        needself = 1;
        __losu_syntaxpar_next(lex);
        lookahead(lex);
        switch (lex->tkahead.token) {
          case '(':
          case '{':
          case TOKEN_STRING: {
            int32_t name = losu_syntax_binder_strconst(
                lex->fs, __losu_syntaxpar_checkname(lex));
            losu_syntax_binder_tostack(lex, v, 1);
            losu_syntax_codegenarg1(lex->fs, INS_PUSHSELF, name);
            __losu_syntaxpar_expFargs(lex, 1, async);
            v->type = losu_syntax_exptype_expr;
            v->value._bool.t = v->value._bool.f = NO_JUMP;
            if (async)
              return;
            break;
          }
          default: {
            losu_syntax_binder_tostack(lex, v, 1);
            losu_syntax_codegenarg1(
                lex->fs, INS_PUSHSTRING,
                losu_syntax_binder_strconst(lex->fs,
                                            __losu_syntaxpar_checkname(lex)));
            v->type = losu_syntax_exptype_index;
            break;
          }
        }
        break;
      }
      case TOKEN_ASSIGN: {
        if (needself)
          losu_syntax_error(lex, "'::' cannot be followed by '.'");
        __losu_syntaxpar_next(lex);
        losu_syntax_binder_tostack(lex, v, 1);
        losu_syntax_codegenarg1(lex->fs, INS_PUSHSTRING,
                                losu_syntax_binder_strconst(
                                    lex->fs, __losu_syntaxpar_checkname(lex)));
        v->type = losu_syntax_exptype_index;
        break;
      }
      case '[': {
        __losu_syntaxpar_next(lex);
        losu_syntax_binder_tostack(lex, v, 1);
        __losu_syntaxpar_exp(lex);
        __losu_syntaxpar_check(lex, ']');
        v->type = losu_syntax_exptype_index;
        break;
      }
      case '(':
      case TOKEN_STRING:
      case '{': {
        losu_syntax_binder_tostack(lex, v, 1);
        __losu_syntaxpar_expFargs(lex, 0, async);
        v->type = losu_syntax_exptype_expr;
        v->value._bool.f = v->value._bool.t = NO_JUMP;
        if (async)
          return;
        break;
      }
      default:
        return;
    }
  }

#undef lookahead
}

static void __losu_syntaxpar_firstname(losu_syntax_lex_t lex,
                                       losu_object_string_t n,
                                       losu_syntax_exp_t var,
                                       varnamelevel getlocal) {
  int32_t l = getlocal(lex, n, var);
  if (l == -1)
    var->value.index = losu_syntax_binder_strconst(lex->fs, n);
  else if (l == 1) {
    losu_syntax_func_t func = lex->fs;
    int16_t c = -1;
    for (int16_t i = 0; i < func->nclosure; i++)
      if (func->closure[i].type == var->type &&
          func->closure[i].value.index == var->value.index) {
        c = i;
        break;
      }
    if (c < 0) {
      __losu_syntaxpar_checklimit(lex, func->nclosure, _losu_vmlim_maxclosure,
                                  "upvalue variable");
      func->closure[func->nclosure] = *var;
      c = func->nclosure++;
    }
    losu_syntax_codegenarg1(lex->fs, INS_PUSHUPVALUE, c);
    var->type = losu_syntax_exptype_expr;
    var->value._bool.f = var->value._bool.t = NO_JUMP;
  } else if (l > 1) {
    var->type = losu_syntax_exptype_global;
    var->value.index = losu_syntax_binder_strconst(lex->fs, n);
    losu_syntax_warning(lex,
                        "upvalue must be global or in neighboring scope. `%s` "
                        "will be treated as a global variable",
                        n->str);
  }
}

static void __losu_syntaxpar_expFargs(losu_syntax_lex_t lex,
                                      losu_ctype_bool slf,
                                      losu_ctype_bool async) {
  losu_syntax_func_t func = lex->fs;
  int32_t sl = func->stacklevel - slf - 1;
  int32_t line = lex->linenumber;
  switch (lex->tk.token) {
    case '(': {
      __losu_syntaxpar_next(lex);
      if (lex->tk.token != ')')
        __losu_syntaxpar_explist(lex);
      __losu_syntaxpar_checkmatch(lex, '(', ')', line);
      break;
    }
    case '{': {
      __losu_syntaxpar_exp_unitmap(lex);
      break;
    }
    case TOKEN_STRING: {
      losu_syntax_codegenarg1(
          lex->fs, INS_PUSHSTRING,
          losu_syntax_binder_strconst(lex->fs, lex->tk.info.s));
      __losu_syntaxpar_next(lex);
      break;
    }
    default: {
      losu_syntax_error(lex, "expected func args ( ... )");
      break;
    }
  }
  func->stacklevel = sl;
  // printf("[call]%d\n", sl);
  if (async)
    losu_syntax_codegenarg1(func, INS_ASYNC, sl);
  else
    losu_syntax_codegenarg2(func, INS_CALL, sl, 255);
}

static void __losu_syntaxpar_exp_unitlist(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  int32_t line = lex->linenumber;
  int32_t pc = losu_syntax_codegenarg2(lex->fs, INS_CREATEUNIT, 0, 0);
  int32_t niss = 0;
  __losu_syntaxpar_check(lex, '[');
  if (lex->tk.token != ']') {
    niss = 1;
    __losu_syntaxpar_exp(lex);
    while (lex->tk.token == ',') {
      __losu_syntaxpar_next(lex);
      if (lex->tk.token == ']')
        break;
      __losu_syntaxpar_exp(lex);
      niss++;
      if (niss % _losu_vmlim_maxsetlist == 0)
        losu_syntax_codegenarg2(func, INS_SETLIST,
                                niss / _losu_vmlim_maxsetlist - 1,
                                _losu_vmlim_maxsetlist);
    }
    losu_syntax_codegenarg2(func, INS_SETLIST, niss / _losu_vmlim_maxsetlist,
                            niss % _losu_vmlim_maxsetlist);
  }
  __losu_syntaxpar_checkmatch(lex, '[', ']', line);
  _losu_vmcg_setA(func->fcode->code[pc], niss);
  _losu_vmcg_setB(func->fcode->code[pc], obj_unittype_array);
}

static void __losu_syntaxpar_exp_unitmap(losu_syntax_lex_t lex) {
  losu_syntax_func_t func = lex->fs;
  int32_t line = lex->linenumber;
  int32_t pc = losu_syntax_codegenarg2(lex->fs, INS_CREATEUNIT, 0, 0);
  int32_t niss = 0;
  __losu_syntaxpar_check(lex, '{');
  if (lex->tk.token != '}') {
    niss = 1;
    switch (lex->tk.token) {
      case TOKEN_NAME: {
        losu_syntax_codegenarg1(
            func, INS_PUSHSTRING,
            losu_syntax_binder_strconst(func, __losu_syntaxpar_checkname(lex)));
        break;
      }
      case TOKEN_STRING: {
        losu_syntax_codegenarg1(
            func, INS_PUSHSTRING,
            losu_syntax_binder_strconst(func, lex->tk.info.s));
        __losu_syntaxpar_next(lex);
        break;
      }
      /* case '[': {
        __losu_syntaxpar_next(lex);
        __losu_syntaxpar_exp(lex);
        __losu_syntaxpar_check(lex, ']');
        break;
      } */
      default: {
        losu_syntax_error(lex, "invalid unit field");
      }
    }
    __losu_syntaxpar_check(lex, ':');
    __losu_syntaxpar_exp(lex);
    while (lex->tk.token == ',') {
      __losu_syntaxpar_next(lex);
      if (lex->tk.token == '}')
        break;
      switch (lex->tk.token) {
        case TOKEN_NAME: {
          losu_syntax_codegenarg1(func, INS_PUSHSTRING,
                                  losu_syntax_binder_strconst(
                                      func, __losu_syntaxpar_checkname(lex)));
          break;
        }
        case TOKEN_STRING: {
          losu_syntax_codegenarg1(
              func, INS_PUSHSTRING,
              losu_syntax_binder_strconst(func, lex->tk.info.s));
          __losu_syntaxpar_next(lex);
          break;
        }
        /* case '[': {
          __losu_syntaxpar_next(lex);
          __losu_syntaxpar_exp(lex);
          __losu_syntaxpar_check(lex, ']');
          break;
        } */
        default: {
          losu_syntax_error(lex, "invalid unit field");
        }
      }
      __losu_syntaxpar_check(lex, ':');
      __losu_syntaxpar_exp(lex);
      niss++;
      if (niss % _losu_vmlim_maxsetmap == 0)
        losu_syntax_codegenarg1(func, INS_SETMAP, _losu_vmlim_maxsetmap);
    }
    losu_syntax_codegenarg1(func, INS_SETMAP, niss % _losu_vmlim_maxsetmap);
  }
  __losu_syntaxpar_checkmatch(lex, '{', '}', line);
  _losu_vmcg_setA(func->fcode->code[pc], niss);
  _losu_vmcg_setB(func->fcode->code[pc], obj_unittype_object);
}

static int32_t __losu_syntaxpar_exp_varlevel(losu_syntax_lex_t lex,
                                             losu_object_string_t n,
                                             losu_syntax_exp_t v) {
  losu_syntax_func_t func = NULL;
  int32_t lev = 0;
  /* search local  */
  for (func = lex->fs; func; func = func->prev) {
    for (int32_t i = func->naloc - 1; i >= 0; i--) {
      if (n == func->fcode->localvar[func->aloc[i]].name) {
        v->type = losu_syntax_exptype_local;
        v->value.index = i;
        return lev;
      }
    }
    lev++;
  }
  /* is global */
  for (func = lex->fs; func; func = func->prev) {
    for (int32_t i = 0; i < func->nglovar; i++) {
      if (n == func->glovar[i]) {
        v->type = losu_syntax_exptype_global;
        return -1;
      }
    }
  }

  losu_syntax_warning(
      lex, "'%s' is not defined, will be treated as a global variable", n->str);
  /* global */
  v->type = losu_syntax_exptype_global;
  return -1;
}

#endif

#endif