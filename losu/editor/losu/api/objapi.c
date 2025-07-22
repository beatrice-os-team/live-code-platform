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

#ifndef FILE_SRC_API_OBJAPI_C
#define FILE_SRC_API_OBJAPI_C

#include "losu.h"
#include "losu_object.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* object */
#if 1

/**
 * @brief get type id
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return int32_t the type id
 */
losu_extern_t int32_t obj_type(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj);
}

/**
 * @brief get type string
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return const char* the type string
 */
losu_extern_t const char* obj_typestr(losu_vm_t vm, losu_object_t obj) {
  const char* obj_typestr_table[] = {
      "false",     /* 0 */
      "true",      /* 1 */
      "number",    /* 2 */
      "string",    /* 3 */
      "function",  /* 4 */
      "unit",      /* 5 */
      "pointer",   /* 6 */
      "coroutine", /* 7 */
      "context",   /* 8 */
      "unknown",   /* 9 */
  };
  const char* unit_type_table[] = {
      "unknown", "array", "object", "class", "module", "global",
  };
  static char tmp[32];
  int32_t type =
      ovtype(obj) < losu_object_type_any ? ovtype(obj) : losu_object_type_any;
  if (type == losu_object_type_unit) {
    snprintf(tmp, sizeof(tmp), "unit-%s", unit_type_table[ovhash(obj)->type]);
    return tmp;
  } else
    return obj_typestr_table[type];
  return NULL;
}

#endif

/**
 * @brief create a null object
 * @param vm the losu vm handle
 * @return losu_object the null object
 */
losu_extern_t losu_object obj_newnull(losu_vm_t vm) {
  return __losu_objconst_null;
}

/**
 * @brief create a true object
 * @param vm the losu vm handle
 * @return losu_object the true object
 */
losu_extern_t losu_object obj_newtrue(losu_vm_t vm) {
  return __losu_objconst_true;
}

/* number */
#if 1

/**
 * @brief create a number object
 * @param vm the losu vm handle
 * @param num the number
 * @return losu_object the number object
 */
losu_extern_t losu_object obj_newnum(losu_vm_t vm, losu_ctype_number num) {
  return (losu_object){
      .type = losu_object_type_number,
      .value.num = num,
  };
}

/**
 * @brief covert object to number
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return losu_ctype_number the number
 */
losu_extern_t losu_ctype_number obj_tonum(losu_vm_t vm, losu_object_t obj) {
  int32_t t = ovtype(obj);
  ovtype(obj) = losu_object_type_number;
  switch (t) {
    case losu_object_type_false: {
      ovnumber(obj) = 0;
      break;
    }
    case losu_object_type_true: {
      ovnumber(obj) = 1;
      break;
    }
    case losu_object_type_number: {
      break;
    }
    case losu_object_type_string: {
      losu_ctype_number n = 0;
      losu_object_str2num(vm, ovSstr(obj), &n);
      ovnumber(obj) = n;
      break;
    }
    default: {
      ovnumber(obj) = 0;
      break;
    }
  }
  return ovnumber(obj);
}

/**
 * @brief get number, if type != number, return 0
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return losu_ctype_number the number
 */
losu_extern_t losu_ctype_number obj_getnum(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj) == losu_object_type_number ? ovnumber(obj) : NAN;
}

#endif

/* string */
#if 1
/**
 * @brief create a string object
 * @param vm the losu vm handle
 * @param str the string
 * @return losu_object the string object
 */
losu_extern_t losu_object obj_newstr(losu_vm_t vm, const char* str) {
  
  return (losu_object){
      .type = losu_object_type_string,
      .value.str = losu_objstring_new(vm, str),
  };
}

/**
 * @brief create a string object
 * @param vm the losu vm handle
 * @param str the string
 * @return losu_object the string object
 */
losu_extern_t losu_object obj_newstrlen(losu_vm_t vm,
                                        const char* str,
                                        uint32_t len) {
  return (losu_object){
      .type = losu_object_type_string,
      .value.str = losu_objstring_newlen(vm, str, len),
  };
}

/**
 * @brief covert object to string
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return const char* the string
 */
