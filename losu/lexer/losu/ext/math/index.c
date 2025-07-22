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

#ifndef FILE_SRC_EXT_LOSU_INDEX_C
#define FILE_SRC_EXT_LOSU_INDEX_C

#include <math.h>
#include <string.h>
#include "losu.h"

static int32_t losu_ext_abs(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "abs() takes exactly one argument");
    return 0;
  }
  losu_ctype_number n = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, n > 0 ? n : -n));
  return 1;
}
static int32_t losu_ext_mod(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 2) {
    vm_error(vm, "mod() takes exactly two arguments");
    return 0;
  }
  losu_ctype_number num_div = obj_tonum(vm, &argv[0]);
  losu_ctype_number num_dived = obj_tonum(vm, &argv[1]);
  losu_ctype_number res = num_div / num_dived;
  stack_push(vm, obj_newnum(vm, res));
  return 1;
}
static int32_t losu_ext_asin(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "asin() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, asin(num)));
  return 1;
}

static int32_t losu_ext_acos(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "acos() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, acos(num)));
  return 1;
}

static int32_t losu_ext_atan(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "atan() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, atan(num)));
  return 1;
}

static int32_t losu_ext_sin(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "sin() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, sin(num)));
  return 1;
}

static int32_t losu_ext_cos(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "cos() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, cos(num)));
  return 1;
}
static int32_t losu_ext_tan(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "tan() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, tan(num)));
  return 1;
}
static int32_t losu_ext_exp(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "exp() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, exp(num)));
  return 1;
}
static int32_t losu_ext_log(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "log() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, log(num)));
  return 1;
}
static int32_t losu_ext_ln(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "ln() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, log10(num)));
  return 1;
}
static int32_t losu_ext_lg(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "lg() takes exactly one argument");
    return 0;
  }
  losu_ctype_number num = obj_tonum(vm, &argv[0]);
  stack_push(vm, obj_newnum(vm, log(num)));
  return 1;
}
static int32_t losu_ext_sqrt(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "sqrt() takes exactly one argument");
    return 0;
  }
  stack_push(vm, obj_newnum(vm, sqrt(obj_tonum(vm, &argv[0]))));
  return 1;
}
static int32_t losu_ext_ceil(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "ceil() takes exactly one argument");
    return 0;
  }
  stack_push(vm, obj_newnum(vm, ceil(obj_tonum(vm, &argv[0]))));
  return 1;
}
static int32_t losu_ext_floor(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "floor() takes exactly one argument");
    return 0;
  }
  stack_push(vm, obj_newnum(vm, floor(obj_tonum(vm, &argv[0]))));
  return 1;
}
static int32_t losu_ext_round(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "round() takes exactly one argument");
    return 0;
  }
  stack_push(vm, obj_newnum(vm, round(obj_tonum(vm, &argv[0]))));
  return 1;
}
static int32_t losu_ext_cut(losu_vm_t vm, int32_t argc, losu_object argv[]) {
  if (argc != 1) {
    vm_error(vm, "cut() takes exactly one argument");
    return 0;
  }

  stack_push(vm, obj_newnum(vm, trunc(obj_tonum(vm, &argv[0]))));
  return 1;
}

#define PI 3.141592653589793
#define E 2.718281828459045
#define GAMMA 0.577215664901532
losu_object losu_ext_math(losu_vm_t vm) {
  losu_object obj = obj_newunit(vm, obj_unittype_module);
  obj_unitsetstr(vm, obj, "abs", obj_newfunc(vm, losu_ext_abs));
  obj_unitsetstr(vm, obj, "mod", obj_newfunc(vm, losu_ext_mod));
  obj_unitsetstr(vm, obj, "acos", obj_newfunc(vm, losu_ext_acos));
  obj_unitsetstr(vm, obj, "asin", obj_newfunc(vm, losu_ext_asin));
  obj_unitsetstr(vm, obj, "atan", obj_newfunc(vm, losu_ext_atan));
  obj_unitsetstr(vm, obj, "cos", obj_newfunc(vm, losu_ext_cos));
  obj_unitsetstr(vm, obj, "sin", obj_newfunc(vm, losu_ext_sin));
  obj_unitsetstr(vm, obj, "tan", obj_newfunc(vm, losu_ext_tan));
  obj_unitsetstr(vm, obj, "exp", obj_newfunc(vm, losu_ext_exp));
  obj_unitsetstr(vm, obj, "log", obj_newfunc(vm, losu_ext_log));
  obj_unitsetstr(vm, obj, "ln", obj_newfunc(vm, losu_ext_ln));
  obj_unitsetstr(vm, obj, "lg", obj_newfunc(vm, losu_ext_lg));
  obj_unitsetstr(vm, obj, "sqrt", obj_newfunc(vm, losu_ext_sqrt));
  obj_unitsetstr(vm, obj, "ceil", obj_newfunc(vm, losu_ext_ceil));
  obj_unitsetstr(vm, obj, "floor", obj_newfunc(vm, losu_ext_floor));
  obj_unitsetstr(vm, obj, "round", obj_newfunc(vm, losu_ext_round));
  obj_unitsetstr(vm, obj, "cut", obj_newfunc(vm, losu_ext_cut));
  obj_unitsetstr(vm, obj, "PI", obj_newnum(vm, PI));
  obj_unitsetstr(vm, obj, "E", obj_newnum(vm, E));
  obj_unitsetstr(vm, obj, "GAMMA", obj_newnum(vm, GAMMA));
  return obj;
}

#undef PI
#undef E
#undef GAMMA

#endif
