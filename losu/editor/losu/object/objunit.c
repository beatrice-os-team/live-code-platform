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

#ifndef FILE_SRC_OBJECT_OBJUNIT_C
#define FILE_SRC_OBJECT_OBJUNIT_C

#include "objunit.h"
#include "losu_mem.h"
#include "losu_object.h"

/* unit: new & del */
#if 1
/**
 * @brief create new unit object (hash table)
 * @param vm losu vm handle
 * @param size size
 * @return unit handle
 */
losu_object_hash_t losu_objunit_new(losu_vm_t vm, int32_t size) {
  losu_object_hash_t t = losu_mem_new(vm, losu_object_hash);
  t->type = obj_unittype_unknown;
  vm->nblocks += sizeObjHash(0); /* malloc */

  t->next = vm->hashpool;
  vm->hashpool = t;
  t->mark = t;
  t->size = 0;
  t->node = NULL;
  {
    int32_t p = 4;
    while (p <= size)
      p <<= 1;
    size = p;
  }
  __losu_objunit_setvector(vm, t, size);
  return t;
}

/**
 * @brief remove k-v, v == false
 * @param vm losu vm handle
 * @param key key
 */
void losu_objunit_remove(losu_vm_t vm, losu_object_t key) {
  /**
   * simple remove, do nothing
   * you can add code to this, for faster GC
   * */
}

/**
 * @brief free a unit object
 * @param vm losu vm handle
 * @param t
 */
void losu_objunit_free(losu_vm_t vm, losu_object_hash_t t) {
  vm->nblocks -= sizeObjHash(t->size);
  losu_mem_free(vm, t->node);
  losu_mem_free(vm, t);
}

#endif

/* unit: set */
#if 1
/**
 * @brief set unit, get t[key] pointer
 * @param vm losu vm handle
 * @param t unit table
 * @param key key
 * @return insert point
 */
losu_object_t losu_objunit_set(losu_vm_t vm,
                               losu_object_hash_t t,
                               losu_object_t key) {
  losu_hash_node_t mp = __losu_objunit_mainkey(t, key);
  losu_hash_node_t n = mp;
  if (mp == NULL) {
    vm_error(vm, "Attempting to create illegal key for 'unit'. ");
    return NULL;
  }
  do {
    if (losu_object_iseq(vm, key, &n->key))
      return &n->value;
    n = n->next;
  } while (n);
  if (ovtype((&mp->key)) != losu_object_type_false) {
    losu_hash_node_t oth;
    n = t->free;
    if (mp > n && (oth = __losu_objunit_mainkey(t, &mp->key)) != mp) {
      while (oth->next != mp)
        oth = oth->next;
      oth->next = n;
      *n = *mp;
      mp->next = NULL;
    } else {
      n->next = mp->next;
      mp->next = n;
      mp = n;
    }
  }
  mp->key = *key;
  while (1) {
    if (ovtype((&t->free->key)) == losu_object_type_false)
      return &mp->value;
    else if (t->free == t->node)
      break;
    else
      (t->free)--;
  }
  __losu_objunit_rehash(vm, t);
  return losu_objunit_set(vm, t, key);
}

/**
 * @brief same as losu_objunit_set
 * @param vm
 * @param t
 * @param key
 * @return
 */
losu_object_t losu_objunit_setnum(losu_vm_t vm,
                                  losu_object_hash_t t,
                                  losu_ctype_number key) {
  losu_object idx = (losu_object){
      .type = losu_object_type_number,
      .value.num = key,
  };
  return losu_objunit_set(vm, t, &idx);
}
/**
 * @brief same as losu_objunit_set
 * @param vm
 * @param t
 * @param key
 * @return
 */
losu_object_t losu_objunit_setstr(losu_vm_t vm,
                                  losu_object_hash_t t,
                                  losu_object_string_t key) {
  losu_object idx = (losu_object){
      .type = losu_object_type_string,
      .value.str = key,
  };
  return losu_objunit_set(vm, t, &idx);
}

#endif

/* unit: get */
#if 1
/**
 * @brief get unit t[key], for get
 * @param vm
 * @param t
 * @param key
 * @return
 */
const losu_object_t losu_objunit_get(losu_vm_t vm,
                                     losu_object_hash_t t,
                                     losu_object_t key) {
  switch (ovtype(key)) {
    case losu_object_type_number:
      return losu_objunit_getnum(vm, t, ovnumber(key));
    case losu_object_type_string:
      return losu_objunit_getstr(vm, t, ovIstr(key));
    default:
      return losu_objunit_getany(vm, t, key);
  }
  return NULL;
}

/**
 * @brief as losu_objunit_get
 * @param vm
 * @param t
 * @param key
 * @return
 */
const losu_object_t losu_objunit_getany(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_object_t key) {
  losu_hash_node_t n = __losu_objunit_mainkey(t, key);
  if (n) {
    do {
      if (losu_object_iseq(vm, (const losu_object_t)(key),
                           (const losu_object_t)(&n->key)))
        return (const losu_object_t)&n->value;
      n = n->next;
    } while (n);
  }
  return (const losu_object_t)&__losu_objconst_null;
}

/**
 * @brief as losu_objunit_get
 * @param vm
 * @param t
 * @param key
 * @return
 */
