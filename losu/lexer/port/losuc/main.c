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

#ifndef FILE_SRC_PORT_LOSUC_MAIN_C
#define FILE_SRC_PORT_LOSUC_MAIN_C

#include "losu.h"
#include "losu_dump.h"
#include "losu_logo.h"
#include "losu_mem.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stdout, "Losu Compiler - compile losu source to LISA bytecode\n");
    fprintf(stdout, "vm bit: %lubit\n", sizeof(losu_ctype_vmins_t) * 8);
    fprintf(stdout, "sdk info: v%s\n", LOSU_RELEASE);
    fprintf(stdout, "usage:\n\tlosuc [path of file]\n");
    fprintf(stdout, "%s\n", logo_txt);
  } else {
    // get path
    const char* srcfile = argv[1];
    const char* objfile = argc > 2 ? argv[2] : "a.out";
    // vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    if (losu_dumpfile(srcfile, objfile)) {
      if (remove(objfile)) {
        printf("remove %s failed:%s\n", objfile, strerror(errno));
      }
    }
  }
  return 0;
}

#endif
