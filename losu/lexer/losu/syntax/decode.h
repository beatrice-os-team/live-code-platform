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
#ifndef FILE_SRC_SYNTAX_DECODE_H
#define FILE_SRC_SYNTAX_DECODE_H

#include "losu.h"
#include "losu_syntax.h"

static void __losu_syntax_decode_header(losu_syntax_lex_t lex);
static losu_object_scode_t __losu_syntax_decode_scode(losu_syntax_lex_t lex);

static uint8_t __losu_syntax_decode_check_endian();

static losu_ctype_number __losu_syntax_decode_number(losu_syntax_lex_t lex);
static losu_object_string_t __losu_syntax_decode_string(losu_syntax_lex_t lex);

static uint16_t __losu_syntax_decode_uint16_t(losu_syntax_lex_t lex);
static int16_t __losu_syntax_decode_int16_t(losu_syntax_lex_t lex);
static uint32_t __losu_syntax_decode_uint32_t(losu_syntax_lex_t lex);
static int32_t __losu_syntax_decode_int32_t(losu_syntax_lex_t lex);
static uint64_t __losu_syntax_decode_uint64_t(losu_syntax_lex_t lex);

static uint8_t __losu_syntax_decode_byte(losu_syntax_lex_t lex);
static void __losu_syntax_decode_blob(losu_syntax_lex_t lex,
                                         void* p,
                                         size_t size);
static char* __losu_syntax_decode_stream(losu_syntax_lex_t lex, size_t size);

#endif