const losu_object_t losu_objunit_getnum(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_ctype_number key) {
  losu_hash_node_t n = &t->node[(losu_ctype_size_t)(key) & (t->size - 1)];
  do {
    if (ovtype((&n->key)) == losu_object_type_number &&
        ovnumber((&n->key)) == key)
      return (const losu_object_t)&n->value;
    n = n->next;
  } while (n);
  return (const losu_object_t)&__losu_objconst_null;
}

/**
 * @brief as losu_objunit_get
 * @param vm
 * @param t
 * @param key
 * @return
 */
const losu_object_t losu_objunit_getstr(losu_vm_t vm,
                                        losu_object_hash_t t,
                                        losu_object_string_t key) {
  losu_hash_node_t n = &t->node[key->hash & (t->size - 1)];

  do {
    if (ovtype((&n->key)) == losu_object_type_string &&
        ovIstr((&n->key)) == key)
      return (const losu_object_t)&n->value;

    n = n->next;
  } while (n);
  return (const losu_object_t)&__losu_objconst_null;
}

/**
 * @brief get a global object from vm->global
 * @param vm losu vm handle
 * @param name object name
 * @return object handle
 */
const losu_object_t losu_objunit_getglobal(losu_vm_t vm, const char* name) {
  return losu_objunit_getstr(vm, vm->global, losu_objstring_new(vm, name));
}

/**
 * @brief get next node for unit
 * @param vm losu vm handle
 * @param t unit table
 * @param key key now
 * @return next node, nil == NULL
 */
const losu_hash_node_t losu_objunit_getnext(losu_vm_t vm,
                                            losu_object_hash_t t,
                                            losu_object_t key) {
  int32_t i;
  if (ovtype(key) == losu_object_type_false)
    i = 0;
  else {
    const losu_object_t v = losu_objunit_get(vm, t, key);
    if (v->type == losu_object_type_false)
      return NULL;
    i = (int32_t)(((char*)v - (char*)(&t->node[0].value)) /
                      sizeof(losu_hash_node) +
                  1);
  }
  for (; i < t->size; i++) {
    losu_hash_node_t n = &(t->node[i]);
    if (ovtype((&n->value)) != losu_object_type_false)
      return n;
  }

  return NULL;
}
#endif

/* static */
#if 1
/**
 * @brief get mainkey for unit
 * @param t unit object
 * @param key key object
 * @return main-key
 */
static losu_hash_node_t __losu_objunit_mainkey(losu_object_hash_t t,
                                               losu_object_t key) {
  losu_ctype_size_t h = 0;
  switch (ovtype(key)) {
    case losu_object_type_number:
      h = (losu_ctype_size_t)(ovnumber(key));
      break;
    case losu_object_type_string:
      h = (losu_ctype_size_t)(ovIstr(key)->hash);
      break;
    case losu_object_type_unit:
      h = (losu_ctype_size_t)((size_t)(ovhash(key)));
      break;
    case losu_object_type_function:
      h = (losu_ctype_size_t)((size_t)(ovfunc(key)));
      break;
    default:
      return NULL;
  }

  return &t->node[h & (t->size - 1)];
}

/**
 * @brief setvector of unit object
 * @param vm losu vm handle
 * @param t unit object
 * @param size new size
 */
static void __losu_objunit_setvector(losu_vm_t vm,
                                     losu_object_hash_t t,
                                     uint32_t size) {
  if (size > INT32_MAX)
    vm_error(vm, "'unit' object overflow size, max= '%d' ", INT32_MAX);
  t->node = losu_mem_newvector(vm, size, losu_hash_node);
  for (int32_t i = 0; i < size; i++) {
    ovtype((&t->node[i].key)) = ovtype((&t->node[i].value)) =
        losu_object_type_false;
    t->node[i].next = NULL;
  }
  vm->nblocks += ((losu_ctype_ssize_t)sizeObjHash(size)) -
                 ((losu_ctype_ssize_t)sizeObjHash(t->size));
  t->size = size;
  t->free = &t->node[size - 1];
}

/**
 * @brief get now size of unit object
 * @param t unit object
 * @return size of unit
 */
static int32_t __losu_objunit_getnsize(losu_object_hash_t t) {
  losu_hash_node_t v = t->node;
  int32_t s = t->size;
  int32_t use = 0;
  for (int32_t i = 0; i < s; i++)
    if (ovtype((&v[i].value)) != losu_object_type_false)
      use++;
  return use;
}

/**
 * @brief rehash now size unit object
 * @param vm losu vm handle
 * @param t unit object
 */
static void __losu_objunit_rehash(losu_vm_t vm, losu_object_hash_t t) {
  uint32_t osize = (uint32_t)t->size;
  losu_hash_node_t onode = t->node;
  int32_t uszie = __losu_objunit_getnsize(t);
  if (uszie >= osize - osize / 4)
    __losu_objunit_setvector(vm, t, osize * 2);
  else if (uszie <= osize / 4 && osize > 4)
    __losu_objunit_setvector(vm, t, osize / 2);
  else
    __losu_objunit_setvector(vm, t, osize);

  for (int32_t i = 0; i < osize; i++)
    if (ovtype((&((onode + i)->value))) != losu_object_type_false)
      *losu_objunit_set(vm, t, &(onode + i)->key) =
          ((losu_hash_node_t)(onode + i))->value;
  losu_mem_free(vm, onode);
}

#endif

#endif