losu_extern_t const char* obj_tostr(losu_vm_t vm, losu_object_t obj) {
  char buff[64] = {0};
  int32_t t = ovtype(obj);
  ovtype(obj) = losu_object_type_string;
  switch (t) {
    case losu_object_type_false: {
      ovIstr(obj) = losu_objstring_new(vm, "(false)");
      break;
    }
    case losu_object_type_true: {
      ovIstr(obj) = losu_objstring_new(vm, "(true)");
      break;
    }
    case losu_object_type_number: {
      snprintf(buff, sizeof(buff), "%.16g", ovnumber(obj));
      ovIstr(obj) = losu_objstring_new(vm, buff);
      break;
    }
    case losu_object_type_string: {
      break;
    }
    case losu_object_type_function: {
      snprintf(buff, sizeof(buff), "(function %p)", ovfunc(obj));
      ovIstr(obj) = losu_objstring_new(vm, buff);
      break;
    }
    case losu_object_type_unit: {
      char tmp[32];
      switch (ovhash(obj)->type) {
        case obj_unittype_unknown:
          snprintf(tmp, sizeof(tmp), "unknown");
          break;
        case obj_unittype_array:
          snprintf(tmp, sizeof(tmp), "array");
          break;
        case obj_unittype_object:
          snprintf(tmp, sizeof(tmp), "object");
          break;
        case obj_unittype_class:
          snprintf(tmp, sizeof(tmp), "class");
          break;
        case obj_unittype_module:
          snprintf(tmp, sizeof(tmp), "module");
          break;
        case obj_unittype_global:
          snprintf(tmp, sizeof(tmp), "global");
          break;
        default:
          snprintf(tmp, sizeof(tmp), "unknown");
          break;
      }
      snprintf(buff, sizeof(buff), "(unit-%s %p)", tmp, ovhash(obj));
      ovIstr(obj) = losu_objstring_new(vm, buff);
      break;
    }
    case losu_object_type_pointer: {
      snprintf(buff, sizeof(buff), "(pointer %p)", ovptr(obj));
      ovIstr(obj) = losu_objstring_new(vm, buff);
      break;
    }
    default: {
      snprintf(buff, sizeof(buff), "(unknown %p)", obj);
      ovIstr(obj) = losu_objstring_new(vm, buff);
      break;
    }
  }
  return ovSstr(obj);
}

/**
 * @brief get string, if type != string, return NULL
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return const char* the string
 */
losu_extern_t const char* obj_getstr(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj) == losu_object_type_string ? ovSstr(obj) : NULL;
}

/**
 * @brief get string length, if type != string, return 0
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return size_t the string length
 */
losu_extern_t size_t obj_getstrlen(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj) == losu_object_type_string ? ovIstr(obj)->len : 0;
}

#endif

/* func */
#if 1
/**
 * @brief create a function object
 * @param vm the losu vm handle
 * @param func the function
 * @return losu_object the function object
 */
losu_extern_t losu_object obj_newfunc(losu_vm_t vm, losu_vmapi_t func) {
  losu_object_function_t f = losu_objfunc_new(vm, 0);
  f->isC = 1;
  f->func.capi = func;
  return (losu_object){
      .type = losu_object_type_function,
      .value.func = f,
  };
}

/**
 * @brief get function, if type != function, return NULL
 * @param vm the losu vm handle
 * @param obj the losu object
 * @return losu_vmapi_t the function
 */
losu_extern_t losu_vmapi_t obj_getfunc(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj) == losu_object_type_function
             ? (ovfunc(obj)->isC ? ovfunc(obj)->func.capi : NULL)
             : NULL;
}

#endif

/* unit */
#if 1
/**
 * @brief create a unit object
 * @param vm the losu vm handle
 * @param type the unit type
 * @return losu_object the unit object
 */
losu_extern_t losu_object obj_newunit(losu_vm_t vm, uint8_t type) {
  losu_object obj = (losu_object){
      .type = losu_object_type_unit,
      .value.hash = losu_objunit_new(vm, 0),
  };
  obj.value.hash->type = type;
  return obj;
}

/**
 * @brief get unit[key], if type != unit, return NULL
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @return const losu_object the value
 */
losu_extern_t const losu_object_t obj_unitindex(losu_vm_t vm,
                                                losu_object unit,
                                                losu_object key) {
  if (ovtype((&unit)) == losu_object_type_unit)
    return losu_objunit_get(vm, ovhash((&unit)), &key);
  return NULL;
}

/**
 * @brief get unit[key], if type != unit, return NULL
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @return const losu_object the value
 */
losu_extern_t const losu_object_t obj_unitindexnum(losu_vm_t vm,
                                                   losu_object unit,
                                                   losu_ctype_number key) {
  if (ovtype((&unit)) == losu_object_type_unit)
    return losu_objunit_getnum(vm, ovhash((&unit)), key);
  return NULL;
}

/**
 * @brief get unit[key], if type != unit, return NULL
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @return const losu_object the value
 */
losu_extern_t const losu_object_t obj_unitindexstr(losu_vm_t vm,
                                                   losu_object unit,
                                                   const char* key) {
  if (ovtype((&unit)) == losu_object_type_unit)
    return losu_objunit_getstr(vm, ovhash((&unit)),
                               losu_objstring_new(vm, key));
  return NULL;
}

/**
 * @brief set unit[key],if type != unit, return 1
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @param val the value
 * @return 0 if success, 1 if failed
 */
