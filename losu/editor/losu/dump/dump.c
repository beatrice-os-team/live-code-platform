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
#ifndef FILE_SRC_DUMP_C
#define FILE_SRC_DUMP_C
#include "dump.h"
#include "losu_object.h"
#include "losu_vm.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief dump a losu file to obj file
 * @param srcfile the src file
 * @param objfile the obj file
 * @return 0 is ok, other is error
 */
int32_t losu_dumpfile(const char* srcfile, const char* objfile) {
  losu_dumper dumper = {0};
  dumper.vm = vm_create(1024);

  if (vm_loadfile(dumper.vm, srcfile, srcfile))
    return -1;

  __losu_vmsig jmp;
  dumper.vm->errjmp = &jmp;
  dumper.vm->name = srcfile;

  int32_t jsta = setjmp(jmp);
  if (!jsta) {
    dumper.fp = fopen(objfile, "wb");
    if (!dumper.fp) {
      losu_dump_error(dumper.vm, "can't open output file %s", objfile);
      return -1;
    }
    // dump
    __losu_dump_header(&dumper);
    __losu_dump_scode(&dumper, ovfunc((dumper.vm->top - 1))->func.sdef);
    __losu_dump_uint64_t(&dumper, LOSU_MAGIC);

  } else {
    if (dumper.fp)
      fclose(dumper.fp);
    vm_close(dumper.vm);
    return -1;
  }
  fclose(dumper.fp);
  vm_close(dumper.vm);
  return 0;
}

/**
 * @brief dump a losu code to obj file
 * @param src the src code
 * @param objfile the obj file
 * @return 0 is ok, other is error
 */
int32_t losu_dumpstr(const char* src, const char* objfile) {
  losu_dumper dumper = {0};
  dumper.vm = vm_create(1024);

  if (vm_loadbyte(dumper.vm, src, strlen(src), "inline code"))
    return -1;

  __losu_vmsig jmp;
  dumper.vm->errjmp = &jmp;
  dumper.vm->name = "inline code";

  int32_t jsta = setjmp(jmp);
  if (!jsta) {
    dumper.fp = fopen(objfile, "wb");
    if (!dumper.fp) {
      losu_dump_error(dumper.vm, "can't open output file %s", objfile);
      return -1;
    }
    // dump
    __losu_dump_header(&dumper);
    __losu_dump_scode(&dumper, ovfunc((dumper.vm->top - 1))->func.sdef);
    __losu_dump_uint64_t(&dumper, LOSU_MAGIC);

  } else {
    if (dumper.fp)
      fclose(dumper.fp);
    vm_close(dumper.vm);
    return -1;
  }
  fclose(dumper.fp);
  vm_close(dumper.vm);
  return 0;
}

/**
 * @brief dump error message
 * @param vm losu vm handle
 * @param fmt fmt error message
 * @param ... var args
 */
void losu_dump_error(losu_vm_t vm, const char* fmt, ...) {
  char tmp[256] = {0};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, 256, fmt, ap);
  fprintf(stderr, "losu v%s\n\tdump error\n\t%s\n\tof %s\n", LOSU_RELEASE, tmp,
          vm->name);
  va_end(ap);
  losu_sigmsg_throw(vm, losu_signal_error);
}

/**
 * @brief check system endian
 * @return 0 is little endian, 1 is big endian
 */
static uint8_t __losu_dump_check_endian() {
  int num = 1;
  char* endian = (char*)&num;
  return (*endian == 1) ? 0 : 1;
}

static void __losu_dump_header(losu_dumper_t d) {
  // magic number
  __losu_dump_uint64_t(d, LOSU_MAGIC);
  // version [min max]  x.x.x
  __losu_dump_uint32_t(d, LOSU_VERSION[1]);
}
static void __losu_dump_scode(losu_dumper_t d, losu_object_scode_t scode) {
  // nargs -> isVarg -> maxstack -> member
  //  -> lineinfo -> lcnum -> lcstr -> lcscode -> instruction
  __losu_dump_int16_t(d, scode->narg);
  __losu_dump_byte(d, scode->isVarg);
  __losu_dump_int16_t(d, scode->maxstacksize);

  // lineinfo
  __losu_dump_uint32_t(d, scode->nlineinfo);
  for (uint32_t i = 0; i < scode->nlineinfo; i++)
    __losu_dump_int32_t(d, scode->lineinfo[i]);

  // lcnum
  __losu_dump_uint32_t(d, scode->nlcnum);
  for (uint32_t i = 0; i < scode->nlcnum; i++)
    __losu_dump_number(d, scode->lcnum[i]);

  // lcstr
  __losu_dump_uint32_t(d, scode->nlcstr);
  for (uint32_t i = 0; i < scode->nlcstr; i++)
    __losu_dump_string(d, scode->lcstr[i]);

  // lcscode
  __losu_dump_uint32_t(d, scode->nlcscode);
  for (uint32_t i = 0; i < scode->nlcscode; i++)
    __losu_dump_scode(d, scode->lcscode[i]);

  // instruction
  __losu_dump_uint32_t(d, scode->ncode);
  for (int32_t i = 0; i < scode->ncode; i++)
    __losu_dump_uint32_t(d, scode->code[i]);
}

static void __losu_dump_string(losu_dumper_t d, losu_object_string_t s) {
  __losu_dump_uint32_t(d, (uint32_t)s->len);
  __losu_dump_stream(d, (void*)s->str, s->len);
}
static void __losu_dump_number(losu_dumper_t d, losu_ctype_number i) {
  __losu_dump_blob(d, (void*)&i, sizeof(losu_ctype_number));
}

static void __losu_dump_uint16_t(losu_dumper_t d, uint16_t i) {
  __losu_dump_blob(d, (void*)&i, sizeof(uint16_t));
}

static void __losu_dump_int16_t(losu_dumper_t d, int16_t i) {
  __losu_dump_blob(d, (void*)&i, sizeof(int16_t));
}
static void __losu_dump_int32_t(losu_dumper_t d, int32_t i) {
  __losu_dump_blob(d, (void*)&i, sizeof(int32_t));
}
static void __losu_dump_uint32_t(losu_dumper_t d, uint32_t i) {
  __losu_dump_blob(d, (void*)&i, sizeof(uint32_t));
}
static void __losu_dump_uint64_t(losu_dumper_t d, uint64_t i) {
  __losu_dump_blob(d, (void*)&i, sizeof(uint64_t));
}
static void __losu_dump_byte(losu_dumper_t d, uint8_t i) {
  fputc(i, d->fp);
}
static void __losu_dump_blob(losu_dumper_t d, void* p, size_t size) {
  // losu bytecode is little endian
  if (__losu_dump_check_endian()) {  // big to little
    for (void* i = p + size - 1; i >= p; i--)
      __losu_dump_byte(d, *(uint8_t*)i);
  } else {
    fwrite(p, size, 1, d->fp);
  }
}
static void __losu_dump_stream(losu_dumper_t d, void* p, size_t size) {
  for (size_t i = 0; i < size; i++) {
    uint8_t c = ((uint8_t*)p)[i];
    uint16_t b = ~((uint16_t)c) << (i % 7 + 1);
    __losu_dump_uint16_t(d, b);
  }
  // fwrite(p, size, 1, d->fp);
}

#endif