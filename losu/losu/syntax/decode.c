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
#ifndef FILE_SRC_SYNTAX_DECODE_C
#define FILE_SRC_SYNTAX_DECODE_C

#include "decode.h"
#include "losu_mem.h"
#include "losu_object.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

losu_object_scode_t losu_syntax_decode(losu_vm_t vm, losu_syntax_io_t io) {
  losu_object_scode_t fs = NULL;
  losu_syntax_lex lex = {0};
  lex.vm = vm;
  lex.io = io;
  lex.linenumber = lex.lastline = -1;

  __losu_syntax_decode_header(&lex);
  fs = __losu_syntax_decode_scode(&lex);

  // check is the scode is valid
  if (__losu_syntax_decode_uint64_t(&lex) != LOSU_MAGIC)
    losu_syntax_error(&lex, "invalid bytecode file");
  return fs;
}

static void __losu_syntax_decode_header(losu_syntax_lex_t lex) {
  // magic number
  if (__losu_syntax_decode_uint64_t(lex) != LOSU_MAGIC)
    losu_syntax_error(lex, "invalid bytecode file");
  // check version
  uint32_t vmin = LOSU_VERSION[0];
  uint32_t vmax = LOSU_VERSION[1];
  uint32_t vfile = __losu_syntax_decode_uint32_t(lex);
  if (!(vmin <= vfile && vfile <= vmax)) {
    losu_syntax_error(lex, "byte code version is not supported");
    return;
  }
}

static losu_object_scode_t __losu_syntax_decode_scode(losu_syntax_lex_t lex) {
  // nargs -> isVarg -> maxstack -> member
  //  -> lineinfo -> lcnum -> lcstr -> lcscode -> instruction
  losu_object_scode_t code = losu_objsocde_new(lex->vm);
  code->narg = __losu_syntax_decode_int16_t(lex);
  code->isVarg = __losu_syntax_decode_byte(lex);
  code->maxstacksize = __losu_syntax_decode_int16_t(lex);

  // lineinfo
  code->nlineinfo = __losu_syntax_decode_uint32_t(lex);
  code->lineinfo = losu_mem_newvector(lex->vm, code->nlineinfo, int32_t);
  for (uint32_t i = 0; i < code->nlineinfo; i++)
    code->lineinfo[i] = __losu_syntax_decode_int32_t(lex);

  // lcnum
  code->nlcnum = __losu_syntax_decode_uint32_t(lex);
  code->lcnum = losu_mem_newvector(lex->vm, code->nlcnum, losu_ctype_number);
  for (uint32_t i = 0; i < code->nlcnum; i++)
    code->lcnum[i] = __losu_syntax_decode_number(lex);

  // lcstr
  code->nlcstr = __losu_syntax_decode_uint32_t(lex);
  code->lcstr = losu_mem_newvector(lex->vm, code->nlcstr, losu_object_string_t);
  for (uint32_t i = 0; i < code->nlcstr; i++)
    code->lcstr[i] = __losu_syntax_decode_string(lex);

  // lcscode
  code->nlcscode = __losu_syntax_decode_uint32_t(lex);
  code->lcscode =
      losu_mem_newvector(lex->vm, code->nlcscode, losu_object_scode_t);
  for (uint32_t i = 0; i < code->nlcscode; i++)
    code->lcscode[i] = __losu_syntax_decode_scode(lex);

  // instruction
  code->ncode = __losu_syntax_decode_uint32_t(lex);
  code->code = losu_mem_newvector(lex->vm, code->ncode, losu_ctype_vmins_t);
  for (uint32_t i = 0; i < code->ncode; i++)
    code->code[i] = __losu_syntax_decode_uint32_t(lex);

  return code;
}

static losu_ctype_number __losu_syntax_decode_number(losu_syntax_lex_t lex) {
  losu_ctype_number num;
  __losu_syntax_decode_blob(lex, &num, sizeof(num));
  return num;
}
static losu_object_string_t __losu_syntax_decode_string(losu_syntax_lex_t lex) {
  uint32_t len = __losu_syntax_decode_uint32_t(lex);
  char* s = __losu_syntax_decode_stream(lex, len);
  return losu_objstring_newlen(lex->vm, s, len);
}

static uint16_t __losu_syntax_decode_uint16_t(losu_syntax_lex_t lex) {
  uint16_t i;
  __losu_syntax_decode_blob(lex, &i, sizeof(i));
  return i;
}
static int16_t __losu_syntax_decode_int16_t(losu_syntax_lex_t lex) {
  int16_t i;
  __losu_syntax_decode_blob(lex, &i, sizeof(i));
  return i;
}
static uint32_t __losu_syntax_decode_uint32_t(losu_syntax_lex_t lex) {
  uint32_t i;
  __losu_syntax_decode_blob(lex, &i, sizeof(i));
  return i;
}
static int32_t __losu_syntax_decode_int32_t(losu_syntax_lex_t lex) {
  int32_t i;
  __losu_syntax_decode_blob(lex, &i, sizeof(i));
  return i;
}
static uint64_t __losu_syntax_decode_uint64_t(losu_syntax_lex_t lex) {
  uint64_t i;
  __losu_syntax_decode_blob(lex, &i, sizeof(i));
  return i;
}

static uint8_t __losu_syntax_decode_byte(losu_syntax_lex_t lex) {
  return (lex->io->size-- > 0) ? (*(lex->io->p++))
                               : ((uint8_t)lex->io->fillbuff(lex->io));
}
static void __losu_syntax_decode_blob(losu_syntax_lex_t lex,
                                      void* p,
                                      size_t size) {
  if (__losu_syntax_decode_check_endian()) {  // big
    for (void* s = p + size - 1; s >= p; s--)
      *(uint8_t*)s = __losu_syntax_decode_byte(lex);
  } else {  // little
    for (size_t i = 0; i < size; i++)
      *((uint8_t*)p + i) = __losu_syntax_decode_byte(lex);
  }
}
static char* __losu_syntax_decode_stream(losu_syntax_lex_t lex, size_t size) {
  if (size > lex->vm->nbufftmp) {
    losu_mem_reallocvector(lex->vm, lex->vm->bufftmp, size, unsigned char);
    lex->vm->nblocks += (size - lex->vm->nbufftmp) * sizeof(unsigned char);
    lex->vm->nbufftmp = size;
    memset(lex->vm->bufftmp, 0, lex->vm->nbufftmp);
  }
  for (size_t i = 0; i < size; i++) {
    uint16_t tmp = __losu_syntax_decode_uint16_t(lex);
    lex->vm->bufftmp[i] = (uint8_t)((~tmp) >> (i % 7 + 1));
  }
  return (char*)lex->vm->bufftmp;
}

/**
 * @brief check endian
 * @return 0 is little endian, 1 is big endian
 */
static uint8_t __losu_syntax_decode_check_endian() {
  int num = 1;
  char* endian = (char*)&num;
  return (*endian == 1) ? 0 : 1;
}
#endif