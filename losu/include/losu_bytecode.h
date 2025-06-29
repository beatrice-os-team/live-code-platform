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

/**
 * This document specifies the bytecode standard for LosuVM, a small-end
 * (default) RISC-architecture virtual machine with about 50 instructions that
 * can be executed in both SBD (default) and thread-code (performance) modes.
 */

#ifndef FILE_SRC_LOSU_BYTECODE_H
#define FILE_SRC_LOSU_BYTECODE_H

/**
 * include head file segment
 */
#include "losu.h"

/**
 * Vm-Ins (V-ISA):
 * INS =
 *     (
 *         OP
 *         OP + U
 *         OP + S
 *         OP + B + A
 *     ) + E
 *  OP -> 8 bit uint8_t
 *  B  -> 8 bit uint8_t
 *  A  -> 32 bit uint32_t
 *  U  -> 32 bit uint32_t
 *  S  -> 32 bit int32_t
`*  E  -> 16 bit uint16_t
 */
typedef uint8_t _losu_vmins_OP, _losu_vmins_B;
typedef uint32_t _losu_vmins_U, _losu_vmins_A;
typedef int32_t _losu_vmins_S;
typedef uint16_t _losu_vmins_E;

#define _losu_vmins_size (sizeof(losu_ctype_vmins_t))
#define _losu_vmins_sizeOP 8
#define _losu_vmins_sizeB 8
#define _losu_vmins_sizeA 16
#define _losu_vmins_sizeU 24
#define _losu_vmins_sizeS 24
#define _losu_vmins_sizeE 16

#define _losu_vmins_posOP 0
#define _losu_vmins_posU 8
#define _losu_vmins_posS 8
#define _losu_vmins_posB 8
#define _losu_vmins_posA 16
#define _losu_vmins_posE 48

#define _losu_vmins_maxU (((losu_ctype_vmins_t)(1) << _losu_vmins_sizeU) - 1)
#define _losu_vmins_maxS (_losu_vmins_maxU >> 1)
#define _losu_vmins_maxB (((losu_ctype_vmins_t)(1) << _losu_vmins_sizeB) - 1)
#define _losu_vmins_maxA (((losu_ctype_vmins_t)(1) << _losu_vmins_sizeA) - 1)

#define _losu_vmlim_maxstack 512  /* max stack for expression */
#define _losu_vmlim_maxlocvar 128 /* < _losu_vmlim_maxstack */
#define _losu_vmlim_maxglovar 128 /* any, global declare in local */
#define _losu_vmlim_maxclosure 64 /* < _losu_vmlim_maxstack */
#define _losu_vmlim_maxsetlist 64 /* < _losu_vmlim_maxstack / 4 */
#define _losu_vmlim_maxsetmap (_losu_vmlim_maxsetlist / 2)
#define _losu_vmlim_maxstrlen UINT32_MAX
#define _losu_vmlim_maxint32 INT32_MAX
#define _losu_vmlim_maxnumbuff 0 /* number buff size, 0 is best */

#define _losu_vmcg_mask1(n, p) \
  ((~((~(losu_ctype_vmins_t)0) << n)) << p) /* lmov n bit 1 to p */
#define _losu_vmcg_mask0(n, p)                   \
  (~_losu_vmcg_mask1(n, p)) /* lmov n bit 0 to p \
                             */

/**
 * codegen segment
 *      OP U S AB
 *  New
 *  Get
 *  Set
 * */
#define _losu_vmcg_newOP(op) ((losu_ctype_vmins_t)(op))
#define _losu_vmcg_getOP(i) ((_losu_vmins_OP)(i))
#define _losu_vmcg_setOP(i, op)                                               \
  ((i) =                                                                      \
       (((losu_ctype_vmins_t)(i) & _losu_vmcg_mask0(_losu_vmins_sizeOP, 0)) | \
        (losu_ctype_vmins_t)(op)))

#define _losu_vmcg_newU(op, u) \
  ((losu_ctype_vmins_t)(op) | ((losu_ctype_vmins_t)(u) << _losu_vmins_posU))
#define _losu_vmcg_getU(i) \
  ((_losu_vmins_U)((losu_ctype_vmins_t)(i) >> _losu_vmins_posU))
#define _losu_vmcg_setU(i, u)                                             \
  ((i) = (((i) & _losu_vmcg_mask0(_losu_vmins_sizeU, _losu_vmins_posU)) | \
          ((losu_ctype_vmins_t)(u) << _losu_vmins_posU)))

#define _losu_vmcg_newS(op, s) (_losu_vmcg_newU((op), (s) + _losu_vmins_maxS))
#define _losu_vmcg_getS(i) \
  ((_losu_vmins_S)(_losu_vmcg_getU(i) - _losu_vmins_maxS))
#define _losu_vmcg_setS(i, s) (_losu_vmcg_setU(i, (s) + _losu_vmins_maxS))

#define _losu_vmcg_newBA(op, a, b)                 \
  (((losu_ctype_vmins_t)(op)) |                    \
   ((losu_ctype_vmins_t)(a) << _losu_vmins_posA) | \
   ((losu_ctype_vmins_t)(b) << _losu_vmins_posB))
