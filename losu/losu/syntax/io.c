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
#ifndef FILE_SRC_SYNTAX_IO_C
#define FILE_SRC_SYNTAX_IO_C

#include "io.h"
#include "losu_gc.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "losu_vm.h"

#include <stdarg.h>
#include <stdio.h>

/**
 * @brief  load file, push bytecode to vm
 * @param vm losu vm handle
 * @param fn file name
 * @return 0 is ok, 1 is fail, -1 is not found
 */
int8_t losu_syntax_io_loadfile(losu_vm_t vm, const char* fn, const char* name) {
  losu_syntax_io io = {0};
  losu_ctype_bool sta = 0;
  FILE* fp = fopen(fn, "rb");
  if (!fp) {
    // vm_warning(vm, "open file '%s' fail", fn);
    fprintf(stderr, "open file '%s' fail\n", fn);
    return -1;
  }
  __losu_syntax_io_openH(&io, (void*)fp, name);
  const char* oldname = vm->name;
  vm->name = fn;
  sta = __losu_syntax_io_doload(vm, &io);
  vm->name = oldname;
  fclose(fp);
  return sta;
}
/**
 * @brief  load string, push bytecode to vm
 * @param vm losu vm handle
 * @param str string
 * @param len string length
 * @return 0 is ok, 1 is fail
 */
losu_ctype_bool losu_syntax_io_loadstring(losu_vm_t vm,
                                          const char* str,
                                          size_t len,
                                          const char* name) {
  losu_syntax_io io = {0};
  losu_ctype_bool sta = 0;
  __losu_syntax_io_openS(&io, str, len, name);
  const char* oldname = vm->name;
  vm->name = name;
  sta = __losu_syntax_io_doload(vm, &io);
  vm->name = oldname;
  return sta;
}

/**
 * @brief throw a syntax error
 * @param vm losu vm handle
 * @param fmt string format
 * @param ... string args
 */
void losu_syntax_error(losu_syntax_lex_t lex, const char* fmt, ...) {
  char tmp[256] = {0};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, 256, fmt, ap);
  fprintf(stderr, "losu v%s\n\tsyntax error\n\t%s\n\tat line %d\n\tof %s\n",
          LOSU_RELEASE, tmp, lex->linenumber,
          lex->io->name != NULL ? lex->io->name : "<unknown>");
  va_end(ap);
  losu_sigmsg_throw(lex->vm, losu_signal_error);
}

/**
 * @brief throw a syntax warning
 * @param vm losu vm handle
 * @param fmt string format
 * @param ... string args
 */
void losu_syntax_warning(losu_syntax_lex_t lex, const char* fmt, ...) {
  char tmp[256] = {0};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, 256, fmt, ap);
  fprintf(stderr,
          "<span style='color:yellow'>losu v%s</span>\n<span "
          "style='color:yellow'>\tsyntax warning</span>\n<span "
          "style='color:yellow'>\t%s</span>\n<span style='color:yellow'>\tat "
          "line %d</span>\n<span style='color:yellow'>\tof %s\n</span>",
          LOSU_RELEASE, tmp, lex->linenumber,
          lex->io->name != NULL ? lex->io->name : "<unknown>");
  va_end(ap);
}

/**
 * @brief open handle-io
 * @param io io
 * @param handle handle pointer
 * @param name handle name
 */
static void __losu_syntax_io_openH(losu_syntax_io_t io,
                                   void* handle,
                                   const char* name) {
  if (!handle)
    return;
  io->size = 0;
  io->p = io->buff;
  io->fillbuff = fillbuffH;
  io->h = handle;
  io->name = name;
  /* bin or not */
  int32_t magic;
  magic = fgetc(io->h);
  io->bin = (magic == 0x00);
  fseek((FILE*)handle, 0, SEEK_SET);
}

/**
 * @brief open string-io
 * @param io io
 * @param p string pointer
 * @param size string size
 * @param name string name
 */
static void __losu_syntax_io_openS(losu_syntax_io_t io,
                                   const char* p,
                                   size_t size,
                                   const char* name) {
  io->size = p == NULL ? 0 : size;
  io->p = (const unsigned char*)p;
  io->fillbuff = fillbuffS;
  io->h = NULL;
  io->name = name;
  /* bin or not */
  io->bin = (io->size > 1 && io->p[0] == 0x00);
}

/**
 * @brief fill string-io
 * @param io io
 * @return first char
 */
static int16_t fillbuffS(losu_syntax_io_t io) {
  return EOF;
}

/**
 * @brief fill handle-io
 * @param io io
 * @return first char
 */
static int16_t fillbuffH(losu_syntax_io_t io) {
  size_t n = 0;
  if (feof((FILE*)io->h))
    return EOF;
  n = fread(io->buff, sizeof(char), 32, (FILE*)io->h);
  if (n == 0)
    return EOF;
  io->size = n - 1;
  io->p = io->buff;
  return (int16_t)(*io->p++);
}

/**
 * @brief do load of io
 * @param vm losu vm handle
 * @param io  io
 * @return 0 is ok, 1 is fail
 */
static losu_ctype_bool __losu_syntax_io_doload(losu_vm_t vm,
                                               losu_syntax_io_t io) {
  /* save context */
  losu_object_t oldtop = vm->top;
  __losu_vmsig_t oldjmp = vm->errjmp;

  /* new context */
  __losu_vmsig newjmp = {0};
  vm->errjmp = &newjmp;
  int32_t i = setjmp(*vm->errjmp);
  switch (i) {
    /* ok */
    case losu_signal_done: {
      losu_object_scode_t fcd =
          io->bin ? losu_syntax_decode(vm, io) : losu_syntax_parse(vm, io);
      losu_object_function_t f = losu_objfunc_new(vm, 0);
      f->func.sdef = fcd;
      f->isC = 0;
      ovtype(vm->top) = losu_object_type_function;
      ovfunc(vm->top) = f;
      vm->top++;
      break;
    }
    /* error */
    default: {
      /* reset context */
      vm->top = oldtop;
      vm->errjmp = oldjmp;
      return losu_signal_error;
    }
  }
  /* reset context, without top */
  vm->errjmp = oldjmp;
  return 0;
}

#endif