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

#ifndef FILE_SRC_LOSU_SYNTAX_H
#define FILE_SRC_LOSU_SYNTAX_H

#include "losu.h"
#include "losu_bytecode.h"

/**
 * Syntax data-structure
 */
#if 1

/* exp-type */
typedef uint8_t losu_syntax_exptype_t;
#define losu_syntax_exptype_global ((losu_syntax_exptype_t)0)
#define losu_syntax_exptype_local ((losu_syntax_exptype_t)1)
#define losu_syntax_exptype_index ((losu_syntax_exptype_t)2)
#define losu_syntax_exptype_expr ((losu_syntax_exptype_t)3)
#define NO_JUMP (-1)

/* exp */
typedef struct losu_syntax_exp {
  losu_syntax_exptype_t type;
  union {
    int32_t index;
    struct {
      int32_t t, f;
    } _bool;
  } value;
} losu_syntax_exp, *losu_syntax_exp_t;

/* token */
typedef struct losu_syntax_token {
  int16_t token;
  union losu_syntax_tkvalue {
    losu_ctype_number num;
    losu_object_string_t s;
  } info;
} losu_syntax_token, *losu_syntax_token_t;

typedef union losu_syntax_tkvalue losu_syntax_tkvalue, *losu_syntax_tkvalue_t;

/* break & continue & raise */
typedef struct losu_syntax_break {
  struct losu_syntax_break* pre;
  int32_t breaklist;
  int16_t stacklevel;
} losu_syntax_break, *losu_syntax_break_t;

typedef struct losu_syntax_continue {
  struct losu_syntax_continue* pre;
  int32_t tmppc;    /* jmp: anypc -> tmppc -> startpos */
  int32_t startpos; /* loop start pc */
  int16_t stacklevel;
} losu_syntax_continue, *losu_syntax_continue_t;

typedef struct losu_syntax_raise {
  struct losu_syntax_raise* pre;
  int32_t startpc;
  int16_t stacklevel;
} losu_syntax_raise, *losu_syntax_raise_t;

/* syntax func */
typedef struct losu_syntax_func {
  losu_object_scode_t fcode;
  losu_object_string_t source;
  struct losu_syntax_func* prev;
  struct losu_syntax_lex* lexer;
  losu_vm_t vm;
  int32_t pc;
  int32_t lasttarget;
  int32_t lastline;
  int32_t jump;
  int16_t stacklevel;
  /* active local */
  int32_t aloc[_losu_vmlim_maxlocvar];
  int16_t naloc;
  /* active global var */
  losu_object_string_t glovar[_losu_vmlim_maxglovar];
  int16_t nglovar;
  /* closure */
  struct losu_syntax_exp closure[_losu_vmlim_maxclosure];
  int16_t nclosure;

  /* break & continue */
  struct losu_syntax_break* breaklist;
  struct losu_syntax_continue* contlist;
  struct losu_syntax_raise* raiselist;
} losu_syntax_func, *losu_syntax_func_t;

/* syntax lex */
typedef struct losu_syntax_lex {
  int16_t current; /* uint8_t -> int16_t */
  struct losu_syntax_token tk;
  struct losu_syntax_token tkahead;
  struct losu_syntax_func* fs;
  losu_vm_t vm;
  struct losu_syntax_io* io;
  int32_t linenumber;
  int32_t lastline;
  struct losu_syntax_lex_idt {
    int32_t read;
    int32_t nowidt;
    int32_t size;
    int32_t tmp;
  } idt;
} losu_syntax_lex, *losu_syntax_lex_t;

typedef struct losu_syntax_lex_idt losu_syntax_lex_idt, *losu_syntax_lex_idt_t;

#endif

/**
 * Syntax IO
 */
#if 1

typedef struct losu_syntax_io {
  losu_ctype_size_t size;
  const unsigned char* p;
  int16_t (*fillbuff)(struct losu_syntax_io*);
  void* h;
  const char* name;
  losu_ctype_bool bin;
  unsigned char buff[1024];
} losu_syntax_io, *losu_syntax_io_t;

#endif

/**
 * Syntax API
 */
#if 1

/* assert */

void losu_syntax_error(losu_syntax_lex_t lex, const char* fmt, ...);

void losu_syntax_warning(losu_syntax_lex_t lex, const char* fmt, ...);

/* IO */

int8_t losu_syntax_io_loadfile(losu_vm_t vm, const char* fn, const char* name);
losu_ctype_bool losu_syntax_io_loadstring(losu_vm_t vm,
                                          const char* str,
                                          size_t len,
                                          const char* name);

#define losu_syntax_io_getc(io) \
  (((io)->size--) > 0 ? ((int16_t)(*((io)->p++))) : ((io)->fillbuff(io)))

/* lex */

void losu_syntax_lex_init(losu_syntax_lex_t lex);
int16_t losu_syntax_lex_next(losu_syntax_lex_t lex, losu_syntax_tkvalue_t tk);

/* syntax */

losu_object_scode_t losu_syntax_parse(losu_vm_t vm, losu_syntax_io_t io);

/* bytecode */

losu_object_scode_t losu_syntax_decode(losu_vm_t vm, losu_syntax_io_t io);

/* codegen */

int32_t losu_syntax_codegen(losu_syntax_func_t func,
                            _losu_vmins_OP o,
                            int32_t arg1,
                            int32_t arg2);
void losu_syntax_codegen_dtstack(losu_syntax_func_t func, int32_t dt);

#define losu_syntax_codegenarg2 losu_syntax_codegen
#define losu_syntax_codegenarg1(fs, o, arg) (losu_syntax_codegen(fs, o, arg, 0))
#define losu_syntax_codegenarg0(fs, o) (losu_syntax_codegen(fs, o, 0, 0))

#endif

#endif