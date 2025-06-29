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

#ifndef FILE_SRC_LOSU_OBJSTRING_C
#define FILE_SRC_LOSU_OBJSTRING_C
#include "objstring.h"
#include "losu_bytecode.h"
#include "losu_mem.h"
#include "losu_object.h"

#include <stdio.h>
#include <string.h>

/**
 * string api
 *    with: new newlen newconst newconstlen
 * */
#if 1
/**
 * @brief create new losu-string with C style
 * @param vm LosuVm handle
 * @param s C string (const char*)
 * @return  losu-string handle
 */
losu_object_string_t losu_objstring_new(losu_vm_t vm, const char* s) {
  return losu_objstring_newlen(vm, s, (uint32_t)strlen(s));
}

/**
 * @brief create const losu-string with C style
 * @param vm LosuVm handle
 * @param s C string (const char*)
 * @return  losu-string handle
 */
losu_object_string_t losu_objstring_newconst(losu_vm_t vm, const char* s) {
  losu_object_string_t ts = losu_objstring_newlen(vm, s, (uint32_t)strlen(s));
  if (!ts->marked)
    ts->marked = losu_objstring_constmark;
  return ts;
}

/**
 * @brief create const losu-string with length
 * @param vm LosuVm handle
 * @param s C string (const char*)
 * @param len length of string
 * @return  losu-string handle
 */
losu_object_string_t losu_objstring_newconstlen(losu_vm_t vm,
                                                const char* s,
                                                losu_ctype_size_t len) {
  losu_object_string_t ts = losu_objstring_newlen(vm, s, len);
  if (!ts->marked)
    ts->marked = losu_objstring_constmark;
  return ts;
}

/**
 * @brief create new losu-string with length
 * @param vm LosuVm handle
 * @param s C string (const char*)
 * @param len length of string
 * @return  losu-string handle
 */
losu_object_string_t losu_objstring_newlen(losu_vm_t vm,
                                           const char* s,
                                           losu_ctype_size_t len) {
  losu_ctype_hash_t h = stringhash(s, len);
  losu_ctype_hash_t h1 = h & (vm->strpool.size - 1);
  losu_object_string_t ts;
  /* is exsist ? */
  for (ts = vm->strpool.strobj[h1]; ts; ts = ts->next)
    if (ts->hash == h && ts->len == len && (memcmp(s, ts->str, len) == 0))
      return ts;
  /* new */
  ts = (losu_object_string_t)losu_mem_malloc(vm, sizestring(len));
  vm->nblocks += sizestring(len); /* malloc & gc add */
  /* set */
  ts->marked = 0;
  ts->next = NULL;
  ts->len = len;
  ts->hash = h;
  ts->cstidx = 0;
  memcpy(ts->str, s, len);
  ts->str[len] = '\0';
  /* strngpool */
  ts->next = vm->strpool.strobj[h1];
  vm->strpool.strobj[h1] = ts;
  vm->strpool.nsize++;
  if (vm->strpool.nsize > vm->strpool.size && vm->strpool.size < 1024)
    __losu_objstrpool_resize(vm, &vm->strpool, vm->strpool.size * 2);
  /* return new */
  return ts;
}

/**
 * @brief hash a C string with length
 * @param s string
 * @param len length
 * @return hash value
 */
static losu_ctype_hash_t stringhash(const char* s, losu_ctype_size_t len) {
  losu_ctype_size_t h = len;
  losu_ctype_size_t step = (len >> 5) | 1;
  for (; len >= step; len -= step)
    h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*(s++));
  return (losu_ctype_hash_t)h;
}

// SDBM Hash
/* static losu_ctype_hash_t stringhash(const char* s, losu_ctype_size_t len) {
  losu_ctype_hash_t hash = 0;
  len = len & 1024UL;
  while (len--) {
    hash = (unsigned char)*s++ + (hash << 6) + (hash << 16) - hash;
  }
  return hash;
} */

// Losu Hash
/* static losu_ctype_hash_t stringhash(const char* s, losu_ctype_size_t len) {
  losu_ctype_size_t h = len;
  losu_ctype_size_t step = (len >> 5) | 1;
  for (; len >= step; len -= step)
    h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*(s++));
  return (losu_ctype_hash_t)h;
} */

// Murmur

/* static losu_ctype_hash_t stringhash(const char* s, losu_ctype_size_t len) {
  losu_ctype_hash_t h = len;
  const uint32_t m = 0x5bd1e995;
  while (len >= 4) {
    uint32_t k = *(uint32_t*)s;
    k *= m;
    k ^= k >> 24;
    k *= m;
    h = (k ^ h) * m;
    s += 4;
    len -= 4;
  }
  while (len--) {
    h = ((h << 5) + h) + (unsigned char)*s++;
  }
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
} */

#endif

/**
 * string pool
 *    with: init deinit resize
 * */
#if 1
/**
 * @brief init vm's string pool
 * @param vm losu vm handle
 */
void __losu_objstrpool_init(losu_vm_t vm) {
  vm->strpool.strobj = losu_mem_newvector(vm, 1, losu_object_string_t);
  vm->nblocks += sizeof(losu_object_string_t); /* malloc & gc add */
  vm->strpool.size = 1;
  vm->strpool.nsize = 0;
  vm->strpool.strobj[0] = NULL;
}

/**
 * @brief resize vm's string pool
 * @param vm losu vm handle
 * @param strpool stringpool handle
 * @param s size
 */
void __losu_objstrpool_resize(losu_vm_t vm,
                              __losu_vm_strpool_t strpool,
                              losu_ctype_hash_t s) {
  losu_object_string_t* newstrobj =
      losu_mem_newvector(vm, s, losu_object_string_t);

  memset(newstrobj, 0, s * sizeof(losu_object_string_t));
  for (losu_ctype_hash_t i = 0; i < strpool->size; i++) {
    losu_object_string_t p = strpool->strobj[i];
    while (p) {
      losu_object_string_t next = p->next;
      losu_ctype_hash_t h = p->hash;
      losu_ctype_hash_t h1 = h & (s - 1);
      p->next = newstrobj[h1];
      newstrobj[h1] = p;
      p = next;
    }
  }
  losu_mem_free(vm, strpool->strobj);
  vm->nblocks += ((losu_ctype_ssize_t)s - (losu_ctype_ssize_t)strpool->size) *
                 sizeof(losu_object_string_t);
  strpool->size = s;
  strpool->strobj = newstrobj;
}

/**
 * @brief deinit vm's string pool
 * @param vm losu vm handle
 */
void __losu_objstrpool_deinit(losu_vm_t vm) {
  vm->nblocks -= vm->strpool.size * sizeof(losu_object_string_t);
  losu_mem_free(vm, vm->strpool.strobj); /* free & gc sub */
}

#endif

#endif