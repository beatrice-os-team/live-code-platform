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
  Copyright  2025  zhang-mohan

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

#ifndef FILE_SRC_EXT_TIME_INDEX_C
#define FILE_SRC_EXT_TIME_INDEX_C

#include <time.h>
#include "losu.h"

static int32_t this_year(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_year) + 1900));
  return 1;
}

static int32_t this_month(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_mon) + 1));
  return 1;
}

static int32_t this_day(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_mday)));
  return 1;
}

static int32_t this_hour(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_hour)));
  return 1;
}

static int32_t this_minute(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_min)));
  return 1;
}

static int32_t this_second(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_sec)));
  return 1;
}

static int32_t this_weekday(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  time_t t = time(NULL);
  stack_push(vm, obj_newnum(vm, (localtime(&t)->tm_wday)));
  return 1;
}

static int32_t this_time(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  stack_push(vm, obj_newnum(vm, (double)clock() / CLOCKS_PER_SEC));
  return 1;
}

losu_object losu_ext_time(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "year", obj_newfunc(vm, this_year));
  obj_unitsetstr(vm, obj, "month", obj_newfunc(vm, this_month));
  obj_unitsetstr(vm, obj, "day", obj_newfunc(vm, this_day));
  obj_unitsetstr(vm, obj, "hour", obj_newfunc(vm, this_hour));
  obj_unitsetstr(vm, obj, "minute", obj_newfunc(vm, this_minute));
  obj_unitsetstr(vm, obj, "second", obj_newfunc(vm, this_second));
  obj_unitsetstr(vm, obj, "weekday", obj_newfunc(vm, this_weekday));
  obj_unitsetstr(vm, obj, "time", obj_newfunc(vm, this_time));

  return obj;
}

#endif