#define _losu_vmcg_getA(i) (((_losu_vmins_A)((i) >> _losu_vmins_posA)))
#define _losu_vmcg_setA(i, a)                                             \
  ((i) = (((i) & _losu_vmcg_mask0(_losu_vmins_sizeA, _losu_vmins_posA)) | \
          ((losu_ctype_vmins_t)(a) << _losu_vmins_posA)))
#define _losu_vmcg_getB(i) ((_losu_vmins_B)(((i) >> _losu_vmins_posB)))
#define _losu_vmcg_setB(i, b)                                             \
  ((i) = (((i) & _losu_vmcg_mask0(_losu_vmins_sizeB, _losu_vmins_posB)) | \
          ((losu_ctype_vmins_t)(b) << _losu_vmins_posB)))

typedef enum _losu_vmins_list {
  INS_END,    /* iO   0   0 */
  INS_RETURN, /* iU   0   0 */
  INS_CALL,   /* iBA  0   0 */

  INS_PUSHNULL, /* iU   V   0 */
  INS_PUSHTRUE, /* iU   V   0 */
  INS_POP,      /* iU   V   0 */

  INS_PUSHSTRING,   /* iU   1   0 */
  INS_PUSHNUM,      /* iU   1   0 */
  INS_PUSHUPVALUE,  /* iU   1   0 */
  INS_PUSHFUNCTION, /* iBA  V   0 */
  INS_PUSHSELF,     /* iU   2   1 */

  INS_GETLOCAL,  /* iU   1   0 */
  INS_GETGLOBAL, /* iU   1   0 */
  INS_SETLOCAL,  /* iU   0   1 */
  INS_SETGLOBAL, /* iU   0   1 */

  INS_CREATEUNIT, /* iBA   1   0 */
  INS_SETUNIT,    /* iBA  V   0 */
  INS_GETUNIT,    /* iBA  1   2 */
  INS_SETLIST,    /* iBA  V   0 */
  INS_SETMAP,     /* iU   V   0 */

  INS_PIPE,   /* iO   1   2 */
  INS_ADD,    /* iO   1   2 */
  INS_SUB,    /* iO   1   2 */
  INS_MULT,   /* iO   1   2 */
  INS_DIV,    /* iO   1   2 */
  INS_POW,    /* iO   1   2 */
  INS_MOD,    /* iO   1   2 */
  INS_CONCAT, /* iO   1   2 */
  INS_NEG,    /* iO   1   1 */
  INS_NOT,    /* iO   1   1 */

  INS_JMPNE, /* iS   0   2 */
  INS_JMPEQ, /* iS   0   2 */
  INS_JMPLT, /* iS   0   2 */
  INS_JMPLE, /* iS   0   2 */
  INS_JMPGT, /* iS   0   2 */
  INS_JMPGE, /* iS   0   2 */

  INS_JMPT,   /* iS   0   1 */
  INS_JMPF,   /* iS   0   1 */
  INS_JMPONT, /* iS   0   1 */
  INS_JMPONF, /* iS   0   1 */
  INS_JMP,    /* iS   0   0 */

  INS_PUSHNULLJMP, /* iO   0   0 */

  INS_FORPREP,   /* iS   0   0 */
  INS_FORNEXT,   /* iS   0   3 */
  INS_FORINPREP, /* iS   2   0 */
  INS_FORINNEXT, /* iS   0   3 */

  INS_YIELD, /* iU   0   0 */

  INS_IMPORT, /* iU  0   0 */
  INS_ASYNC,  /* iU   0   0 */

  INS_COPY, /* iO  1   0 */

  INS_TEST, /* iO  0   0 */
  INS_TOTAL_NUMBER,
} _losu_vmins_list;

#define _losu_vmins_isjmp(op) ((INS_JMPNE <= (op) && (op) <= INS_JMP))

typedef enum _losu_vmins_binOP {
  INS_BINOP_ADD,    /* + */
  INS_BINOP_SUB,    /* - */
  INS_BINOP_MULT,   /* * */
  INS_BINOP_DIV,    /* / */
  INS_BINOP_POW,    /* ** */
  INS_BINOP_MOD,    /* % */
  INS_BINOP_CONCAT, /* & */

  INS_BINOP_NE, /* != */
  INS_BINOP_EQ, /* == */

  INS_BINOP_LT, /* < */
  INS_BINOP_LE, /* <= */

  INS_BINOP_GT, /* > */
  INS_BINOP_GE, /* >= */

  INS_BINOP_PIPE, /* pipe */

  INS_BINOP_AND, /* and */
  INS_BINOP_OR,  /* or */

  INS_BINOP_NULL,
} _losu_vmins_binOP;

typedef enum _losu_vmins_unOP {
  INS_UNOP_NEG, /* - */
  INS_UNOP_NOT, /* not */

  INS_UNOP_NULL,
} _losu_vmins_unOP;

typedef enum _losu_vmins_mode {
  INS_MODE_OP,
  INS_MODE_IU,
  INS_MODE_IS,
  INS_MODE_IBA,
} _losu_vmins_mode;

#endif