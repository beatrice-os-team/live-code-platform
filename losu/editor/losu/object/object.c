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

#ifndef FILE_SRC_OBJECT_OBJECT_C
#define FILE_SRC_OBJECT_OBJECT_C

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "losu_object.h"

const losu_object __losu_objconst_null = {
    .type = losu_object_type_false,
    .value = {NULL},
};

const losu_object __losu_objconst_true = {
    .type = losu_object_type_true,
    .value = {NULL},
};

/**
 * @brief check 2 objects is EQU (type and value)
 * @param vm LosuVm handle
 * @param t1 object 1
 * @param t2 object 2
 * @return 1 is EQU, 0 is not
 */
losu_ctype_bool losu_object_iseq(losu_vm_t vm,
                                 const losu_object_t t1,
                                 const losu_object_t t2) {
  if (t1 == NULL || t2 == NULL)
    return 0;
  if (ovtype(t1) != ovtype(t2))
    return ((ovtype(t1) == losu_object_type_true &&
             ovtype(t2) != losu_object_type_false) ||
            (ovtype(t2) == losu_object_type_true &&
             ovtype(t1) != losu_object_type_false));
  /* check with same type */
  switch (ovtype(t1)) {
    case losu_object_type_false:
    case losu_object_type_true:
      return 1;
    case losu_object_type_number:
      return ovnumber(t1) == ovnumber(t2);
    case losu_object_type_string:
      return ovIstr(t1) == ovIstr(t2);
      /* return (ovIstr(t1)->hash == ovIstr(t2)->hash) &&
             (ovIstr(t1)->len == ovIstr(t2)->len) &&
             !memcmp(ovSstr(t1), ovSstr(t2), ovIstr(t1)->len); */
    case losu_object_type_function:
      // case losu_object_type_asyncfunc:
      // case losu_object_type_classfunc:
      return ovfunc(t1) == ovfunc(t2);
    case losu_object_type_unit:
      return ovhash(t1) == ovhash(t2);
    case losu_object_type_pointer:
      return ovptr(t1) == ovptr(t2);
    case losu_object_type_coroutine:
      return ovcoro(t1) == ovcoro(t2);
    default:
      return 0;
  }
  return 0;
}

/**
 * @brief check Obj1 is less than Obj2
 * @param vm LosuVm handle
 * @param t1 Obj1
 * @param t2 Obj2
 * @return 1 is EQU, 0 is not
 */
losu_ctype_bool losu_object_less(losu_vm_t vm,
                                 const losu_object_t t1,
                                 const losu_object_t t2) {
  if (t1 == NULL || t2 == NULL)
    return 0;
  if (ovtype(t1) != ovtype(t2))
    vm_error(
        vm, "It is not permitted to compare multiple data types: '%s' and '%s'",
        (obj_typestr(vm, t1)), (obj_typestr(vm, t2)));
  switch (ovtype(t1)) {
    case losu_object_type_number:
      return ovnumber(t1) < ovnumber(t2);
    case losu_object_type_string: {
      const void* l = (const void*)(ovSstr(t1));
      const void* r = (const void*)(ovSstr(t2));
      losu_ctype_size_t ll = ovIstr(t1)->len;
      losu_ctype_size_t lr = ovIstr(t2)->len;
      losu_ctype_size_t ml = ll < lr ? ll : lr;
      int32_t i = memcmp(l, r, ml);
      return i < 0 ? 1 : ((i == 0) ? (ll < lr) : 0);
    }
    default:
      vm_error(
          vm,
          "It is not permitted to compare multiple data types: '%s' and '%s'",
          (obj_typestr(vm, t1)), (obj_typestr(vm, t2)));
  }
  return 0;
}

/**
 * @brief string(const char*) to number(losu_ctype_number)
 * @param vm LosuVm handle
 * @param s string
 * @param n number
 * @return 1 is Ok, 0 is not
 */
losu_ctype_bool losu_object_str2num(losu_vm_t vm,
                                    const char* s,
                                    losu_ctype_number* n) {
  char* endp;
  losu_ctype_number res = strtod(s, &endp);
  if (endp == s)
    return 0;
  while (isspace((uint8_t)*endp))
    endp++;
  if (*endp != '\0')
    return 0;
  *n = res;
  return 1;
}

#endif