losu_extern_t losu_ctype_bool obj_unitset(losu_vm_t vm,
                                          losu_object unit,
                                          losu_object key,
                                          losu_object val) {
  if (ovtype((&unit)) != losu_object_type_unit)
    return 1;
  *losu_objunit_set(vm, ovhash((&unit)), &key) = val;
  return 0;
}

/**
 * @brief set unit[key],if type != unit, return 1
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @param val the value
 * @return 0 if success, 1 if failed
 */
losu_extern_t losu_ctype_bool obj_unitsetnum(losu_vm_t vm,
                                             losu_object unit,
                                             losu_ctype_number key,
                                             losu_object val) {
  if (ovtype((&unit)) != losu_object_type_unit)
    return 1;
  *losu_objunit_setnum(vm, ovhash((&unit)), key) = val;
  return 0;
}

/**
 * @brief set unit[key],if type != unit, return 1
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param key the key
 * @param val the value
 * @return 0 if success, 1 if failed
 */
losu_extern_t losu_ctype_bool obj_unitsetstr(losu_vm_t vm,
                                             losu_object unit,
                                             const char* key,
                                             losu_object val) {
  if (ovtype((&unit)) != losu_object_type_unit)
    return 1;
  *losu_objunit_setstr(vm, ovhash((&unit)), losu_objstring_new(vm, key)) = val;
  return 0;
}

/**
 * @brief get next node of unit
 * @param vm the losu vm handle
 * @param unit the unit object
 * @param node the node, if NULL,return first node
 * @return losu_hash_node_t the next node,NULL is end
 */
losu_extern_t losu_hash_node_t obj_unititer(losu_vm_t vm,
                                            losu_object unit,
                                            losu_hash_node_t node) {
  if (ovtype((&unit)) != losu_object_type_unit)
    return NULL;
  if (!node)
    return losu_objunit_getnext(vm, ovhash((&unit)),
                                (losu_object_t)(&__losu_objconst_null));
  return losu_objunit_getnext(vm, ovhash((&unit)), &(node->key));
}
/**
 * @brief get key of unit
 * @param vm the losu vm handle
 * @param node the node
 * @return losu_object the key
 */
losu_extern_t losu_object obj_unititerkey(losu_vm_t vm, losu_hash_node_t node) {
  if (node)
    return node->key;
  return __losu_objconst_null;
}
/**
 * @brief get  value of unit
 * @param vm the losu vm handle
 * @param node the node
 * @return losu_object the value
 */
losu_extern_t losu_object obj_unititerval(losu_vm_t vm, losu_hash_node_t node) {
  if (node)
    return node->value;
  return __losu_objconst_null;
}

#endif

/* pointer */
#if 1
/**
 * @brief create a pointer object
 * @param vm the losu vm handle
 * @param ptr the pointer
 * @return losu_object the pointer object
 */
losu_extern_t losu_object obj_newptr(losu_vm_t vm, void* ptr) {
  return (losu_object){
      .type = losu_object_type_pointer,
      .value.ptr = ptr,
  };
}

/**
 * @brief convert object to pointer
 * @param vm the losu vm handle
 * @param obj the object
 * @return void* the pointer
 */
losu_extern_t void* obj_toptr(losu_vm_t vm, losu_object_t obj) {
  int32_t type = ovtype(obj);
  ovtype(obj) = losu_object_type_pointer;
  switch (type) {
    case losu_object_type_false: {
      ovptr(obj) = NULL;
      break;
    }
    case losu_object_type_true: {
      ovptr(obj) = (void*)1;
      break;
    }
    case losu_object_type_number: {
      ovptr(obj) = (void*)((size_t)(ovnumber(obj)));
      break;
    }
    case losu_object_type_string: {
      ovptr(obj) = (void*)(ovIstr(obj));
      break;
    }
    case losu_object_type_function: {
      ovptr(obj) = (void*)(ovfunc(obj));
    }
    case losu_object_type_unit: {
      ovptr(obj) = (void*)(ovhash(obj));
      break;
    }
    case losu_object_type_pointer: {
      break;
    }
    case losu_object_type_coroutine: {
      ovptr(obj) = (void*)(ovcoro(obj));
      break;
    }
    case losu_object_type_context: {
      ovptr(obj) = (void*)(ovcontext(obj));
      break;
    }
    default:
      ovptr(obj) = NULL;
      break;
  }

  return ovptr(obj);
}

/**
 * @brief get pointer from object, if type != pointer, return NULL
 * @param vm the losu vm handle
 * @param obj the object
 * @return  void* the pointer
 */
losu_extern_t void* obj_getptr(losu_vm_t vm, losu_object_t obj) {
  return ovtype(obj) == losu_object_type_pointer ? ovptr(obj) : NULL;
}

#endif

#endif