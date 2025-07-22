/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like "copyright" restrictions. Losu qualifies
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
#ifndef FILE_SRC_LOSU_MEM_H
#define FILE_SRC_LOSU_MEM_H
#include <stdlib.h>
#include "losu.h"



/**
 * @brief Losu built-in memory allocator
 * @param vm Losu vm
 * @param old old space
 * @param size need size
 * @return pointer of new space
 */
void* losu_mem_realloc(losu_vm_t vm, void* old, losu_ctype_size_t size);

/**
 * @brief grow vector
 * @param vm Losu vm
 * @param block old block
 * @param i_num issue number
 * @param inc include
 * @param size size of issue
 * @param limit limit
 * @param errormsg Information when a failure occurs
 * @return pointer of new space
 */
void* losu_mem_grow(losu_vm_t vm,
                    void* block,
                    losu_ctype_size_t i_num,
                    int32_t inc,
                    losu_ctype_size_t size,
                    losu_ctype_size_t limit,
                    const char* errormsg);

#define losu_mem_malloc(vm, s) (losu_mem_realloc((vm), NULL, s))
#define losu_mem_free(vm, p) (losu_mem_realloc((vm), p, 0))
#define losu_mem_new(vm, t) ((t*)losu_mem_malloc((vm), sizeof(t)))
#define losu_mem_newvector(vm, n, t) \
  ((t*)losu_mem_malloc((vm), (n) * sizeof(t)))
#define losu_mem_growvector(vm, v, n, i, t, l, e) \
  ((v) = (t*)losu_mem_grow((vm), (v), (n), (i), sizeof(t), (l), (e)))
#define losu_mem_reallocvector(vm, v, s, t) \
  ((v) = (t*)losu_mem_realloc((vm), (v), (s) * sizeof(t)))

#endif
