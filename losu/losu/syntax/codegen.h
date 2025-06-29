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

#ifndef FILE_SRC_LOSU_SYNTAX_CODEGEN_H
#define FILE_SRC_LOSU_SYNTAX_CODEGEN_H

#include "losu_syntax.h"

#define iO INS_MODE_OP
#define iBA INS_MODE_IBA
#define iU INS_MODE_IU
#define iS INS_MODE_IS
#define V 0xff /* V means variable */
static struct {
  uint8_t op;
  int8_t dlt; /* push - pop */
} const _vmops[] = {
    {iO, 0},  /* INS_END 0, 0 */
    {iU, 0},  /* INS_RETURN 0, 0 */
    {iBA, 0}, /* INS_CALL 0, 0 */

    {iU, V}, /* INS_PUSHNULL V, 0 */
    {iU, V}, /* INS_PUSHTRUE V, 0 */
    {iU, V}, /* INS_POP V, 0 */

    {iU, 1},  /* INS_PUSHSTRING 1, 0  */
    {iU, 1},  /* INS_PUSHNUM 1, 0 */
    {iU, 1},  /* INS_PUSHUPVALUE 1, 0 */
    {iBA, V}, /* INS_PUSHFUNCTION V, 0 */
    {iU, 1},  /* INS_PUSHSELF 2, 1 */

    {iU, 1},  /* INS_GETLOCAL 1, 0 */
    {iU, 1},  /* INS_GETGLOBAL 1, 0 */
    {iU, -1}, /* INS_SETLOCAL 0, 1 */
    {iU, -1}, /* INS_SETGLOBAL 0, 1 */

    {iBA, 1},  /* INS_CREATEUNIT 1, 0 */
    {iBA, V},  /* INS_SETUNIT V, 0 */
    {iBA, -1}, /* INS_GETUNIT 1, 2 */
    {iBA, V},  /* INS_SETLIST V, 0 */
    {iU, V},   /* INS_SETMAP V, 0 */

    {iO, -1}, /* INS_PIPE 0, 1 */
    {iO, -1}, /* INS_ADD 1, 2 */
    {iO, -1}, /* INS_SUB 1, 2 */
    {iO, -1}, /* INS_MULT 1, 2 */
    {iO, -1}, /* INS_DIV 1, 2 */
    {iO, -1}, /* INS_POW 1, 2 */
    {iO, -1}, /* INS_MOD 1, 2 */
    {iO, -1}, /* INS_CONCAT 1, 2 */
    {iO, 0},  /* INS_NEG 1, 1 */
    {iO, 0},  /* INS_NOT 1, 1 */

    {iS, -2}, /* INS_JMPNE 0, 2 */
    {iS, -2}, /* INS_JMPEQ 0, 2 */
    {iS, -2}, /* INS_JMPLT 0, 2 */
    {iS, -2}, /* INS_JMPLE 0, 2 */
    {iS, -2}, /* INS_JMPGT 0, 2 */
    {iS, -2}, /* INS_JMPGE 0, 2 */

    {iS, -1}, /* INS_JMPT 0, 1 */
    {iS, -1}, /* INS_JMPF 0, 1 */
    {iS, -1}, /* INS_JMPONT 0, 1 */
    {iS, -1}, /* INS_JMPONF 0, 1 */
    {iS, 0},  /* INS_JMP 0, 0 */

    {iO, 0}, /* INS_PUSHNULLJMP 0, 0 */

    {iS, 0},  /* INS_FORPREP 0, 0 */
    {iS, -3}, /* INS_FORNEXT 0, 3 */
    {iS, 2},  /* INS_FORINPREP 2, 0 */
    {iS, -3}, /* INS_FORINNEXT 0, 3 */

    {iU, 0}, /* INS_YIELD 0, 0 */

    {iU, 0}, /* INS_IMPORT 0, 0 */
    {iU, 0}, /* INS_ASYNC 0 0 */

    {iO, 1}, /* INS_COPY 1 0 */
    {iO, 0}, /* INS_TEST 0 0 */

};

#undef iO
#undef iBA
#undef iU
#undef iS
#undef V

#endif