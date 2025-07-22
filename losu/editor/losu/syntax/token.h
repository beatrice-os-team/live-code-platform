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

#ifndef FILE_SRC_LOSU_SYNTAX_TOKEN_H
#define FILE_SRC_LOSU_SYNTAX_TOKEN_H
#include "losu.h"

/* token */
typedef enum _losu_token {
  /** logic */
  TOKEN_AND = 256,
  TOKEN_OR,
  TOKEN_NOT,
  TOKEN_TRUE,
  TOKEN_FALSE,

  /** pass */
  TOKEN_PASS,

  /** if */
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_ELSEIF,

  /** def */
  TOKEN_DEF,
  TOKEN_LAMBDA,
  TOKEN_ARG,

  /** let global  */
  TOKEN_LET,
  TOKEN_GLOBAL,
  TOKEN_CLASS,
  // TOKEN_MODULE,

  /* return break continue yield */
  TOKEN_RETURN,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_YIELD,

  /* with until for */
  TOKEN_WHILE,
  TOKEN_UNTIL,
  TOKEN_FOR,

  /* import */
  TOKEN_IMPORT,
  /* async */
  TOKEN_ASYNC,

  /* match case */
  TOKEN_MATCH,
  TOKEN_CASE,
  /* assert except */
  // TOKEN_ASSERT,
  TOKEN_EXCEPT,
  TOKEN_RAISE,

  /* == >= <= != :: ** |> */
  TOKEN_EQ,
  TOKEN_GE,
  TOKEN_LE,
  TOKEN_NE,
  TOKEN_ASSIGN,
  TOKEN_POW,
  TOKEN_PIPE,


  /* other */
  TOKEN_NAME,
  TOKEN_NUMBER,
  TOKEN_STRING,

  TOKEN_EOZ,
} _losu_token;

typedef struct losu_syntax_keyword {
  const char* str;
  int8_t idt;
  int16_t token;
} losu_syntax_keyword_t;
extern const losu_syntax_keyword_t losu_syntax_keyword[TOKEN_EOZ - 0xff];
#endif