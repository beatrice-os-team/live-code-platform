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
#ifndef FILE_SRC_DUMP_H
#define FILE_SRC_DUMP_H

#include "losu.h"
#include "losu_dump.h"

typedef struct losu_dumper {
  losu_vm_t vm;
  FILE* fp;
} losu_dumper, *losu_dumper_t;

static uint8_t __losu_dump_check_endian();
static void __losu_dump_header(losu_dumper_t d);
static void __losu_dump_scode(losu_dumper_t d, losu_object_scode_t scode);

static void __losu_dump_string(losu_dumper_t d, losu_object_string_t s);
static void __losu_dump_number(losu_dumper_t d, losu_ctype_number i);

static void __losu_dump_uint16_t(losu_dumper_t d, uint16_t i);
static void __losu_dump_int16_t(losu_dumper_t d, int16_t i);
static void __losu_dump_uint32_t(losu_dumper_t d, uint32_t i);
static void __losu_dump_int32_t(losu_dumper_t d, int32_t i);
static void __losu_dump_uint64_t(losu_dumper_t d, uint64_t i);

static void __losu_dump_byte(losu_dumper_t d, uint8_t i);
static void __losu_dump_blob(losu_dumper_t d, void* p, size_t size);
static void __losu_dump_stream(losu_dumper_t d, void* p, size_t size);

#endif