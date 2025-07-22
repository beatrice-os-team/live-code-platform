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

#ifndef FILE_SRC_VM_SIGNAL_C
#define FILE_SRC_VM_SIGNAL_C
#include "losu_vm.h"

#include <stdlib.h>

/**
 * @brief create signal msg from list
 * @param msg signal msg list
 * @return losu_sigmsg_t signal msg
 */
/* losu_extern_t losu_sigmsg_t losu_sigmsg_create(losu_sigmsg_t msg[]) {
  losu_sigmsg_t ret = losu_sigmsg_none;
  for (int32_t i = 0; msg[i] != losu_sigmsg_none && i < losu_sigmsg_total; i++)
    ret += msg[i];
  return ret;
}
 */
/**
 * @brief  check if the signal msg is valid
 * @param signal source signal
 * @param check  check signal
 * @return  losu_ctype_bool
 */
/* losu_extern_t losu_ctype_bool losu_sigmsg_check(losu_sigmsg_t signal,
                                                losu_sigmsg_t check) {
  return (signal & check) == check;
} */

/**
 * @brief throw a msg with longjmp
 * @param vm Losu vm
 * @param s signal
 * @param msg signal msg
 * @return
 */
losu_extern_t void losu_sigmsg_throw(losu_vm_t vm, losu_signal_t s) {
  if (vm->errjmp)
    longjmp(*vm->errjmp, s);
  exit(s);
}

#endif
